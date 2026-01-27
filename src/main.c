#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>
#include <common.h>
#include <tcp.h>
#include <kiss.h>
#include <ax25.h>
#include <tnc2.h>
#include <buffer.h>

static ax25_addr_t own_call;

static int num_untraced_aliases = 0;
static ax25_addr_t untraced_aliases[16];

static int num_traced_aliases = 0;
static ax25_addr_t traced_aliases[16];

static uint8_t tnc2_output_buffer[512];
static volatile sig_atomic_t g_shutdown_requested = 0;

static void signal_handler(int sig)
{
    (void)sig;
    g_shutdown_requested = 1;
}

static void configure_untraced_alias(const char *alias, int max_hops)
{
    char tmp[16] = {0};
    for (int hops = 1; hops <= max_hops; hops++)
    {
        snprintf(tmp, 16, "%s%d", alias, hops);
        ax25_addr_init_with(&untraced_aliases[num_untraced_aliases++], tmp, 0, false);
    }
}

static void configure_traced_alias(const char *alias, int max_hops)
{
    char tmp[16] = {0};
    for (int hops = 1; hops <= max_hops; hops++)
    {
        snprintf(tmp, 16, "%s%d", alias, hops);
        ax25_addr_init_with(&traced_aliases[num_traced_aliases++], tmp, 0, false);
    }
}

static bool base_call_equal(const ax25_addr_t *addr1, const ax25_addr_t *addr2)
{
    if (addr1 == NULL)
        return addr2 == NULL;
    if (addr2 == NULL)
        return addr1 == NULL;
    return !strncasecmp(addr1->callsign, addr2->callsign, AX25_ADDR_MAX_CALLSIGN_LEN);
}

static void process_packet(ax25_packet_t *packet)
{
    buffer_t tnc2_buf = {
        .data = tnc2_output_buffer,
        .capacity = sizeof(tnc2_output_buffer),
        .size = 0};

    if (tnc2_packet_to_string(packet, &tnc2_buf) < 0)
    {
        LOG("unable to encode packet to tnc2");
        return;
    }

    LOG("rx %s", (char *)tnc2_buf.data);

    int own_call_idx = -1;
    int untraced_alias_idx = -1;
    int traced_alias_idx = -1;

    for (int i = 0; i < packet->path_len; i++)
    {
        ax25_addr_t *addr = &packet->path[i];
        if (addr->repeated)
            continue;

        if (own_call_idx < 0 && base_call_equal(addr, &own_call))
            own_call_idx = i;

        for (int j = 0; j < num_untraced_aliases && untraced_alias_idx < 0; j++)
            if (base_call_equal(addr, &untraced_aliases[j]))
                untraced_alias_idx = i;

        for (int j = 0; j < num_traced_aliases && traced_alias_idx < 0; j++)
            if (base_call_equal(addr, &traced_aliases[j]))
                traced_alias_idx = i;
    }

    if (own_call_idx >= 0)
        LOG("own call at index %d", own_call_idx)

    if (untraced_alias_idx >= 0)
        LOG("untraced alias at index %d", untraced_alias_idx)

    if (traced_alias_idx >= 0)
        LOG("traced alias at index %d", traced_alias_idx)
}

static void process_kiss(kiss_decoder_t *decoder, uint8_t byte, ax25_packet_t *packet)
{
    kiss_message_t kiss_msg;

    int result = kiss_decoder_process(decoder, byte, &kiss_msg);
    if (result <= 0)
        return;

    buffer_t kiss_buf = {
        .data = kiss_msg.data,
        .capacity = kiss_msg.data_length,
        .size = kiss_msg.data_length};

    if (ax25_packet_unpack(packet, &kiss_buf) != AX25_SUCCESS)
    {
        LOG("invalid ax25 packet");
        return;
    }

    process_packet(packet);
}

int main(void)
{
    tcp_client_t client;
    kiss_decoder_t decoder;
    ax25_packet_t packet;

    char tcp_buf_data[TCP_READ_BUF_SIZE];
    buffer_t tcp_buf = {
        .data = tcp_buf_data,
        .capacity = TCP_READ_BUF_SIZE,
        .size = 0};

    _log_level = LOG_LEVEL_VERBOSE;
    _func_pad = -16;

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    kiss_decoder_init(&decoder);
    ax25_addr_init_with(&own_call, "SR5DZ", 0, false);

    num_untraced_aliases = 0;
    configure_untraced_alias("SP", 2);
    configure_untraced_alias("XR", 2);
    configure_untraced_alias("ND", 2);

    num_traced_aliases = 0;
    configure_traced_alias("WIDE", 2);
    configure_traced_alias("TRACE", 2);

    LOG("connecting to TNC at 192.168.0.9:8144...");

    if (tcp_client_init(&client, "192.168.0.9", 8144) < 0)
        EXIT("failed to connect to TNC");

    LOG("connected, waiting for data...");

    while (!g_shutdown_requested)
    {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(client.fd, &fds);

        struct timeval tv = {1, 0};
        int ret = select(client.fd + 1, &fds, NULL, NULL, &tv);
        if (ret < 0)
        {
            LOG("select() failed");
            break;
        }

        if (ret == 0)
            continue;

        int len = tcp_client_listen(&client, &tcp_buf);
        if (len < 0)
        {
            LOG("connection lost");
            break;
        }

        if (len == 0)
            continue;

        for (int i = 0; i < len; i++)
            process_kiss(&decoder, (uint8_t)tcp_buf_data[i], &packet);
    }

    LOG("shutting down...");
    tcp_client_free(&client);
    LOG("shutdown complete");

    return 0;
}

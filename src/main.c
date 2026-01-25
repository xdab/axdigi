#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <common.h>
#include <tcp.h>
#include <kiss.h>
#include <ax25.h>
#include <tnc2.h>
#include <buffer.h>

static uint8_t tnc2_output_buffer[512];
static volatile sig_atomic_t g_shutdown_requested = 0;

static void signal_handler(int sig)
{
    (void)sig;
    g_shutdown_requested = 1;
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
}

int main(void)
{
    tcp_client_t client;
    char tcp_buf[TCP_READ_BUF_SIZE];
    kiss_decoder_t decoder;
    ax25_packet_t packet;

    _log_level = LOG_LEVEL_VERBOSE;
    _func_pad = -16;

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    kiss_decoder_init(&decoder);

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

        int len = tcp_client_process(&client, tcp_buf, sizeof(tcp_buf));
        if (len < 0)
        {
            LOG("connection lost");
            break;
        }

        if (len == 0)
            continue;

        for (int i = 0; i < len; i++)
            process_kiss(&decoder, (uint8_t)tcp_buf[i], &packet);
    }

    LOG("shutting down...");
    tcp_client_free(&client);
    LOG("shutdown complete");

    return 0;
}

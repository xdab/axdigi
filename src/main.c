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
#include <crc.h>
#include <time.h>
#include "alias.h"

#define DEDUPLICATION_SECONDS 30

static ax25_addr_t own_call;
static alias_list_t aliases;

static volatile sig_atomic_t g_shutdown_requested = 0;

static void signal_handler(int sig)
{
    (void)sig;
    g_shutdown_requested = 1;
}

static void log_packet(const char *msg, const ax25_packet_t *packet)
{
    static char tnc2_output_buffer[512];
    static buffer_t tnc2_buf = {
        .data = tnc2_output_buffer,
        .capacity = sizeof(tnc2_output_buffer),
        .size = 0};

    if (tnc2_packet_to_string(packet, &tnc2_buf) < 0)
    {
        LOG("unable to encode packet to tnc2");
        return;
    }

    LOG("%s %s", msg, tnc2_buf.data);
}

static bool digipeat(ax25_packet_t *packet)
{
    if (!strncasecmp(packet->source.callsign, own_call.callsign, AX25_ADDR_MAX_CALLSIGN_LEN) && packet->source.ssid == own_call.ssid)
    {
        LOGV("own packet");
        return false;
    }

    int own_call_idx = -1;
    alias_t *alias = NULL;
    int alias_idx = -1;

    for (int i = 0; i < packet->path_len; i++)
    {
        ax25_addr_t *addr = &packet->path[i];

        if (own_call_idx < 0 && !strncasecmp(addr->callsign, own_call.callsign, AX25_ADDR_MAX_CALLSIGN_LEN) && addr->ssid == own_call.ssid)
            own_call_idx = i;

        if (addr->repeated)
            continue;

        if (!alias && (alias = alias_list_find(&aliases, addr)))
            alias_idx = i;
    }

    if (own_call_idx >= 0)
    {
        LOGV("own call at index %d", own_call_idx);
        ax25_addr_t *own_call_addr = &packet->path[own_call_idx];
        if (own_call_addr->repeated)
        {
            LOGV("already repeated");
            return false;
        }
    }

    if (alias_idx < 0)
    {
        LOGV("no matching digipeating instruction");
        return false;
    }

    LOGV("alias at index %d, %straced", alias_idx, alias->traced ? "" : "un");

    if (0 == packet->path[alias_idx].ssid)
    {
        LOGV("fully used alias");
        return false;
    }

    if (alias->traced)
    {
        if (packet->path_len == AX25_MAX_PATH_LEN)
        {
            LOGV("packet path full");
            return false;
        }

        packet->path[alias_idx + 1] = packet->path[alias_idx];
        if (0 == --packet->path[alias_idx + 1].ssid)
            packet->path[alias_idx + 1].repeated = true;

        packet->path[alias_idx] = own_call;
        packet->path[alias_idx].repeated = true;

        packet->path_len++;
    }
    else // (untraced)
    {
        if (0 == --packet->path[alias_idx].ssid)
            packet->path[alias_idx].repeated = true;
    }

    return true;
}

static bool decode(kiss_decoder_t *decoder, uint8_t byte, ax25_packet_t *packet)
{
    kiss_message_t kiss_msg;

    int result = kiss_decoder_process(decoder, byte, &kiss_msg);
    if (result <= 0)
        return false;

    buffer_t kiss_buf = {
        .data = kiss_msg.data,
        .capacity = kiss_msg.data_length,
        .size = kiss_msg.data_length};

    if (ax25_packet_unpack(packet, &kiss_buf) != AX25_SUCCESS)
    {
        LOG("invalid ax25 packet");
        return false;
    }

    return true;
}

static bool encode(ax25_packet_t *packet, buffer_t *out_buf)
{
    kiss_message_t kiss_msg;
    kiss_msg.command = 0;
    kiss_msg.port = 0;

    buffer_t kiss_buf = {
        .data = kiss_msg.data,
        .capacity = sizeof(kiss_msg.data),
        .size = 0};

    ax25_error_e err = ax25_packet_pack(packet, &kiss_buf);
    if (err != AX25_SUCCESS)
        return false;
    kiss_msg.data_length = kiss_buf.size;

    int result = kiss_encode(&kiss_msg, out_buf->data, out_buf->capacity);
    if (result <= 0)
        return false;
    out_buf->size = result;

    return true;
}

static bool deduplicate(const ax25_packet_t *packet, uint16_t *last_crc, time_t *last_time)
{
    crc_ccitt_t crc;
    crc_ccitt_init(&crc);
    crc_ccitt_update_buffer(&crc, (const uint8_t *)&packet->source, sizeof(ax25_addr_t));
    crc_ccitt_update_buffer(&crc, (const uint8_t *)&packet->destination, sizeof(ax25_addr_t));
    crc_ccitt_update_buffer(&crc, (const uint8_t *)&packet->control, 1);
    crc_ccitt_update_buffer(&crc, (const uint8_t *)&packet->protocol, 1);
    crc_ccitt_update_buffer(&crc, (const uint8_t *)packet->info, packet->info_len);
    uint16_t packet_crc = crc_ccitt_get(&crc);

    time_t now = time(NULL);

    if (packet_crc != *last_crc)
    {
        *last_crc = packet_crc;
        *last_time = now;
        return true;
    }

    time_t time_diff = now - *last_time;
    *last_time = now;
    return time_diff > DEDUPLICATION_SECONDS;
}

int main(void)
{
    tcp_client_t client;
    kiss_decoder_t decoder;
    ax25_packet_t packet;
    uint16_t last_rx_crc, last_tx_crc;
    time_t last_rx_time, last_tx_time;

    char tcp_buf_data[TCP_READ_BUF_SIZE];
    buffer_t tcp_buf = {
        .data = tcp_buf_data,
        .capacity = TCP_READ_BUF_SIZE,
        .size = 0};

    char tcp_send_buf_data[TCP_READ_BUF_SIZE];
    buffer_t tcp_send_buf = {
        .data = tcp_send_buf_data,
        .capacity = TCP_READ_BUF_SIZE,
        .size = 0};

    _log_level = LOG_LEVEL_VERBOSE;
    _func_pad = -16;

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    kiss_decoder_init(&decoder);
    ax25_addr_init_with(&own_call, "SR5DZ", 0, false);
    alias_list_init(&aliases);
    alias_list_add(&aliases, "SP", 2, false);
    alias_list_add(&aliases, "XR", 2, false);
    alias_list_add(&aliases, "ND", 2, false);
    alias_list_add(&aliases, "WIDE", 2, true);
    alias_list_add(&aliases, "TRACE", 2, true);

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
        {
            if (!decode(&decoder, tcp_buf_data[i], &packet))
                continue;

            if (!deduplicate(&packet, &last_rx_crc, &last_rx_time))
            {
                LOGV("duplicate");
                continue;
            }

            log_packet("rx", &packet);

            if (!digipeat(&packet))
                continue;

            if (!deduplicate(&packet, &last_tx_crc, &last_tx_time))
            {
                LOGV("would transmit duplicate");
                continue;
            }

            if (!encode(&packet, &tcp_send_buf))
            {
                LOGV("could not encode packet for tx");
                continue;
            }

            if (tcp_client_send(&client, &tcp_send_buf) < 0)
            {
                LOG("failed to send packet");
                continue;
            }

            log_packet("TX", &packet);
        }
    }

    LOG("shutting down...");
    tcp_client_free(&client);
    LOG("shutdown complete");

    return 0;
}

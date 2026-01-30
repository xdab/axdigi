#include <stdio.h>
#include <stdbool.h>
#include <signal.h>
#include <common.h>
#include <buffer.h>
#include <tcp.h>
#include <time.h>
#include "digipeater.h"
#include "deduplicator.h"
#include "packet.h"

#define TCP_SEND_BUF_SIZE 512

static volatile sig_atomic_t g_shutdown_requested = 0;

static void signal_handler(int sig)
{
    (void)sig;
    g_shutdown_requested = 1;
}

int main(void)
{
    tcp_client_t client;
    kiss_decoder_t decoder;
    ax25_packet_t packet;

    char tcp_buf_data[TCP_READ_BUF_SIZE];
    char tcp_send_buf_data[TCP_SEND_BUF_SIZE];

    buffer_t tcp_buf = {
        .data = tcp_buf_data,
        .capacity = TCP_READ_BUF_SIZE,
        .size = 0};
    buffer_t tcp_send_buf = {
        .data = tcp_send_buf_data,
        .capacity = TCP_SEND_BUF_SIZE,
        .size = 0};

    _log_level = LOG_LEVEL_VERBOSE;
    _func_pad = -16;

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    digipeater_t digipeater;
    deduplicator_t rx_dedup, tx_dedup;

    digipeater_init(&digipeater, "SR5DZ", 0);
    digipeater_add_alias(&digipeater, "SP", 2, false);
    digipeater_add_alias(&digipeater, "XR", 2, false);
    digipeater_add_alias(&digipeater, "ND", 2, false);
    digipeater_add_alias(&digipeater, "WIDE", 2, true);
    digipeater_add_alias(&digipeater, "TRACE", 2, true);

    deduplicator_init(&rx_dedup);
    deduplicator_init(&tx_dedup);

    kiss_decoder_init(&decoder);

    LOG("connecting to TNC at 192.168.0.9:8144...");

    if (tcp_client_init(&client, "192.168.0.9", 8144) < 0)
    {
        LOG("failed to connect to TNC");
        goto SHUTDOWN;
    }

    LOG("connected, waiting for data...");

    while (!g_shutdown_requested)
    {
        int len = tcp_client_listen(&client, &tcp_buf);
        if (len < 0)
        {
            LOG("connection lost");
            goto SHUTDOWN;
        }

        if (len == 0)
            continue;

        for (int i = 0; i < len; i++)
        {
            if (!packet_decode(&decoder, tcp_buf_data[i], &packet))
                continue;

            if (!deduplicator_check(&rx_dedup, &packet))
            {
                LOGV("< duplicate");
                continue;
            }

            packet_log("<", &packet);

            if (!digipeater_process(&digipeater, &packet))
                continue;

            if (!deduplicator_check(&tx_dedup, &packet))
            {
                LOGV("would transmit duplicate");
                continue;
            }

            if (!packet_encode(&packet, &tcp_send_buf))
            {
                LOGV("could not encode packet for tx");
                continue;
            }

            packet_log(">>>>>", &packet);

            if (tcp_client_send(&client, &tcp_send_buf) < 0)
            {
                LOG("failed to send packet");
                goto SHUTDOWN;
            }
        }
    }

SHUTDOWN:
    LOG("shutting down...");
    tcp_client_free(&client);

    return 0;
}

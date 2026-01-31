#include <stdio.h>
#include <stdbool.h>
#include <signal.h>
#include <common.h>
#include <buffer.h>
#include <time.h>
#include "connection.h"
#include "digipeater.h"
#include "deduplicator.h"
#include "packet.h"
#include "options.h"

#define READ_BUF_SIZE 2048
#define SEND_BUF_SIZE 512

static volatile sig_atomic_t g_shutdown_requested = 0;

static void signal_handler(int sig)
{
    (void)sig;
    g_shutdown_requested = 1;
}

int main(int argc, char *argv[])
{
    options_t opts = {0};
    opts_init(&opts);
    opts_parse_args(&opts, argc, argv);
    opts_parse_conf_file(&opts, opts.config_file);
    opts_defaults(&opts);

    _log_level = opts.log_level;
    _func_pad = -18;

    connection_t conn;
    kiss_decoder_t decoder;
    ax25_packet_t packet;

    char buf_data[READ_BUF_SIZE];
    char send_buf_data[SEND_BUF_SIZE];

    buffer_t buf = {
        .data = buf_data,
        .capacity = READ_BUF_SIZE,
        .size = 0};
    buffer_t send_buf = {
        .data = send_buf_data,
        .capacity = SEND_BUF_SIZE,
        .size = 0};

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    digipeater_t digipeater;
    deduplicator_t rx_dedup, tx_dedup;

    digipeater_init(&digipeater, opts.call, opts.ssid);

    opts_alias_t aliases[OPT_MAX_ALIASES];
    int num_aliases = opts_parse_aliases(&opts, aliases, OPT_MAX_ALIASES);
    for (int i = 0; i < num_aliases; i++)
        digipeater_add_alias(&digipeater, aliases[i].name, aliases[i].hops, aliases[i].traced);

    deduplicator_init(&rx_dedup);
    deduplicator_init(&tx_dedup);

    kiss_decoder_init(&decoder);

    if (opts.dry_run)
        LOG("dry run mode enabled, no packets will be transmitted");

    if (opts.socket[0] != '\0')
    {
        LOG("connecting to TNC via Unix socket at %s...", opts.socket);
    }
    else
    {
        LOG("connecting to TNC via TCP at %s:%d...", opts.host, opts.port);
    }

    if (connection_init(&conn, opts.host, opts.port, opts.socket) < 0)
    {
        LOG("failed to connect to TNC");
        goto SHUTDOWN;
    }

    LOG("connected, waiting for data...");

    while (!g_shutdown_requested)
    {
        int len = connection_listen(&conn, &buf);
        if (len < 0)
        {
            LOG("connection lost");
            goto SHUTDOWN;
        }

        if (len == 0)
            continue;

        for (int i = 0; i < len; i++)
        {
            if (!packet_decode(&decoder, buf_data[i], &packet))
                continue;

            if (!deduplicator_check(&rx_dedup, &packet))
            {
                LOGV("< (duplicate)");
                continue;
            }

            packet_log("<", &packet);

            if (!digipeater_process(&digipeater, &packet))
                continue;

            if (!deduplicator_check(&tx_dedup, &packet))
            {
                LOGV("x would transmit duplicate");
                continue;
            }

            if (!packet_encode(&packet, &send_buf))
            {
                LOGV("could not encode packet for tx");
                continue;
            }

            packet_log(opts.dry_run ? "D" : ">", &packet);

            if (!opts.dry_run && connection_send(&conn, &send_buf) < 0)
            {
                LOG("failed to send packet");
                goto SHUTDOWN;
            }
        }
    }

SHUTDOWN:
    LOG("shutting down...");
    connection_free(&conn);

    return 0;
}

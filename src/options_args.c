#include <argp.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "options.h"

static struct argp_option options[] = {
    {OPT_CONFIG, OPT_SHORT_CONFIG, "FILE", 0, "Configuration file", 0},
    {OPT_HOST, OPT_SHORT_HOST, "HOST", 0, "TNC host address", 1},
    {OPT_PORT, OPT_SHORT_PORT, "PORT", 0, "TNC port (default: 8144)", 1},
    {OPT_SOCKET, OPT_SHORT_SOCKET, "PATH", 0, "Unix socket path", 1},

    {OPT_CALL, OPT_SHORT_CALL, "CALL", 0, "Digipeater callsign", 2},
    {OPT_SSID, OPT_SHORT_SSID, "SSID", 0, "Digipeater SSID", 2},

    {OPT_ALIASES_UNTRACED, OPT_SHORT_ALIASES_UNTRACED, "ALIASES", 0, "Comma-separated untraced aliases", 3},
    {OPT_ALIASES_TRACED, OPT_SHORT_ALIASES_TRACED, "ALIASES", 0, "Comma-separated traced aliases", 3},
    {OPT_MAX_UNTRACED_HOPS, OPT_SHORT_MAX_UNTRACED_HOPS, "HOPS", 0, "Max hops for untraced aliases (default: 2)", 3},
    {OPT_MAX_TRACED_HOPS, OPT_SHORT_MAX_TRACED_HOPS, "HOPS", 0, "Max hops for traced aliases (default: 2)", 3},

    {OPT_LOG_LEVEL, OPT_SHORT_LOG_LEVEL, "LEVEL", 0, "Log level: standard, verbose, debug", 4},

    {OPT_DRY_RUN, OPT_SHORT_DRY_RUN, 0, 0, "Don't actually send packets, just log them", 5},

    {0, 0, 0, 0, 0, 0}};

static error_t parse_opt(int key, char *arg, struct argp_state *state)
{
    options_t *opts = state->input;
    switch (key)
    {
    case OPT_SHORT_CONFIG:
        strncpy(opts->config_file, arg, sizeof(opts->config_file) - 1);
        break;
    case OPT_SHORT_HOST:
        strncpy(opts->host, arg, sizeof(opts->host) - 1);
        break;
    case OPT_SHORT_PORT:
        opts->port = atoi(arg);
        break;
    case OPT_SHORT_SOCKET:
        strncpy(opts->socket, arg, sizeof(opts->socket) - 1);
        break;
    case OPT_SHORT_CALL:
        strncpy(opts->call, arg, sizeof(opts->call) - 1);
        break;
    case OPT_SHORT_SSID:
        opts->ssid = atoi(arg);
        break;
    case OPT_SHORT_ALIASES_UNTRACED:
        strncpy(opts->aliases_untraced, arg, sizeof(opts->aliases_untraced) - 1);
        break;
    case OPT_SHORT_ALIASES_TRACED:
        strncpy(opts->aliases_traced, arg, sizeof(opts->aliases_traced) - 1);
        break;
    case OPT_SHORT_MAX_UNTRACED_HOPS:
        opts->max_untraced_hops = atoi(arg);
        break;
    case OPT_SHORT_MAX_TRACED_HOPS:
        opts->max_traced_hops = atoi(arg);
        break;
    case OPT_SHORT_LOG_LEVEL:
        if (strcmp(arg, OPT_VAL_LOG_VERBOSE) == 0)
            opts->log_level = LOG_LEVEL_VERBOSE;
        else if (strcmp(arg, OPT_VAL_LOG_DEBUG) == 0)
            opts->log_level = LOG_LEVEL_DEBUG;
        else
            opts->log_level = LOG_LEVEL_STANDARD;
        break;
    case OPT_SHORT_DRY_RUN:
        opts->dry_run = true;
        break;
    case ARGP_KEY_NO_ARGS:
        break;
    default:
        return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

struct argp argp = {
    options,
    parse_opt,
    "",
    "AX.25 Digipeater"};

void opts_parse_args(options_t *opts, int argc, char *argv[])
{
    nonnull(opts, "opts");
    nonzero(argc, "argc");
    nonnull(argv, "argv");

    argp_parse(&argp, argc, argv, 0, 0, opts);
}

#pragma once

#include "common.h"
#include <stdbool.h>

#define OPT_CONFIG "config"
#define OPT_SHORT_CONFIG 'c'

#define OPT_HOST "host"
#define OPT_PORT "port"
#define OPT_SOCKET "socket"
#define OPT_CALL "call"
#define OPT_SSID "ssid"
#define OPT_ALIASES_UNTRACED "aliases-untraced"
#define OPT_ALIASES_TRACED "aliases-traced"
#define OPT_MAX_UNTRACED_HOPS "max-untraced-hops"
#define OPT_MAX_TRACED_HOPS "max-traced-hops"
#define OPT_LOG_LEVEL "log-level"

#define OPT_VAL_LOG_STANDARD "standard"
#define OPT_VAL_LOG_VERBOSE "verbose"
#define OPT_VAL_LOG_DEBUG "debug"

#define OPT_DRY_RUN "dry-run"
#define OPT_SHORT_DRY_RUN 'n'

#define OPT_SHORT_HOST 'h'
#define OPT_SHORT_PORT 'p'
#define OPT_SHORT_SOCKET 'x'
#define OPT_SHORT_CALL 'C'
#define OPT_SHORT_SSID 's'
#define OPT_SHORT_ALIASES_UNTRACED 'u'
#define OPT_SHORT_ALIASES_TRACED 't'
#define OPT_SHORT_MAX_UNTRACED_HOPS 'U'
#define OPT_SHORT_MAX_TRACED_HOPS 'T'
#define OPT_SHORT_LOG_LEVEL 'v'

#define OPT_STR_SIZE 256
#define OPT_MAX_ALIASES 32

typedef struct
{
    char name[8];
    int hops;
    bool traced;
} opts_alias_t;

typedef struct options
{
    char config_file[OPT_STR_SIZE];
    char host[64];
    int port;
    char socket[OPT_STR_SIZE];

    char call[8];
    int ssid;
    
    char aliases_untraced[OPT_STR_SIZE];
    char aliases_traced[OPT_STR_SIZE];
    int max_untraced_hops;
    int max_traced_hops;
    
    log_level_e log_level;
    bool dry_run;
} options_t;

// Clears out options_t setting null/zero values
void opts_init(options_t *opts);

// Parses command line arguments to options_t, OVERWRITING ALL VALUES
void opts_parse_args(options_t *opts, int argc, char *argv[]);

// Parses file to options_t, overwriting only null/zero/empty values
void opts_parse_conf_file(options_t *opts, const char *filename);

// Applies default values, overwriting only null/zero/empty values
void opts_defaults(options_t *opts);

// Parse comma-separated aliases from options into an array
int opts_parse_aliases(const options_t *opts, opts_alias_t *aliases, int max_aliases);

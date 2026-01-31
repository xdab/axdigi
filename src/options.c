#include "options.h"
#include <string.h>
#include <ctype.h>
#include <limits.h>

void opts_init(options_t *opts)
{
    nonnull(opts, "opts");

    opts->config_file[0] = '\0';
    opts->host[0] = '\0';
    opts->port = 0;

    opts->call[0] = '\0';
    opts->ssid = 0;

    opts->aliases_untraced[0] = '\0';
    opts->aliases_traced[0] = '\0';
    opts->max_untraced_hops = 0;
    opts->max_traced_hops = 0;

    opts->log_level = LOG_LEVEL_STANDARD;
    opts->dry_run = false;
}

void opts_defaults(options_t *opts)
{
    nonnull(opts, "opts");

    REPLACE_IF_a_WITH_b(opts->port, 0, 8144);
    REPLACE_IF_a_WITH_b(opts->max_untraced_hops, 0, 2);
    REPLACE_IF_a_WITH_b(opts->max_traced_hops, 0, 2);
}

int opts_parse_aliases(const options_t *opts, opts_alias_t *aliases, int max_aliases)
{
    nonnull(opts, "opts");
    nonnull(aliases, "aliases");

    int count = 0;
    char *saveptr;
    char *token;

    // Parse untraced aliases
    if (opts->aliases_untraced[0] != '\0' && count < max_aliases)
    {
        char buf[OPT_STR_SIZE];
        strncpy(buf, opts->aliases_untraced, OPT_STR_SIZE - 1);
        buf[OPT_STR_SIZE - 1] = '\0';

        token = strtok_r(buf, ",", &saveptr);
        while (token != NULL && count < max_aliases)
        {
            // Trim whitespace
            while (*token == ' ')
                token++;
            char *end = token + strlen(token) - 1;
            while (end > token && *end == ' ')
                *end-- = '\0';

            strncpy(aliases[count].name, token, sizeof(aliases[count].name) - 1);
            aliases[count].name[sizeof(aliases[count].name) - 1] = '\0';
            aliases[count].hops = opts->max_untraced_hops;
            aliases[count].traced = false;
            count++;

            token = strtok_r(NULL, ",", &saveptr);
        }
    }

    // Parse traced aliases
    if (opts->aliases_traced[0] != '\0' && count < max_aliases)
    {
        char buf[OPT_STR_SIZE];
        strncpy(buf, opts->aliases_traced, OPT_STR_SIZE - 1);
        buf[OPT_STR_SIZE - 1] = '\0';

        token = strtok_r(buf, ",", &saveptr);
        while (token != NULL && count < max_aliases)
        {
            // Trim whitespace
            while (*token == ' ')
                token++;
            char *end = token + strlen(token) - 1;
            while (end > token && *end == ' ')
                *end-- = '\0';

            strncpy(aliases[count].name, token, sizeof(aliases[count].name) - 1);
            aliases[count].name[sizeof(aliases[count].name) - 1] = '\0';
            aliases[count].hops = opts->max_traced_hops;
            aliases[count].traced = true;
            count++;

            token = strtok_r(NULL, ",", &saveptr);
        }
    }

    return count;
}

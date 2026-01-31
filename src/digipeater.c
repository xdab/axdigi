#include "digipeater.h"
#include <string.h>

void digipeater_init(digipeater_t *d, const char *call, int ssid)
{
    ax25_addr_init_with(&d->own_call, call, ssid, false);
    alias_list_init(&d->aliases);
}

void digipeater_add_alias(digipeater_t *d, const char *call, int max_hops, bool traced)
{
    alias_list_add(&d->aliases, call, max_hops, traced);
}

bool digipeater_process(digipeater_t *d, ax25_packet_t *packet)
{
    if (!strncasecmp(packet->source.callsign, d->own_call.callsign, AX25_ADDR_MAX_CALLSIGN_LEN) && packet->source.ssid == d->own_call.ssid)
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

        if (own_call_idx < 0 && !strncasecmp(addr->callsign, d->own_call.callsign, AX25_ADDR_MAX_CALLSIGN_LEN) && addr->ssid == d->own_call.ssid)
            own_call_idx = i;

        if (addr->repeated)
            continue;

        if (!alias && (alias = alias_list_find(&d->aliases, addr)))
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

        own_call_addr->repeated = true;
        return true;
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

        packet->path[alias_idx] = d->own_call;
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

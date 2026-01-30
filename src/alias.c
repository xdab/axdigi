#include "alias.h"
#include <string.h>

void alias_init(alias_t *alias, const char *call, int hops, bool traced)
{
    char tmp[16] = {0};
    snprintf(tmp, 16, "%s%d", call, hops);
    ax25_addr_init_with(&alias->addr, tmp, 0, false);
    alias->hops = hops;
    alias->traced = traced;
}

bool alias_compare(const alias_t *alias, const ax25_addr_t *addr)
{
    return !strncasecmp(alias->addr.callsign, addr->callsign, AX25_ADDR_MAX_CALLSIGN_LEN);
}

void alias_list_init(alias_list_t *alist)
{
    alist->num_aliases = 0;
}

int alias_list_add(alias_list_t *alist, const char *call, int max_hops, bool traced)
{
    for (int hops = max_hops; hops >= 1; hops--)
    {
        if (alist->num_aliases == MAX_ALIASES)
            return -1;
        alias_t *a_ptr = &alist->aliases[alist->num_aliases++];
        alias_init(a_ptr, call, hops, traced);
    }
    return 0;
}

alias_t *alias_list_find(const alias_list_t *alist, const ax25_addr_t *addr)
{
    for (int i = 0; i < alist->num_aliases; i++)
    {
        alias_t *a_ptr = (alias_t *)&alist->aliases[i];
        if (alias_compare(a_ptr, addr))
            return a_ptr;
    }
    return NULL;
}
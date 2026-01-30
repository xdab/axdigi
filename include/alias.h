#pragma once

#include <stdbool.h>
#include <ax25.h>

#define MAX_ALIASES 32

typedef struct
{
    ax25_addr_t addr;
    int hops;
    bool traced;

} alias_t;

void alias_init(alias_t *alias, const char *call, int hops, bool traced);

bool alias_compare(const alias_t *alias, const ax25_addr_t *addr);

typedef struct
{
    alias_t aliases[MAX_ALIASES];
    int num_aliases;

} alias_list_t;

void alias_list_init(alias_list_t *alist);

int alias_list_add(alias_list_t *alist, const char *call, int max_hops, bool traced);

alias_t *alias_list_find(const alias_list_t *alist, const ax25_addr_t *addr);

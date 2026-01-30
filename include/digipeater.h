#pragma once

#include <stdbool.h>
#include <ax25.h>
#include "alias.h"

typedef struct
{
    ax25_addr_t own_call;
    alias_list_t aliases;

} digipeater_t;

void digipeater_init(digipeater_t *d, const char *call, int ssid);
void digipeater_add_alias(digipeater_t *d, const char *call, int max_hops, bool traced);
bool digipeater_process(digipeater_t *d, ax25_packet_t *packet);

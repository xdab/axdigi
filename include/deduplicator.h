#pragma once

#include <stdbool.h>
#include <time.h>
#include <ax25.h>

#define DEDUPLICATION_SECONDS 30

typedef struct
{
    uint16_t last_crc;
    time_t last_time;

} deduplicator_t;

void deduplicator_init(deduplicator_t *d);
bool deduplicator_check(deduplicator_t *d, const ax25_packet_t *packet);
bool deduplicator_check_at(deduplicator_t *d, const ax25_packet_t *packet, time_t time);

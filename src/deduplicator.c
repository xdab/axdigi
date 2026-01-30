#include "deduplicator.h"
#include <crc.h>

void deduplicator_init(deduplicator_t *d)
{
    d->last_crc = 0;
    d->last_time = 0;
}

bool deduplicator_check(deduplicator_t *d, const ax25_packet_t *packet)
{
    crc_ccitt_t crc;
    crc_ccitt_init(&crc);
    crc_ccitt_update_buffer(&crc, (const uint8_t *)&packet->source, sizeof(ax25_addr_t));
    crc_ccitt_update_buffer(&crc, (const uint8_t *)&packet->destination, sizeof(ax25_addr_t));
    crc_ccitt_update_buffer(&crc, (const uint8_t *)&packet->control, 1);
    crc_ccitt_update_buffer(&crc, (const uint8_t *)&packet->protocol, 1);
    crc_ccitt_update_buffer(&crc, (const uint8_t *)packet->info, packet->info_len);
    uint16_t packet_crc = crc_ccitt_get(&crc);

    time_t now = time(NULL);

    if (packet_crc != d->last_crc)
    {
        d->last_crc = packet_crc;
        d->last_time = now;
        return true;
    }

    time_t time_diff = now - d->last_time;
    d->last_time = now;
    return time_diff > DEDUPLICATION_SECONDS;
}

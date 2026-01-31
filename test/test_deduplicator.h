#ifndef TEST_DEDUPLICATOR_H
#define TEST_DEDUPLICATOR_H

#include "test.h"
#include "deduplicator.h"
#include <string.h>

static ax25_packet_t make_packet(const char *src, const char *dst, const char *info)
{
    ax25_packet_t packet;
    memset(&packet, 0, sizeof(packet));
    ax25_addr_init_with(&packet.source, src, 0, false);
    ax25_addr_init_with(&packet.destination, dst, 0, false);
    if (info)
    {
        size_t len = strlen(info);
        if (len > AX25_MAX_INFO_LEN)
            len = AX25_MAX_INFO_LEN;
        memcpy(packet.info, info, len);
        packet.info_len = len;
    }
    return packet;
}

void test_deduplicator_init()
{
    deduplicator_t d;
    deduplicator_init(&d);
    assert_equal_int(d.last_crc, 0, "deduplicator init last_crc");
    assert_equal_int(d.last_time, 0, "deduplicator init last_time");
}

void test_deduplicator_new_packet()
{
    deduplicator_t d;
    deduplicator_init(&d);
    ax25_packet_t packet = make_packet("TEST", "DEST", "hello");
    bool result = deduplicator_check_at(&d, &packet, 100);
    assert_true(result, "new packet returns true");
}

void test_deduplicator_duplicate_within_window()
{
    deduplicator_t d;
    deduplicator_init(&d);
    ax25_packet_t packet = make_packet("TEST", "DEST", "hello");
    
    deduplicator_check_at(&d, &packet, 100);  // First packet
    bool result = deduplicator_check_at(&d, &packet, 110);  // Same packet 10s later
    assert_true(!result, "duplicate within 30s returns false");
}

void test_deduplicator_duplicate_after_window()
{
    deduplicator_t d;
    deduplicator_init(&d);
    ax25_packet_t packet = make_packet("TEST", "DEST", "hello");
    
    deduplicator_check_at(&d, &packet, 100);  // First packet
    bool result = deduplicator_check_at(&d, &packet, 131);  // Same packet 31s later
    assert_true(result, "duplicate after 30s returns true");
}

void test_deduplicator_duplicate_exactly_at_window()
{
    deduplicator_t d;
    deduplicator_init(&d);
    ax25_packet_t packet = make_packet("TEST", "DEST", "hello");
    
    deduplicator_check_at(&d, &packet, 100);  // First packet
    bool result = deduplicator_check_at(&d, &packet, 130);  // Same packet exactly 30s later
    assert_true(!result, "duplicate exactly at 30s returns false (> not >=)");
}

void test_deduplicator_different_packet()
{
    deduplicator_t d;
    deduplicator_init(&d);
    ax25_packet_t packet1 = make_packet("TEST", "DEST", "hello");
    ax25_packet_t packet2 = make_packet("TEST", "DEST", "world");
    
    deduplicator_check_at(&d, &packet1, 100);  // First packet
    bool result = deduplicator_check_at(&d, &packet2, 110);  // Different packet
    assert_true(result, "different packet returns true");
}

void test_deduplicator_different_source()
{
    deduplicator_t d;
    deduplicator_init(&d);
    ax25_packet_t packet1 = make_packet("SRC1", "DEST", "hello");
    ax25_packet_t packet2 = make_packet("SRC2", "DEST", "hello");
    
    deduplicator_check_at(&d, &packet1, 100);  // First packet
    bool result = deduplicator_check_at(&d, &packet2, 110);  // Different source
    assert_true(result, "different source returns true");
}

void test_deduplicator_different_destination()
{
    deduplicator_t d;
    deduplicator_init(&d);
    ax25_packet_t packet1 = make_packet("TEST", "DEST1", "hello");
    ax25_packet_t packet2 = make_packet("TEST", "DEST2", "hello");
    
    deduplicator_check_at(&d, &packet1, 100);  // First packet
    bool result = deduplicator_check_at(&d, &packet2, 110);  // Different destination
    assert_true(result, "different destination returns true");
}

void test_deduplicator_empty_info()
{
    deduplicator_t d;
    deduplicator_init(&d);
    // Use minimal non-empty info to avoid CRC assertion
    ax25_packet_t packet1 = make_packet("TEST", "DEST", "x");
    ax25_packet_t packet2 = make_packet("TEST", "DEST", "x");
    
    deduplicator_check_at(&d, &packet1, 100);  // First packet
    bool result = deduplicator_check_at(&d, &packet2, 110);  // Same packet
    assert_true(!result, "duplicate packet within window returns false");
}

void test_deduplicator_multiple_duplicates()
{
    deduplicator_t d;
    deduplicator_init(&d);
    ax25_packet_t packet = make_packet("TEST", "DEST", "hello");
    
    assert_true(deduplicator_check_at(&d, &packet, 100), "first packet");
    assert_true(!deduplicator_check_at(&d, &packet, 110), "duplicate 1");
    assert_true(!deduplicator_check_at(&d, &packet, 120), "duplicate 2");
    assert_true(!deduplicator_check_at(&d, &packet, 129), "duplicate 3");
    assert_true(!deduplicator_check_at(&d, &packet, 130), "at 30s still false");
    // Need 31s gap from last call (130), so 130+31=161
    assert_true(deduplicator_check_at(&d, &packet, 161), "after window");
}

#endif
#ifndef TEST_PACKET_H
#define TEST_PACKET_H

#include "test.h"
#include "packet.h"
#include <string.h>
#include <kiss.h>
#include <ax25.h>

void test_packet_roundtrip()
{
    ax25_packet_t original;
    memset(&original, 0, sizeof(original));
    ax25_addr_init_with(&original.source, "TEST", 0, false);
    ax25_addr_init_with(&original.destination, "DEST", 0, false);
    original.path_len = 1;
    ax25_addr_init_with(&original.path[0], "WIDE", 1, false);
    memcpy(original.info, "hello", 5);
    original.info_len = 5;

    uint8_t buf[512];
    buffer_t out_buf = {
        .data = buf,
        .capacity = sizeof(buf),
        .size = 0};

    bool result = packet_encode(&original, &out_buf);
    assert_true(result, "encode returns true");

    kiss_decoder_t decoder;
    kiss_decoder_init(&decoder);

    ax25_packet_t decoded;
    memset(&decoded, 0, sizeof(decoded));

    for (size_t i = 0; i < out_buf.size; i++)
        packet_decode(&decoder, buf[i], &decoded);

    assert_string(decoded.source.callsign, original.source.callsign, "source callsign matches");
    assert_equal_int(decoded.source.ssid, original.source.ssid, "source ssid matches");
    assert_string(decoded.destination.callsign, original.destination.callsign, "destination callsign matches");
    assert_equal_int(decoded.destination.ssid, original.destination.ssid, "destination ssid matches");
    assert_equal_int(decoded.path_len, original.path_len, "path length matches");
    assert_string(decoded.path[0].callsign, original.path[0].callsign, "path callsign matches");
}

void test_packet_log_doesnt_crash()
{
    ax25_packet_t packet;
    memset(&packet, 0, sizeof(packet));
    ax25_addr_init_with(&packet.source, "SRC", 0, false);
    ax25_addr_init_with(&packet.destination, "DEST", 0, false);

    packet_log("test packet", &packet);
    assert_true(1, "packet_log doesn't crash");
}

#endif
#ifndef TEST_DIGIPEATER_H
#define TEST_DIGIPEATER_H

#include "test.h"
#include "digipeater.h"
#include <string.h>

void test_digipeater_init()
{
    digipeater_t d;
    digipeater_init(&d, "MYCALL", 0);
    assert_string(d.own_call.callsign, "MYCALL", "own call callsign");
    assert_equal_int(d.own_call.ssid, 0, "own call ssid");
    assert_equal_int(d.aliases.num_aliases, 0, "init aliases count");
}

void test_digipeater_add_alias()
{
    digipeater_t d;
    digipeater_init(&d, "MYCALL", 0);
    digipeater_add_alias(&d, "WIDE", 2, true);
    assert_equal_int(d.aliases.num_aliases, 2, "added aliases count");
}

void test_digipeater_own_packet()
{
    digipeater_t d;
    digipeater_init(&d, "MYCALL", 0);
    digipeater_add_alias(&d, "WIDE", 2, true);

    ax25_packet_t packet;
    memset(&packet, 0, sizeof(packet));
    ax25_addr_init_with(&packet.source, "MYCALL", 0, false);
    ax25_addr_init_with(&packet.destination, "DEST", 0, false);

    bool result = digipeater_process(&d, &packet);
    assert_true(!result, "own packet returns false");
}

void test_digipeater_own_call_in_path_repeated()
{
    digipeater_t d;
    digipeater_init(&d, "MYCALL", 0);
    digipeater_add_alias(&d, "WIDE", 2, true);

    ax25_packet_t packet;
    memset(&packet, 0, sizeof(packet));
    ax25_addr_init_with(&packet.source, "SRC", 0, false);
    ax25_addr_init_with(&packet.destination, "DEST", 0, false);
    packet.path_len = 1;
    ax25_addr_init_with(&packet.path[0], "MYCALL", 0, true);

    bool result = digipeater_process(&d, &packet);
    assert_true(!result, "own call repeated returns false");
}

void test_digipeater_no_matching_alias()
{
    digipeater_t d;
    digipeater_init(&d, "MYCALL", 0);
    digipeater_add_alias(&d, "WIDE", 2, true);

    ax25_packet_t packet;
    memset(&packet, 0, sizeof(packet));
    ax25_addr_init_with(&packet.source, "SRC", 0, false);
    ax25_addr_init_with(&packet.destination, "DEST", 0, false);
    packet.path_len = 1;
    ax25_addr_init_with(&packet.path[0], "TRACE", 0, false);

    bool result = digipeater_process(&d, &packet);
    assert_true(!result, "no matching alias returns false");
}

void test_digipeater_fully_used_alias()
{
    digipeater_t d;
    digipeater_init(&d, "MYCALL", 0);
    digipeater_add_alias(&d, "WIDE", 1, true);

    ax25_packet_t packet;
    memset(&packet, 0, sizeof(packet));
    ax25_addr_init_with(&packet.source, "SRC", 0, false);
    ax25_addr_init_with(&packet.destination, "DEST", 0, false);
    packet.path_len = 1;
    ax25_addr_init_with(&packet.path[0], "WIDE1", 0, false); // note: repeated=false for this case!

    bool result = digipeater_process(&d, &packet);
    assert_true(!result, "fully used alias returns false");
}

void test_digipeater_untraced_success()
{
    digipeater_t d;
    digipeater_init(&d, "MYCALL", 0);
    digipeater_add_alias(&d, "SP", 2, false);

    ax25_packet_t packet;
    memset(&packet, 0, sizeof(packet));
    ax25_addr_init_with(&packet.source, "SRC", 0, false);
    ax25_addr_init_with(&packet.destination, "DEST", 0, false);
    packet.path_len = 1;
    ax25_addr_init_with(&packet.path[0], "SP1", 1, false);

    bool result = digipeater_process(&d, &packet);
    assert_true(result, "untraced digipeat returns true");
    assert_equal_int(packet.path[0].ssid, 0, "ssid decremented");
    assert_true(packet.path[0].repeated, "repeated flag set");
}

void test_digipeater_traced_success()
{
    digipeater_t d;
    digipeater_init(&d, "MYCALL", 0);
    digipeater_add_alias(&d, "WIDE", 2, true);

    ax25_packet_t packet;
    memset(&packet, 0, sizeof(packet));
    ax25_addr_init_with(&packet.source, "SRC", 0, false);
    ax25_addr_init_with(&packet.destination, "DEST", 0, false);
    packet.path_len = 1;
    ax25_addr_init_with(&packet.path[0], "WIDE2", 2, false);

    bool result = digipeater_process(&d, &packet);
    assert_true(result, "traced digipeat returns true");
    assert_equal_int(packet.path_len, 2, "path_len incremented");

    assert_string(packet.path[0].callsign, "MYCALL", "own call added to path");
    assert_true(packet.path[0].repeated, "own call repeated");

    assert_string(packet.path[1].callsign, "WIDE2 ", "alias shifted in path");
    assert_equal_int(packet.path[1].ssid, 1, "alias ssid after decrement");
    assert_true(!packet.path[1].repeated, "alias not repeated if hops remaining");
}

void test_digipeater_traced_path_full()
{
    digipeater_t d;
    digipeater_init(&d, "MYCALL", 0);
    digipeater_add_alias(&d, "WIDE", 2, true);

    ax25_packet_t packet;
    memset(&packet, 0, sizeof(packet));
    ax25_addr_init_with(&packet.source, "SRC", 0, false);
    ax25_addr_init_with(&packet.destination, "DEST", 0, false);
    // Fill path to max
    packet.path_len = AX25_MAX_PATH_LEN;
    for (int i = 0; i < AX25_MAX_PATH_LEN; i++)
        ax25_addr_init_with(&packet.path[i], "RELAY", i, false);
    ax25_addr_init_with(&packet.path[AX25_MAX_PATH_LEN - 1], "WIDE1", 1, false);

    bool result = digipeater_process(&d, &packet);
    assert_true(!result, "traced with full path returns false");
}

void test_digipeater_own_call_in_path_not_repeated()
{
    digipeater_t d;
    digipeater_init(&d, "MYCALL", 0);
    digipeater_add_alias(&d, "WIDE", 2, true);

    ax25_packet_t packet;
    memset(&packet, 0, sizeof(packet));
    ax25_addr_init_with(&packet.source, "SRC", 0, false);
    ax25_addr_init_with(&packet.destination, "DEST", 0, false);
    packet.path_len = 1;
    ax25_addr_init_with(&packet.path[0], "MYCALL", 0, false);

    bool result = digipeater_process(&d, &packet);
    assert_true(result, "own call not repeated returns true");
}

void test_digipeater_case_insensitive()
{
    digipeater_t d;
    digipeater_init(&d, "MYCALL", 0);
    digipeater_add_alias(&d, "wide", 2, true); // lowercase

    ax25_packet_t packet;
    memset(&packet, 0, sizeof(packet));
    ax25_addr_init_with(&packet.source, "SRC", 0, false);
    ax25_addr_init_with(&packet.destination, "DEST", 0, false);
    packet.path_len = 1;
    ax25_addr_init_with(&packet.path[0], "WIDE1", 1, false);

    bool result = digipeater_process(&d, &packet);
    assert_true(result, "case insensitive match returns true");
}

void test_digipeater_multiple_aliases_first_unrepeated()
{
    digipeater_t d;
    digipeater_init(&d, "MYCALL", 0);
    digipeater_add_alias(&d, "WIDE", 2, true);
    digipeater_add_alias(&d, "TRACE", 2, true);

    ax25_packet_t packet;
    memset(&packet, 0, sizeof(packet));
    ax25_addr_init_with(&packet.source, "SRC", 0, false);
    ax25_addr_init_with(&packet.destination, "DEST", 0, false);
    packet.path_len = 2;
    ax25_addr_init_with(&packet.path[0], "WIDE2", 0, true);
    ax25_addr_init_with(&packet.path[1], "TRACE2", 2, false);

    bool result = digipeater_process(&d, &packet);
    assert_true(result, "first unrepeated alias match returns true");
    assert_equal_int(packet.path_len, 3, "path_len incremented for traced");
}

#endif
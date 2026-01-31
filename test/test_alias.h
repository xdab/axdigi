#ifndef TEST_ALIAS_H
#define TEST_ALIAS_H

#include "test.h"
#include "alias.h"
#include <string.h>

void test_alias_init()
{
    alias_t alias;
    alias_init(&alias, "TEST", 3, true);
    assert_string(alias.addr.callsign, "TEST3 ", "alias init callsign");
    assert_equal_int(alias.addr.ssid, 0, "alias init ssid (always 0)");
    assert_equal_int(alias.hops, 3, "alias init hops");
    assert_true(alias.traced, "alias init traced");
}

void test_alias_compare_match()
{
    alias_t alias;
    alias_init(&alias, "TEST", 1, false);
    ax25_addr_t addr;
    ax25_addr_init_with(&addr, "TEST", 1, false);
    // This should NOT match because alias callsign is "TEST1 " and addr is "TEST  "
    assert_true(!alias_compare(&alias, &addr), "alias compare different (concat vs ssid)");
}

void test_alias_compare_same_concat()
{
    alias_t alias;
    alias_init(&alias, "TEST", 1, false);
    ax25_addr_t addr;
    ax25_addr_init_with(&addr, "TEST1", 0, false);
    assert_true(alias_compare(&alias, &addr), "alias compare same concat");
}

void test_alias_compare_different_callsign()
{
    alias_t alias;
    alias_init(&alias, "TEST", 1, false);
    ax25_addr_t addr;
    ax25_addr_init_with(&addr, "DIFF", 1, false);
    assert_true(!alias_compare(&alias, &addr), "alias compare different callsign");
}

void test_alias_compare_different_ssid()
{
    alias_t alias;
    alias_init(&alias, "TEST", 1, false);
    ax25_addr_t addr;
    ax25_addr_init_with(&addr, "TEST", 2, false);
    assert_true(!alias_compare(&alias, &addr), "alias compare different ssid");
}

void test_alias_list_init()
{
    alias_list_t alist;
    alias_list_init(&alist);
    assert_equal_int(alist.num_aliases, 0, "alias list init count");
}

void test_alias_list_add_single()
{
    alias_list_t alist;
    alias_list_init(&alist);
    int result = alias_list_add(&alist, "TEST", 1, false);
    assert_equal_int(result, 0, "alias list add single result");
    assert_equal_int(alist.num_aliases, 1, "alias list add single count");
}

void test_alias_list_add_multiple_hops()
{
    alias_list_t alist;
    alias_list_init(&alist);
    int result = alias_list_add(&alist, "TEST", 3, true);
    assert_equal_int(result, 0, "alias list add multiple result");
    assert_equal_int(alist.num_aliases, 3, "alias list add multiple count");
    assert_equal_int(alist.aliases[0].hops, 3, "alias list first hop");
    assert_equal_int(alist.aliases[1].hops, 2, "alias list second hop");
    assert_equal_int(alist.aliases[2].hops, 1, "alias list third hop");
}

void test_alias_list_add_max_aliases()
{
    alias_list_t alist;
    alias_list_init(&alist);
    // Add aliases until we hit the limit
    for (int i = 0; i < MAX_ALIASES; i++)
    {
        int result = alias_list_add(&alist, "TEST", 1, false);
        assert_equal_int(result, 0, "alias list add within limit");
    }
    // Next add should fail
    int result = alias_list_add(&alist, "TEST", 1, false);
    assert_equal_int(result, -1, "alias list add at max");
    assert_equal_int(alist.num_aliases, MAX_ALIASES, "alias list max count");
}

void test_alias_list_find_existing()
{
    alias_list_t alist;
    alias_list_init(&alist);
    alias_list_add(&alist, "TEST", 2, false);
    // Find by matching the callsign that was created (TEST2)
    ax25_addr_t addr;
    ax25_addr_init_with(&addr, "TEST2", 0, false);
    alias_t *found = alias_list_find(&alist, &addr);
    assert_true(found != NULL, "alias list find existing");
    assert_equal_int(found->hops, 2, "alias list find hops");
}

void test_alias_list_find_not_existing()
{
    alias_list_t alist;
    alias_list_init(&alist);
    alias_list_add(&alist, "TEST", 1, false);
    ax25_addr_t addr;
    ax25_addr_init_with(&addr, "NOSUCH", 1, false);
    alias_t *found = alias_list_find(&alist, &addr);
    assert_true(found == NULL, "alias list find not existing");
}

void test_alias_list_find_with_ssid()
{
    alias_list_t alist;
    alias_list_init(&alist);
    alias_list_add(&alist, "TEST", 3, true);
    // Find TEST-2 (should exist because alias_list_add creates TEST2)
    ax25_addr_t addr;
    ax25_addr_init_with(&addr, "TEST2", 0, false);
    alias_t *found = alias_list_find(&alist, &addr);
    assert_true(found != NULL, "alias list find with ssid");
    assert_equal_int(found->hops, 2, "alias list find ssid hops");
    assert_true(found->traced, "alias list find traced");
}

#endif
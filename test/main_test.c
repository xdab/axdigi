#include "test.h"
#include "test_alias.h"
#include "test_deduplicator.h"
#include "test_digipeater.h"
#include "test_packet.h"

int main(void)
{
    begin_suite();

    begin_module("Alias");
    test_alias_init();
    test_alias_compare_match();
    test_alias_compare_different_callsign();
    test_alias_compare_different_ssid();
    test_alias_list_init();
    test_alias_list_add_single();
    test_alias_list_add_multiple_hops();
    test_alias_list_add_max_aliases();
    test_alias_list_find_existing();
    test_alias_list_find_not_existing();
    test_alias_list_find_with_ssid();
    end_module();

    begin_module("Deduplicator");
    test_deduplicator_init();
    test_deduplicator_new_packet();
    test_deduplicator_duplicate_within_window();
    test_deduplicator_duplicate_after_window();
    test_deduplicator_duplicate_exactly_at_window();
    test_deduplicator_different_packet();
    test_deduplicator_different_source();
    test_deduplicator_different_destination();
    test_deduplicator_empty_info();
    test_deduplicator_multiple_duplicates();
    end_module();

    begin_module("Digipeater");
    test_digipeater_init();
    test_digipeater_add_alias();
    test_digipeater_own_packet();
    test_digipeater_own_call_in_path_repeated();
    test_digipeater_no_matching_alias();
    test_digipeater_fully_used_alias();
    test_digipeater_untraced_success();
    test_digipeater_traced_success();
    test_digipeater_traced_path_full();
    test_digipeater_own_call_in_path_not_repeated();
    test_digipeater_case_insensitive();
    test_digipeater_multiple_aliases_first_unrepeated();
    end_module();

    begin_module("Packet");
    test_packet_roundtrip();
    test_packet_log_doesnt_crash();
    end_module();

    int failed = end_suite();

    return failed ? 1 : 0;
}

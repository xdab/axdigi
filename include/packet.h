#pragma once

#include <stdbool.h>
#include <ax25.h>
#include <kiss.h>
#include <buffer.h>

void packet_log(const char *msg, const ax25_packet_t *packet);
bool packet_decode(kiss_decoder_t *decoder, uint8_t byte, ax25_packet_t *packet);
bool packet_encode(ax25_packet_t *packet, buffer_t *out_buf);

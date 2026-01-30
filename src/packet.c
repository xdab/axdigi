#include "packet.h"
#include <tnc2.h>

static char tnc2_buffer[512];
static buffer_t tnc2_buf = {
    .data = tnc2_buffer,
    .capacity = sizeof(tnc2_buffer),
    .size = 0};

void packet_log(const char *msg, const ax25_packet_t *packet)
{
    tnc2_buf.size = 0;
    if (tnc2_packet_to_string(packet, &tnc2_buf) < 0)
    {
        LOG("unable to encode packet to tnc2");
        return;
    }

    LOG("%s %s", msg, tnc2_buf.data);
}

bool packet_decode(kiss_decoder_t *decoder, uint8_t byte, ax25_packet_t *packet)
{
    kiss_message_t kiss_msg;

    int result = kiss_decoder_process(decoder, byte, &kiss_msg);
    if (result <= 0)
        return false;

    buffer_t kiss_buf = {
        .data = kiss_msg.data,
        .capacity = kiss_msg.data_length,
        .size = kiss_msg.data_length};

    if (ax25_packet_unpack(packet, &kiss_buf) != AX25_SUCCESS)
    {
        LOG("invalid ax25 packet");
        return false;
    }

    return true;
}

bool packet_encode(ax25_packet_t *packet, buffer_t *out_buf)
{
    kiss_message_t kiss_msg;
    kiss_msg.command = 0;
    kiss_msg.port = 0;

    buffer_t kiss_buf = {
        .data = kiss_msg.data,
        .capacity = sizeof(kiss_msg.data),
        .size = 0};

    ax25_error_e err = ax25_packet_pack(packet, &kiss_buf);
    if (err != AX25_SUCCESS)
        return false;
    kiss_msg.data_length = kiss_buf.size;

    int result = kiss_encode(&kiss_msg, out_buf->data, out_buf->capacity);
    if (result <= 0)
        return false;
    out_buf->size = result;

    return true;
}

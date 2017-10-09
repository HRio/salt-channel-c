#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "salt_v2.h"
#include "test_data.h"

void randombytes(unsigned char *p_bytes, unsigned long long length)
{
    memcpy(p_bytes, salt_test_data.client_ek_sec, length);
}

salt_ret_t my_write(salt_io_channel_t *p_wchannel)
{
    (void) p_wchannel;
    return SALT_SUCCESS;
}

salt_ret_t my_read(salt_io_channel_t *p_rchannel)
{
    static uint8_t i = 0;
    static uint32_t size;

    static uint8_t buf[SALT_HNDSHK_BUFFER_SIZE];

    switch (i) {
        case 0:
            memcpy(p_rchannel->p_data, salt_test_data.m2, 4);
            p_rchannel->size = 4;
            break;
        case 1:
            memcpy(p_rchannel->p_data, &salt_test_data.m2[4], p_rchannel->size_expected);
            p_rchannel->size = p_rchannel->size_expected;
            break;
        case 2:
            memcpy(p_rchannel->p_data, salt_test_data.m3, 4);
            p_rchannel->size = 4;
            break;
        case 3:
            memcpy(p_rchannel->p_data, &salt_test_data.m3[4], p_rchannel->size_expected);
            p_rchannel->size = p_rchannel->size_expected;
            break;
        case 4:
            size = read(0, buf, SALT_HNDSHK_BUFFER_SIZE);
            p_rchannel->size = 4;
            memcpy(p_rchannel->p_data, &size, 4);
            break;
        case 5:
            memcpy(p_rchannel->p_data, buf, p_rchannel->size_expected);
            p_rchannel->size = p_rchannel->size_expected;
            break;
        default:
            return SALT_ERROR;
    }

    i++;

    return SALT_SUCCESS;
}

void my_time_impl(uint32_t *p_time) {
    memset(p_time, 0, 4);
}

int main(void) {

    salt_channel_t channel;
    salt_ret_t ret;
    uint8_t hndsk_buffer[SALT_HNDSHK_BUFFER_SIZE];
    memset(hndsk_buffer, 0xcc, SALT_HNDSHK_BUFFER_SIZE);

    ret = salt_create(&channel, SALT_CLIENT, my_write, my_read, my_time_impl);
    ret = salt_set_signature(&channel, salt_test_data.client_sk_sec);
    ret = salt_init_session(&channel, hndsk_buffer, SALT_HNDSHK_BUFFER_SIZE);
    ret = salt_handshake(&channel);

    salt_msg_t read_msg;
    ret = salt_read_begin(&channel, hndsk_buffer, sizeof(hndsk_buffer), &read_msg);

    if (ret == SALT_ERROR) {
        return 0;
    }

    do {
        memset(read_msg.read.p_message, 0x00, read_msg.read.message_size);
    } while (salt_read_next(&read_msg) == SALT_SUCCESS);

    if (ret == SALT_SUCCESS) {
        return 0;
    }

    return 0;
}
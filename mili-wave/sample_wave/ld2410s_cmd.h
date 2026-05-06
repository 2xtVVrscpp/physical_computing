#include <stdio.h>
#include "pico/stdlib.h"
#include "debug.h"

#ifndef _H_LD2410S_CMD_
#define _H_LD2410S_CMD_

static uint8_t _semaphore_uart = 0;

const uint8_t CMD_READ_VERSION[] = {
    0x02, 0x00,
    0x00, 0x00
};

struct version_format {
    uint32_t header;
    uint16_t data_len;
    uint16_t cmd_word;
    uint32_t type;
    uint16_t version_type;
    uint16_t major_version;
    uint16_t minor_version;
    uint16_t patch_version;
    uint32_t footer;
} __attribute__((packed));

const uint8_t CMD_READ_SERIAL[] = {
    0x02, 0x00,
    0x11, 0x00
};

struct serial_format {
    uint32_t header;
    uint16_t data_len;
    uint16_t cmd_word;
    uint16_t ack;
    uint16_t serial_len;
    uint64_t serial;
    uint32_t footer;
}  __attribute__((packed));

const uint8_t CMD_ENABLE_CONFIG[] = {
    0x04, 0x00, 
    0xFF, 0x00,
    0x01, 0x00
};

struct enable_config_format {
    uint32_t header;
    uint16_t data_len;
    uint16_t cmd_word;
    uint16_t enable;
    uint16_t protocol_version;
    uint16_t buf_size;
    uint32_t footer;
} __attribute__((packed));

const uint8_t CMD_END_CONFIG[] = {
    0x02, 0x00,
    0xFE, 0x00
};

struct end_config_format {
    uint32_t header;
    uint16_t data_len;
    uint16_t cmd_word;
    uint16_t ack;
    uint32_t footer;
} __attribute__((packed));

struct report_data_format {
    uint8_t header;
    uint8_t status;
    uint16_t distance;
    uint8_t footer;
} __attribute__((packed));

void clear_uart_buffer(uart_inst_t *uart) {
    // Read and discard all bytes currently in the RX buffer
    dprintf("clean uart\n");
    while (uart_is_readable(uart)) {
        uint8_t ch = uart_getc(uart);
        dprintf("%02X ", ch);
    }
    dprintf("\n");
}

void send_frame_header(uart_inst_t *uart)
{
    static const uint8_t head[4] = {0xFD, 0xFC, 0xFB, 0xFA};
    uart_write_blocking(uart, head, sizeof(head));

    sleep_ms(100);
}

void send_frame_footer(uart_inst_t *uart)
{
    static const uint8_t foot[4] = {0x04, 0x03, 0x02, 0x01};
    uart_write_blocking(uart, foot, sizeof(foot));

    sleep_ms(100);
}

int send_cmd(uart_inst_t *uart, const uint8_t *cmd) {
    clear_uart_buffer(uart);

    // 確実に全バイトを送信する
    if (!uart_is_writable(uart)) {
        dprintf("cannot write due to some trouble\n");
        return 1;
    }

    while(_semaphore_uart) {
        sleep_ms(100);
    }
    _semaphore_uart = 1;

    send_frame_header(uart);
    uart_write_blocking(uart, cmd, sizeof(cmd));
    send_frame_footer(uart);

    sleep_ms(500);

    _semaphore_uart = 0;

    return 0;
}

extern void enable_config_mode(uart_inst_t *uart);
extern void end_config_mode(uart_inst_t *uart);
extern void get_version(uart_inst_t *uart);
extern void get_serial(uart_inst_t *uart);
extern void get_distance(uart_inst_t *uart);

#endif /* _H_LD2410S_CMD_ */
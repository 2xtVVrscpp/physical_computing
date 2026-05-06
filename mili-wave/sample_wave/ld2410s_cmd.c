#include "ld2410s_cmd.h"
#include "debug.h"

#define WAIT_SEND_CMD 500

static uint8_t _semaphore_uart = 0;

void clear_uart_buffer(uart_inst_t *uart) {
    // Read and discard all bytes currently in the RX buffer
    while (uart_is_readable(uart)) {
#ifdef _DPRINT
        uint8_t ch = uart_getc(uart);
        dprintf("%02X ", ch);
#else
        uart_getc(uart);
#endif
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

    sleep_ms(WAIT_SEND_CMD);

    _semaphore_uart = 0;

    return 0;
}

int send_cmd_without_frame(uart_inst_t *uart, const uint8_t *cmd) {
    // 確実に全バイトを送信する
    if (!uart_is_writable(uart)) {
        dprintf("cannot write due to some trouble\n");
        return 1;
    }

    uart_write_blocking(uart, cmd, sizeof(cmd));
    sleep_ms(100);

    return 0;
}

int get_packet(uart_inst_t *uart, uint8_t *buffer, int packet_size) {
    int idx = 0;

    dprintf("get packet\n");
    while (uart_is_readable(uart)) {
        uint8_t ch = uart_getc(uart);
        dprintf("%02X ", ch);

        if (idx == 0 && ch == 0xFD) { // ヘッダー待ち
            buffer[idx++] = ch;
        } else if (idx != 0 && ch == 0xFD) {
            idx = 0;
            buffer[idx++] = ch;
        } else if (idx > 0) {
            buffer[idx++] = ch;

            if (idx != packet_size) {
                // dprintf("n e\n");
                continue;
            }
            break;
        }
    }

    dprintf("\n");

    if (idx == packet_size) {
        return 0;
    } else {
        return 1;
    }
}

int get_minimal_packet(uart_inst_t *uart, uint8_t *buffer, int packet_size) {
    int idx = 0;

    while (uart_is_readable(uart)) {
        uint8_t ch = uart_getc(uart);
        // dprintf("%02X ", ch);

        if (idx == 0 && ch == 0x6E) { // ヘッダー待ち
            buffer[idx++] = ch;
        } else if (idx != 0 && ch == 0x6E) {
            idx = 0;
            buffer[idx++] = ch;
        } else if (idx > 0) {
            buffer[idx++] = ch;

            if (idx != packet_size) {
                continue;
            }
            break;
        }
    }

    // dprintf("\n");

    if (idx == packet_size) {
        return 0;
    } else {
        return 1;
    }
}

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

#define ENABLE_CONFIG_SIZE 18
void enable_config_mode(uart_inst_t *uart) {
    struct enable_config_format data;

    send_cmd(uart, CMD_ENABLE_CONFIG);
    dprintf("enable config mode:\n");

    get_packet(uart, (uint8_t *)&data, ENABLE_CONFIG_SIZE);
    if (data.header != 0xFAFBFCFD || data.cmd_word != 0x01FF) {
        dprintf("enable config\n");
        dprintf("header: 0x%x, cmd: 0x%x",data.header, data.cmd_word);
        dprintf("invalid data\n");
        return;;
    }
    printf("enable:0x%x\n", data.enable);
    printf("protocol version:0x%x\n", data.protocol_version);

    return;
}

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

#define END_CONFIG_SIZE 14
void end_config_mode(uart_inst_t *uart) {
    struct end_config_format data;

    send_cmd(uart, CMD_END_CONFIG);
    dprintf("end config mode: ");
    get_packet(uart, (uint8_t *)&data, END_CONFIG_SIZE);

    if (data.header != 0xFAFBFCFDU || data.cmd_word != 0x01FE) {
        dprintf("end config\n");
        dprintf("header: 0x%x, cmd: 0x%x", data.header, data.cmd_word);
        dprintf("invalid data\n");
        return;;
    }
    printf("ack: 0x%x\n", data.ack);

    return;
}

uint8_t CMD_GENERIC_PARAMETER[] = {
    0x26, 0x00,
    0x70, 0x00
};
const uint8_t farthest_distance[6] = {0x05, 0x00, 0x01, 0x00, 0x00, 0x00};      // 1m
const uint8_t nearest_distance[6] = {0x0A, 0x00, 0x00, 0x00, 0x00, 0x00};       // 0.0m
const uint8_t delay_time[6] = {0x06, 0x00, 0x14, 0x00, 0x00, 0x00};             // 20s
const uint8_t freq_status_report[6] = {0x02, 0x00, 0x0A, 0x00, 0x00, 0x00};     // 1.0 Hz
const uint8_t freq_distance_report[6] = {0x0C, 0x00, 0x0A, 0x00, 0x00, 0x00};   // 1.0 Hz
const uint8_t response_speed[6] = {0x0B, 0x00, 0x0A, 0x00, 0x00, 0x00};         // response normal

struct set_generic_param {
    uint32_t header;
    uint16_t data_len;
    uint16_t cmd_word;
    uint16_t ack;
    uint32_t footer; 
};

#define SET_GENERIC_PARAM_SIZE 14
void set_generic_parameters(uart_inst_t *uart) {
    struct set_generic_param data;

    dprintf("set_generic_parameters\n");

    enable_config_mode(uart);

    while(_semaphore_uart) {
        sleep_ms(100);
    }
    _semaphore_uart = 1;

    clear_uart_buffer(uart);

    CMD_GENERIC_PARAMETER[0] = 2 + (2 + 4)*3;
    send_frame_header(uart);
    send_cmd_without_frame(uart, CMD_GENERIC_PARAMETER);
    // send_cmd_without_frame(uart, farthest_distance);
    // send_cmd_without_frame(uart, nearest_distance);
    send_cmd_without_frame(uart, delay_time);
    send_cmd_without_frame(uart, freq_status_report);
    send_cmd_without_frame(uart, freq_distance_report);
    // send_cmd_without_frame(uart, response_speed);
    send_frame_footer(uart);
    while(true) {
        sleep_ms(WAIT_SEND_CMD);

        get_packet(uart, (uint8_t *)&data, SET_GENERIC_PARAM_SIZE);
        if (data.header != 0xFAFBFCFDU || data.cmd_word != 0x0170) {
            dprintf("header: 0x%x, cmd: 0x%x\n", data.header, data.cmd_word);
            dprintf("invalid data\n");
            continue;
        }
        break;
    }

    _semaphore_uart = 0;

    printf("ack: 0x%x\n", data.ack);

    end_config_mode(uart);

    return;
}

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

#define VERSION_DATA_SIZE 24
void get_version(uart_inst_t *uart) {
    struct version_format data;

    dprintf("get version\n");

    enable_config_mode(uart);

    send_cmd(uart, CMD_READ_VERSION);
    dprintf("get_version: ");
    get_packet(uart, (uint8_t *)&data, VERSION_DATA_SIZE);

    if ((data.header != 0xFAFBFCFD) || (data.cmd_word != 0x0100)) {
        dprintf("invalid data\n");
        return;
    }
    printf("ver_type:0x%x\n", data.version_type);
    printf("major, minor version: 0x%x%x\n", data.major_version, data.minor_version);
    printf("patch version: 0x%x\n", data.patch_version);

    end_config_mode(uart);

    return;
}

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

#define SERIAL_DATA_SIZE 24
void get_serial(uart_inst_t *uart) {
    struct serial_format data;

    dprintf("get serial\n");

    enable_config_mode(uart);

    send_cmd(uart, CMD_READ_SERIAL);
    dprintf("get_serial: ");
    get_packet(uart, (uint8_t *)&data, SERIAL_DATA_SIZE);

    if ((data.header != 0xFAFBFCFD) || (data.cmd_word != 0x0111)) {
        dprintf("invalid data\n");
        return;
    }
    printf("serial: 0x%llx\n", data.serial);
    printf("serial_len: 0x%x\n", data.serial_len);

    end_config_mode(uart);

    return;
}

struct report_data_format {
    uint8_t header;
    uint8_t status;
    uint16_t distance;
    uint8_t footer;
} __attribute__((packed));

#define REPORT_DATA_SIZE 5
int get_distance(uart_inst_t *uart) {
    struct report_data_format data;

    dprintf("get distance\n");

    get_minimal_packet(uart, (uint8_t *)&data, REPORT_DATA_SIZE);
    dprintf("header: 0x%x, footer: 0x%x\n", data.header, data.footer);
    if ((data.header == 0x6E) && (data.footer == 0x62)) {
        dprintf("S: %s, D: %d cm\n", (data.status >= 2) ? "M" : "U", data.distance);
        return data.distance;
    } else {
        return -1;
    }
}
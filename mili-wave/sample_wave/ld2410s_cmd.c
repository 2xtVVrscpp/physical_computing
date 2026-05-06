#include "ld2410s_cmd.h"
#include "debug.h"

#define WAIT_SEND_CMD 500

#define ENABLE_CONFIG_SIZE 18
void enable_config_mode(uart_inst_t *uart) {
    struct enable_config_format *enable_config;
    uint8_t buffer[ENABLE_CONFIG_SIZE];
    int idx = 0;

    send_cmd(uart, CMD_ENABLE_CONFIG);
    sleep_ms(WAIT_SEND_CMD);
    dprintf("enable config mode:\n");
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

            if (idx != ENABLE_CONFIG_SIZE) {
                // dprintf("n e\n");
                continue;
            }

            idx = 0;

            enable_config = (struct enable_config_format *)buffer;
            if (enable_config->header != 0xFAFBFCFD || enable_config->cmd_word != 0x01FF) {
                dprintf("header: 0x%x, cmd: 0x%x",enable_config->header, enable_config->cmd_word);
                dprintf("invalid data\n");
                continue;
            }
            printf("\n");
            printf("enable:0x%x\n", enable_config->enable);
            printf("protocol version:0x%x\n", enable_config->protocol_version);
        }
    }
    dprintf("\n");
}

#define END_CONFIG_SIZE 14
void end_config_mode(uart_inst_t *uart) {
    struct end_config_format *end_config;
    uint8_t buffer[END_CONFIG_SIZE];
    int idx = 0;

    send_cmd(uart, CMD_END_CONFIG);
    sleep_ms(WAIT_SEND_CMD);
    dprintf("end config mode: ");
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

            if (idx != END_CONFIG_SIZE) {
                continue;
            }
            idx = 0;

            end_config = (struct end_config_format *)buffer;
            if (end_config->header != 0xFAFBFCFDU || end_config->cmd_word != 0x01FE) {
                continue;
            }
            printf("\n");
            printf("ack: 0x%x\n", end_config->ack);
        }
    }
    dprintf("\n");
}

#define VERSION_DATA_SIZE 24
void get_version(uart_inst_t *uart) {
    struct version_format *fw_ver;
    uint8_t buffer[VERSION_DATA_SIZE];
    int idx = 0;

    enable_config_mode(uart);

    send_cmd(uart, CMD_READ_VERSION);
    sleep_ms(WAIT_SEND_CMD);
    dprintf("get_version: ");
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

            if (idx != VERSION_DATA_SIZE) {
                continue;
            }

            fw_ver = (struct version_format *)buffer;
            idx = 0; // バッファリセット
            if ((fw_ver->header == 0xFAFBFCFD) && (fw_ver->cmd_word == 0x0100)) { // フッターが正しいか
                printf("\n");
                printf("ver_type:0x%x\n", fw_ver->version_type);
                printf("major, minor version: 0x%x%x\n", fw_ver->major_version, fw_ver->minor_version);
                printf("patch version: 0x%x\n", fw_ver->patch_version);
            }
        }
    }
    dprintf("\n");

    end_config_mode(uart);
    sleep_ms(WAIT_SEND_CMD);
}

#define SERIAL_DATA_SIZE 24
void get_serial(uart_inst_t *uart) {
    struct serial_format *data;
    uint8_t buffer[SERIAL_DATA_SIZE];
    int idx = 0;

    enable_config_mode(uart);

    send_cmd(uart, CMD_READ_SERIAL);
    sleep_ms(WAIT_SEND_CMD);
    dprintf("get_serial: ");
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

            if (idx != SERIAL_DATA_SIZE) {
                continue;
            }
            idx = 0;

            data = (struct serial_format *)buffer;
            printf("\nheader: 0x%x, cmd_word: 0x%x\n", data->header, data->cmd_word);
            if ((data->header == 0xFAFBFCFD) && (data->cmd_word == 0x0111)) {
                printf("\n");
                printf("serial: 0x%llx\n", data->serial);
                printf("serial_len: 0x%x\n", data->serial_len);
            }
            
        }
    }
    dprintf("\n");

    end_config_mode(uart);
    sleep_ms(WAIT_SEND_CMD);
}

#define REPORT_DATA_SIZE 5
void get_distance(uart_inst_t *uart) {
    struct report_data_format *data;
    uint8_t buffer[REPORT_DATA_SIZE];
    int idx = 0;

    while (uart_is_readable(uart)) {
        uint8_t ch = uart_getc(uart);
        dprintf("%02X ", ch);

        if (idx == 0 && ch == 0x6E) {
            buffer[idx++] = ch;
        } else if (idx > 0) {
            buffer[idx++] = ch;

            if (idx != REPORT_DATA_SIZE) { // 5バイト溜まったら検証
                continue;
            }
            idx = 0;

            data = (struct report_data_format *)buffer;
            if ((data->header == 0x6E) && (data->footer == 0x62)) {
                if(data->distance < 75){
                    dprintf("S: %s, D: %d cm\n", (data->status >= 2) ? "M" : "U", data->distance);
                    gpio_put(LED_PIN, 1);
                }else {
                    gpio_put(LED_PIN, 0);
                }
            }
            
        }
    }
    dprintf("\n");
}
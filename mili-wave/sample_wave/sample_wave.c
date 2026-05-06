#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "ld2410s_cmd.h"
#include "debug.h"

// definitions for gpio
#define UART_ID uart0
#define BAUD_RATE 115200
#define UART_TX_PIN 0
#define UART_RX_PIN 1
#define PRESENCE_PIN 2
#define LED_PIN 3
#define WAIT_SEND_CMD 500

void init_raspi_picow (void) {
    stdio_init_all();

    // 1. init UART
    uart_init(UART_ID, BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    gpio_pull_up(UART_TX_PIN);
    gpio_pull_up(UART_RX_PIN);

    uart_set_format(UART_ID, 8, 1, UART_PARITY_NONE);

    // 2. init OT2
    gpio_init(PRESENCE_PIN);
    gpio_set_dir(PRESENCE_PIN, GPIO_IN);

    // 3. led setting
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
}


int main() {
    int distance;
    
    init_raspi_picow();

    sleep_ms(5000);

    get_version(UART_ID);
    get_serial(UART_ID);

    // set_generic_parameters(UART_ID); // DOESN'T WORK YET

    dprintf("get distance:\n");
    while (true) {
        // check digital output
        if (!gpio_get(PRESENCE_PIN)) { /* un-detect */
            continue;
        }

        distance = get_distance(UART_ID);
        if(distance == -1) {
            continue;
        }

        if(distance < 65){
            printf("D: %d cm\n", distance);
            gpio_put(LED_PIN, 1);
        }else {
            gpio_put(LED_PIN, 0);
        }
        
        sleep_ms(500);
    }
}
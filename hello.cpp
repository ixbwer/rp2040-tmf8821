#include <stdio.h>
#include "pico/stdlib.h"
#include "tmf8828_app.h"
#include "st7789.h"
#ifndef LED_DELAY_MS
#define LED_DELAY_MS 250
#endif
#define LED_PIN 4
#define SPI_TX_PIN 3
#define SPI_SCK_PIN 2
#define SPI_DC_PIN 1
#define SPI_RESET_PIN 0

// Perform initialisation
int pico_led_init(void) {
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    return PICO_OK;
}

// Turn the led on or off
void pico_set_led(bool led_on) {
    gpio_put(LED_PIN, led_on);
}

void st7789_test(void) {
    printf("ST7789 test\n");
    spi_inst_t *spi = spi0;

    gpio_init(SPI_DC_PIN);
    gpio_init(SPI_RESET_PIN);
    gpio_set_dir(SPI_DC_PIN, GPIO_OUT);
    gpio_set_dir(SPI_RESET_PIN, GPIO_OUT);

    gpio_set_function(SPI_TX_PIN, GPIO_FUNC_SPI);
    gpio_set_function(SPI_SCK_PIN, GPIO_FUNC_SPI);
    spi_init(spi, 20000000);
    spi_set_format(spi, 8, SPI_CPOL_1, SPI_CPHA_1, SPI_MSB_FIRST);

    int disp_width = 240;
    int disp_height = 240;
    uint8_t st7789_res = 0;
    uint8_t st7789_dc = 1;
    uint8_t font2[] = {0x20, 0x7f, 8, 8, 8};
    ST7789 display(spi, disp_width, disp_height, st7789_res, st7789_dc, 0, 0, 0, 0, 0);

    display.text(font2, "Hello!", 10, 10, WHITE, BLACK);
    display.text(font2, "RPi Pico", 10, 40, WHITE, BLACK);
    display.text(font2, "MicroPython", 35, 100, WHITE, BLACK);
    display.text(font2, "EETREE", 35, 150, WHITE, BLACK);
    display.text(font2, "www.eetree.cn", 30, 200, WHITE, BLACK);
    printf("ST7789 test end\n");
}

int main() {
    stdio_init_all();
    int rc = pico_led_init();
    printf("Hello, world!\n");
    st7789_test();
    hard_assert(rc == PICO_OK);
    setupforTMF882x();
    //setupFn(255, 115200, 400000);
    while (true) {
        loopFnforTMF882x();
        //loopFn();
    }
}

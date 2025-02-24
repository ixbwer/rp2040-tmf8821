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

ST7789* init_st7789() {
    spi_inst_t *spi = spi0;

    gpio_init(SPI_DC_PIN);
    gpio_init(SPI_RESET_PIN);
    gpio_set_dir(SPI_DC_PIN, GPIO_OUT);
    gpio_set_dir(SPI_RESET_PIN, GPIO_OUT);

    gpio_set_function(SPI_TX_PIN, GPIO_FUNC_SPI);
    gpio_set_function(SPI_SCK_PIN, GPIO_FUNC_SPI);
    spi_init(spi, 20000000);
    spi_set_format(spi, 8, SPI_CPOL_1, SPI_CPHA_1, SPI_MSB_FIRST);

    return new ST7789(spi, 240, 240, 0, 1, 0, 0, 0, 0, 0);
}

void st7789_test(ST7789* st7789) {
    printf("ST7789 test\n");
    ST7789* display = init_st7789();
    uint8_t font2[] = {0x20, 0x7f, 8, 8, 8};

    display->text(font2, "Hello!", 10, 10, WHITE, BLACK);
    display->text(font2, "RPi Pico", 10, 40, WHITE, BLACK);
    display->text(font2, "MicroPython", 35, 100, WHITE, BLACK);
    display->text(font2, "EETREE", 35, 150, WHITE, BLACK);
    display->text(font2, "www.eetree.cn", 30, 200, WHITE, BLACK);
    display->fill_rect(120, 0, 20, 100, GREEN);
    display->fill_rect(0, 200, 240, 20, GREEN);
    display->test_pic();
    
    printf("ST7789 test end\n");
}

#include <random>

int get_random()
{
    // get random number 0 or 1
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 1);
    return dis(gen);
}

#define BAR_DIRECTION_UP 0
#define BAR_DIRECTION_DOWN 1

#define HEIGHT_MAX 240
#define HEIGHT_MIN 0

int height = 0;

int direction = 0;

void paint()
{   
    static int last_time = 0;
    int current_time = to_ms_since_boot(get_absolute_time());
    if (current_time - last_time < 1000)
    {
        return;
    }
    height = get_random() == 0 ? height + 1 : height - 1;
    if (height > HEIGHT_MAX)
    {
        height = HEIGHT_MAX;
    }
    if (height < HEIGHT_MIN)
    {
        height = HEIGHT_MIN;
    }
    last_time = current_time;
}

void update_paint()
{
    paint();
}

int main() {
    stdio_init_all();
    pico_set_led(false);
    int rc = pico_led_init();
    printf("Hello, world!\n");
    ST7789 *display = init_st7789();
    hard_assert(rc == PICO_OK);
    setupforTMF882x();
    //setupFn(255, 115200, 400000);
    while (true) {
        update_paint();
        // calculate_position();
        //loopFnforTMF882x();
        //loopFn();
    }
}

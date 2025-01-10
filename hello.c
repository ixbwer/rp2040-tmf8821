#include <stdio.h>
#include "pico/stdlib.h"
#include "tmf8828_app.h"

#ifndef LED_DELAY_MS
#define LED_DELAY_MS 250
#endif
#define LED_PIN 4

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

int main() {
    stdio_init_all();
    int rc = pico_led_init();
    hard_assert(rc == PICO_OK);
    setupFn(6, 115200, 400000);
    while (true) {
        loopFn();
        pico_set_led(true);
        sleep_ms(LED_DELAY_MS);
        pico_set_led(false);
        sleep_ms(LED_DELAY_MS);
    }
}

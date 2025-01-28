#ifndef ST7789_H
#define ST7789_H

#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "pico/time.h"
#include "pico/stdlib.h"



class ST7789 {
private:
    int _display_width;
    int _display_height;
    int _rotation;

public:
    spi_inst_t *spi;
    uint8_t reset;
    uint8_t dc;
    uint8_t cs;
    uint8_t backlight;
    int width;
    int height;
    int xstart;
    int ystart;
    ST7789(spi_inst_t *spi, int width, int height, uint8_t reset, uint8_t dc, uint8_t cs, uint8_t backlight, int xstart, int ystart, int rotation);
    void write(uint8_t command, uint8_t *data);
    void write_command(uint8_t command);
    void write_data(uint8_t *data, int length);
    void hard_reset();
    void soft_reset();
    void sleep_mode(bool value);
    void inversion_mode(bool value);
    void set_color_mode(uint8_t mode);
    void rotation(int rotation);
    void _set_columns(int start, int end);
    void _set_rows(int start, int end);
    void set_window(int x0, int y0, int x1, int y1);
    void vline(int x, int y, int length, uint16_t color);
    void hline(int x, int y, int length, uint16_t color);
    void pixel(int x, int y, uint16_t color);
    void blit_buffer(uint8_t *buffer, int x, int y, int width, int height);
    void rect(int x, int y, int w, int h, uint16_t color);
    void fill_rect(int x, int y, int width, int height, uint16_t color);
    void fill(uint16_t color);
    void line(int x0, int y0, int x1, int y1, uint16_t color);
    void vscrdef(int tfa, int vsa, int bfa);
    void vscsad(int vssa);
    void text8(const uint8_t *font, const char *text, int x0, int y0, uint16_t color, uint16_t background);
    void text16(const uint8_t *font, const char *text, int x0, int y0, uint16_t color, uint16_t background);
    void text(const uint8_t *font, const char *text, int x0, int y0, uint16_t color, uint16_t background);
};

void st7789_test();
#endif

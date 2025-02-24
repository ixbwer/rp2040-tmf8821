#ifndef ST7789_H
#define ST7789_H

#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "pico/time.h"
#include "pico/stdlib.h"

// Define constants
#define ST7789_NOP 0x00
#define ST7789_SWRESET 0x01
#define ST7789_RDDID 0x04
#define ST7789_RDDST 0x09
#define ST7789_SLPIN 0x10
#define ST7789_SLPOUT 0x11
#define ST7789_PTLON 0x12
#define ST7789_NORON 0x13
#define ST7789_INVOFF 0x20
#define ST7789_INVON 0x21
#define ST7789_DISPOFF 0x28
#define ST7789_DISPON 0x29
#define ST7789_CASET 0x2A
#define ST7789_RASET 0x2B
#define ST7789_RAMWR 0x2C
#define ST7789_RAMRD 0x2E
#define ST7789_PTLAR 0x30
#define ST7789_VSCRDEF 0x33
#define ST7789_COLMOD 0x3A
#define ST7789_MADCTL 0x36
#define ST7789_VSCSAD 0x37
#define ST7789_MADCTL_MY 0x80
#define ST7789_MADCTL_MX 0x40
#define ST7789_MADCTL_MV 0x20
#define ST7789_MADCTL_ML 0x10
#define ST7789_MADCTL_BGR 0x08
#define ST7789_MADCTL_MH 0x04
#define ST7789_MADCTL_RGB 0x00
#define ST7789_RDID1 0xDA
#define ST7789_RDID2 0xDB
#define ST7789_RDID3 0xDC
#define ST7789_RDID4 0xDD

#define COLOR_MODE_65K 0x50
#define COLOR_MODE_262K 0x60
#define COLOR_MODE_12BIT 0x03
#define COLOR_MODE_16BIT 0x05
#define COLOR_MODE_18BIT 0x06
#define COLOR_MODE_16M 0x07

#define BLACK 0x0000
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF

#define _ENCODE_PIXEL  ">H"
#define _ENCODE_POS    ">HH"
#define _ENCODE_RECT   ">BBB"

#define _BUFFER_SIZE 2

#define SPI_TX_PIN 3
#define SPI_SCK_PIN 2
#define SPI_DC_PIN 1
#define SPI_RESET_PIN 0


#define BLACK 0x0000
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF

#define _ENCODE_PIXEL  ">H"
#define _ENCODE_POS    ">HH"
#define _ENCODE_RECT   ">BBB"

#define _BUFFER_SIZE 2

#define ACTION_Y 4
#define ACTION_HEIGHT 230
#define ACTION_X 205
#define ACTION_WIDTH 16

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
    void test_pic();
    
    // Draw a circle
    void circle(int x0, int y0, int r, uint16_t color);
    
    // Draw a filled circle
    void fill_circle(int x0, int y0, int r, uint16_t color);
    
    // Draw an ellipse
    void ellipse(int x0, int y0, int a, int b, uint16_t color);
    
    // Draw a filled ellipse
    void fill_ellipse(int x0, int y0, int a, int b, uint16_t color);
    void ST7789::paint_energybar(int user_height,int action_height);

private:
    // Helper function for circle/ellipse drawing
    void plot_points(int x0, int y0, int x, int y, uint16_t color);
    void fill_points(int x0, int y0, int x, int y, uint16_t color);
};

void st7789_test();
#endif

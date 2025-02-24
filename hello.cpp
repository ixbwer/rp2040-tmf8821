#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/sync.h"
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

#define HEIGHT_MAX 180
#define HEIGHT_MIN 0

// 添加高度映射相关的定义
#define SENSOR_HEIGHT_MIN 0     // 传感器最小高度
#define SENSOR_HEIGHT_MAX 2000  // 传感器最大高度 (2000mm = 2m)
#define DISPLAY_HEIGHT_MIN 0
#define DISPLAY_HEIGHT_MAX 220  // 显示高度范围


// Game States
#define STATE_MENU 0
#define STATE_CONFIG 1
#define STATE_GAME 2
#define STATE_GAME_OVER 3

// Direction values from sensor
#define DIRECTION_NONE 0
#define DIRECTION_LEFT 1
#define DIRECTION_RIGHT 2
#define DIRECTION_UP 3
#define DIRECTION_DOWN 4

// Game parameters
typedef struct
{
    int state;
    int selected_option; // 0 for Start, 1 for Config
    int difficulty; // 1-Easy, 2-Medium, 3-Hard
    int step_size; // Step size for bar movement
    int score;
    int match_score; // Score for matching heights
    int match_threshold; // Distance within which heights match
    bool option_selected;
    int config_selected_option; // 0-Easy, 1-Medium, 2-Hard
    int last_direction;
    uint32_t last_direction_time;
    uint32_t direction_debounce_ms; // Debounce time for direction changes
}GAME;

static GAME game = 
{
    .state = STATE_MENU,
    .selected_option = 0,
    .difficulty = 1,
    .step_size = 2,
    .score = 0,
    .match_score = 100,
    .match_threshold = 20,
    .option_selected = false,
    .config_selected_option = 0,
    .last_direction = DIRECTION_NONE,
    .last_direction_time = 0,
    .direction_debounce_ms = 800
};

// // 动态范围调整参数
// static struct {
//     int min_height_seen = SENSOR_HEIGHT_MAX;
//     int max_height_seen = SENSOR_HEIGHT_MIN;
//     int smoothed_min = SENSOR_HEIGHT_MAX;
//     int smoothed_max = SENSOR_HEIGHT_MIN;
//     float alpha = 0.1f;  // 平滑因子
// } range_tracker;

// // 添加自定义的clamp函数
// template<typename T>
// T clamp(T value, T min_val, T max_val) {
//     if (value < min_val) return min_val;
//     if (value > max_val) return max_val;
//     return value;
// }

// 将传感器高度映射到显示高度
int map_height_to_display(int sensor_height) {
    if (sensor_height>220)
    {
        sensor_height=220;
    }
    if (sensor_height < 20) 
    {
        sensor_height = 20;
    }
    sensor_height = 240 - sensor_height;
    return sensor_height;
    // // 更新观察到的范围
    // if (sensor_height > 0) {  // 只在有效高度时更新
    //     range_tracker.min_height_seen = std::min(range_tracker.min_height_seen, sensor_height);
    //     range_tracker.max_height_seen = std::max(range_tracker.max_height_seen, sensor_height);
        
    //     // 平滑更新范围
    //     range_tracker.smoothed_min += range_tracker.alpha * (range_tracker.min_height_seen - range_tracker.smoothed_min);
    //     range_tracker.smoothed_max += range_tracker.alpha * (range_tracker.max_height_seen - range_tracker.smoothed_max);
    // }

    // // 使用平滑后的范围进行映射
    // int effective_min = (int)range_tracker.smoothed_min;
    // int effective_max = (int)range_tracker.smoothed_max;
    
    // // 确保有效范围
    // if (effective_max <= effective_min) {
    //     effective_min = SENSOR_HEIGHT_MIN;
    //     effective_max = SENSOR_HEIGHT_MAX;
    // }

    // // 映射高度值
    // int mapped_height = (sensor_height - effective_min) * 
    //                    (DISPLAY_HEIGHT_MAX - DISPLAY_HEIGHT_MIN) / 
    //                    (effective_max - effective_min) + 
    //                    DISPLAY_HEIGHT_MIN;

    // // 确保结果在显示范围内
    // return clamp(mapped_height, DISPLAY_HEIGHT_MIN, DISPLAY_HEIGHT_MAX);
}

int bar_height = 30;
static int direction = BAR_DIRECTION_UP;  // 控制方向

static std::random_device rd;
static std::mt19937 gen(rd());
static std::uniform_int_distribution<> direction_change_dis(0, 100);  // 0-100的随机数

void update_paint()
{   
    static int last_time = 0;
    int current_time = to_ms_since_boot(get_absolute_time());
    if (current_time - last_time < 10)
    {
        return;
    }

    // 10%的概率改变方向
    if (direction_change_dis(gen) < 5) {
        direction = (direction == BAR_DIRECTION_UP) ? BAR_DIRECTION_DOWN : BAR_DIRECTION_UP;
    }

    // 根据当前方向移动
    if (direction == BAR_DIRECTION_UP) {
        bar_height += game.step_size;
        if (bar_height >= HEIGHT_MAX) {
            bar_height = HEIGHT_MAX;
            direction = BAR_DIRECTION_DOWN;
        }
    } else {
        bar_height -= game.step_size;
        if (bar_height <= HEIGHT_MIN) {
            bar_height = HEIGHT_MIN;
            direction = BAR_DIRECTION_UP;
        }
    }
    
    last_time = current_time;
}


// Draw the main menu screen
void draw_menu(ST7789* display) {
    uint8_t font[] = {0x20, 0x7f, 8, 16, 8}; // Larger font
    
    // Clear screen
    display->fill_rect(0, 0, 240, 240, BLACK);
    
    // Draw title
    display->text(font, "DTOF GAME made by ixbwer", 30, 30, WHITE, BLACK);
    
    // Draw options
    if (game.selected_option == 0) {
        display->text(font, "> START", 60, 100, GREEN, BLACK);
        display->text(font, "  CONFIG", 60, 140, WHITE, BLACK);
    } else {
        display->text(font, "  START", 60, 100, WHITE, BLACK);
        display->text(font, "> CONFIG", 60, 140, GREEN, BLACK);
    }
    
    // Draw instructions
    uint8_t small_font[] = {0x20, 0x7f, 8, 8, 8};
    display->text(small_font, "Move hand up/down to select", 20, 190, YELLOW, BLACK);
    display->text(small_font, "Move hand left to select", 20, 210, YELLOW, BLACK);
}


// Draw the config screen
void draw_config(ST7789* display) {
    uint8_t font[] = {0x20, 0x7f, 8, 16, 8};
    
    // Clear screen
    display->fill_rect(0, 0, 240, 240, BLACK);
    
    // Draw title
    display->text(font, "DIFFICULTY", 60, 30, WHITE, BLACK);
    printf("config_selected_option=%d\n",game.config_selected_option);
    // Draw options
    if (game.config_selected_option == 0) {
        display->text(font, "> EASY", 60, 80, GREEN, BLACK);
        display->text(font, "  MEDIUM", 60, 110, WHITE, BLACK);
        display->text(font, "  HARD", 60, 140, WHITE, BLACK);

    } else if (game.config_selected_option == 1) {
        display->text(font, "  EASY", 60, 80, WHITE, BLACK);
        display->text(font, "> MEDIUM", 60, 110, GREEN, BLACK);
        display->text(font, "  HARD", 60, 140, WHITE, BLACK);

    } else if (game.config_selected_option == 2) {
        display->text(font, "  EASY", 60, 80, WHITE, BLACK);
        display->text(font, "  MEDIUM", 60, 110, WHITE, BLACK);
        display->text(font, "> HARD", 60, 140, GREEN, BLACK);

    } else{
        display->text(font, "  EASY", 60, 80, WHITE, BLACK);
        display->text(font, "  MEDIUM", 60, 110, WHITE, BLACK);
        display->text(font, "  HARD", 60, 140, WHITE, BLACK);
    }
}

// Draw the game over screen
void draw_game_over(ST7789* display) {
    uint8_t font[] = {0x20, 0x7f, 8, 16, 8};
    
    // Clear screen
    display->fill_rect(0, 0, 240, 240, BLACK);
    
    // Draw title and score
    display->text(font, "GAME OVER", 65, 30, RED, BLACK);
    
    char score_text[20];
    sprintf(score_text, "SCORE: %d", game.score);
    display->text(font, score_text, 70, 80, WHITE, BLACK);
    
    // Draw options
    if (game.selected_option == 0) {
        display->text(font, "> RESTART", 60, 140, GREEN, BLACK);
        display->text(font, "  MENU", 60, 180, WHITE, BLACK);
    } else {
        display->text(font, "  RESTART", 60, 140, WHITE, BLACK);
        display->text(font, "> MENU", 60, 180, GREEN, BLACK);
    }
}

// Reset game state for new game
void reset_game() {
    game.score = 0;
    game.match_score = 100;
    // Set match threshold based on difficulty
    switch (game.difficulty) {
        case 1: // Easy
            game.step_size = 1;
            break;
        case 2: // Medium
            game.step_size = 4;
            break;
        case 3: // Hard
            game.step_size = 8;
            break;
    }
}


// Process direction input with debounce
void process_direction(char direction) {
    uint32_t current_time = to_ms_since_boot(get_absolute_time());
    
    // Debounce direction changes
    if (current_time - game.last_direction_time < game.direction_debounce_ms) {
        return;
    }
    // printf("process direction=%c gamestate=%d gameoption=%d\n",direction,game.state,game.config_selected_option);
    int direction_value = DIRECTION_NONE;
    
    // Convert character direction to numeric value
    if (direction == 'l') {
        direction_value = DIRECTION_LEFT;
    } else if (direction == 'r') {
        direction_value = DIRECTION_RIGHT;
    } else if (direction == 'u') {
        direction_value = DIRECTION_UP;
    } else if (direction == 'd') {
        direction_value = DIRECTION_DOWN;
    }
    
    // Only process if direction changed
    if (direction_value != DIRECTION_NONE) {
        game.last_direction = direction_value;
        game.last_direction_time = current_time;
        
        if (game.state == STATE_MENU) {
            // Toggle between Start and Config
            if (direction_value == DIRECTION_UP) {
                game.selected_option = 0;
            } else if (direction_value == DIRECTION_DOWN) {
                game.selected_option = 1;
            // } else if (direction_value == DIRECTION_LEFT) {
            //     game.selected_option = 1;
            } else if (direction_value == DIRECTION_LEFT && game.selected_option == 0) {
                reset_game();
                game.state = STATE_GAME;
            } else if (direction_value == DIRECTION_LEFT && game.selected_option == 1) {
                game.state = STATE_CONFIG;
            }
        } else if (game.state == STATE_CONFIG) {
            // Cycle through difficulty options
            if (direction_value == DIRECTION_UP) {
                if (game.config_selected_option == 2) {
                    game.config_selected_option = 1;
                    game.difficulty = 2;
                } else if (game.config_selected_option == 1) {
                    game.config_selected_option = 0;
                    game.difficulty = 1;
                } else if (game.config_selected_option == 0) {
                    game.config_selected_option = 0;
                    game.difficulty = 1;
                }
            } else if (direction_value == DIRECTION_DOWN) {
                if (game.config_selected_option == 0) {
                    game.config_selected_option = 1;
                    game.difficulty = 2;
                } else if (game.config_selected_option == 1) {
                    game.config_selected_option = 2;
                    game.difficulty = 3;
                } else if (game.config_selected_option == 2) {
                    game.config_selected_option = 2;
                    game.difficulty = 3;
                } 
            } else if (direction_value == DIRECTION_LEFT) {
                game.state = STATE_MENU;
            } 
            printf("config_selected_option %d  difficulty %d", game.config_selected_option,game.difficulty);
        } else if (game.state == STATE_GAME_OVER) {
            // Toggle between Restart and Menu
            if (direction_value == DIRECTION_UP) {
                game.selected_option = 0;
            } else if (direction_value == DIRECTION_DOWN) {
                game.selected_option = 1;
            } else if (direction_value == DIRECTION_LEFT && game.selected_option == 0) {
                reset_game();
                game.state = STATE_GAME;
            } else if (direction_value == DIRECTION_LEFT && game.selected_option == 1) {
                game.state = STATE_MENU;
            }
        }
    }
    // printf("after process direction=%c gamestate=%d gameoption=%d\n",direction,game.state,game.config_selected_option);

}

// Check if heights are close enough to match
bool heights_match(int mapped_height, int action_height) {
    int diff = abs(mapped_height - action_height);
    return diff <= game.match_threshold;
}


// Update game display
void update_game_display(ST7789* display, int mapped_height, int bar_height) {
    // Clear screen except for the bars
    //display->test_pic();
    //display->fill_rect(0, 0, 240, 40, BLACK);
    // Display score
    uint8_t font[] = {0x20, 0x7f, 8, 8, 8};
    char score_text[20];
    sprintf(score_text, "SCORE: %d", game.score);
    display->text(font, score_text, 10, 10, WHITE, 0x1bb5);
    
    // Display difficulty
    printf("game dif%d",game.difficulty);
    char diff_text[20];
    sprintf(diff_text, "LEVEL: %s", 
        game.difficulty == 1 ? "EASY" : 
        (game.difficulty == 2 ? "MED" : "HARD"));
    display->text(font, diff_text, 120, 10, WHITE, 0x1bb5);
    
    // Draw the bars
    display->paint_energybar(mapped_height, bar_height); // Game bar
    
    // Draw sensor bar with color based on match status
    int color = RED;
    if (heights_match(mapped_height, bar_height + 30)) {
        color = GREEN;
        game.score++;
        game.match_score++;
        if (game.match_score >=220)
            game.match_score = 220;
    }
    else
    {
        game.match_score -= 2;
        if (game.match_score <=0)
            game.state = STATE_GAME_OVER;
    }
    
    // Draw sensor height bar
    for (int i = 238; i < 240; i++) {
        for (int x = 0; x < 220; x++) {
            display->pixel(x, i, 0x1bb5);
        }
    }
    for (int i = 238; i < 240; i++) {
        for (int x = 0; x < game.match_score; x++) {
            display->pixel(x, i, color);
        }
    }
}

// Shared data structure between cores
struct SharedHeight {
    int height;
    bool new_data_available;
    mutex_t mutex;
};

struct SharedDirection {
    mutex_t mutex;
    bool new_data_available;
    char direction;
};

static SharedHeight shared_height;
static SharedDirection shared_direction;

void update_shared_direction(char direction) {
    mutex_enter_blocking(&shared_direction.mutex);
    shared_direction.direction = direction;
    shared_direction.new_data_available = true;
    mutex_exit(&shared_direction.mutex);
}

char get_shared_direction(bool *new_data) {
    char direction;
    mutex_enter_blocking(&shared_direction.mutex);
    direction = shared_direction.direction;
    if (new_data) {
        *new_data = shared_direction.new_data_available;
        shared_direction.new_data_available = false;
    }
    mutex_exit(&shared_direction.mutex);
    return direction;
}

// Function to safely update the sensor data
void update_shared_sensor_data(int height) {
    mutex_enter_blocking(&shared_height.mutex);
    shared_height.height = height;
    shared_height.new_data_available = true;
    mutex_exit(&shared_height.mutex);
}

// Function to safely retrieve the sensor data
int get_shared_sensor_data(bool *new_data) {
    int height;
    mutex_enter_blocking(&shared_height.mutex);
    height = shared_height.height;
    if (new_data) {
        *new_data = shared_height.new_data_available;
        shared_height.new_data_available = false;
    }
    mutex_exit(&shared_height.mutex);
    return height;
}
bool core1_separate_stack = true;

// Core 1 function - handles sensor data acquisition
void core1_sensor_acquisition() {
    while (true) {
        // Get sensor data
        SensorData sensor_data;
        loopFnforTMF882x(&sensor_data);
        // printf("--Core 1: Height: %d, Direction: %c\n", sensor_data.average_height, sensor_data.direction);
        if (sensor_data.direction != '-') {
            update_shared_direction(sensor_data.direction);
        }
        if (sensor_data.valid) {
            update_shared_sensor_data(sensor_data.average_height);
        }
        // printf("Core 1: Height: %d, Direction: %c\n", sensor_data.average_height, sensor_data.direction);
        sleep_ms(10);
    }

}

int main() {
    stdio_init_all();
    pico_set_led(false);
    int rc = pico_led_init();
    printf("Core 0: Hello, world!\n");
    setupforTMF882x();
    // Initialize display
    ST7789 *display = init_st7789();
    display->fill_rect(0, 0, 240, 240, BLACK); // Clear screen first
    
    // Show initial display content
    printf("Core 0: Display initialized\n");
    
    // Initialize the mutex before both cores try to use it
    mutex_init(&shared_height.mutex);
    mutex_init(&shared_direction.mutex);
    shared_direction.new_data_available = false;
    shared_height.new_data_available = false;
    
    hard_assert(rc == PICO_OK);
    
    printf("Core 0: Launching sensor acquisition on Core 1...\n");
    
    // Launch Core 1 task for sensor data acquisition
    // We're on Core 0 by default, so launch on Core 1
    multicore_launch_core1(core1_sensor_acquisition);
    sleep_ms(1000); // Give Core 1 time to start up
    printf("Core 0: Core 1 launched\n");
    
    // Draw initial menu after system initialization
    draw_menu(display);
    
    // Main thread (Core 0) handles game logic and display
    static int mapped_height = 0;
    uint32_t last_debug_time = 0;
    uint32_t current_time = 0;
    static GAME last_game = game;
    while (true) {
        current_time = to_ms_since_boot(get_absolute_time());
        
        // Get sensor data from shared structure
        bool new_height = false;
        int height = get_shared_sensor_data(&new_height);
        if (new_height) {
            mapped_height = map_height_to_display(height);
            // printf("height: %d\n", height);
        }
        
        // Get direction data from shared structure
        bool new_direction = false;
        char direction = get_shared_direction(&new_direction);
        if (new_direction) {
            process_direction(direction);
            printf("direction: %c\n", direction);
        }
        
        // Update action height for the game
        update_paint();
        // Always update the appropriate display based on current game state
        if (game.state == STATE_MENU) 
        {
            if(last_game.state != game.state || last_game.selected_option != game.selected_option)
                draw_menu(display);
        } 
        if (game.state == STATE_CONFIG ) 
        {
            if(last_game.state != game.state || last_game.config_selected_option != game.config_selected_option)
                draw_config(display);
        }
        if (game.state == STATE_GAME) 
        {
            if(last_game.state != game.state)
                display->test_pic();
            update_game_display(display, mapped_height, bar_height);
            sleep_ms(50);
        }
        if (game.state == STATE_GAME_OVER) 
        {
            if(last_game.state != game.state || last_game.selected_option != game.selected_option)
                draw_game_over(display);
        }
        last_game = game;

        // switch (game.state) {
        //     case STATE_MENU:
        //         draw_menu(display);
        //         break;
                
        //     case STATE_CONFIG:
        //         draw_config(display);
        //         break;
                
        //     case STATE_GAME:
        //         update_game_display(display, mapped_height, bar_height);
        //         break;
                
        //     case STATE_GAME_OVER:
        //         draw_game_over(display);
        //         break;
        // }
        // // Debug output every 5 seconds
        // if (current_time - last_debug_time > 5000) {
        //     printf("Core 0: Game running\n");
        //     last_debug_time = current_time;
        // }
        
        // sleep_ms(50); // Add a small delay to prevent CPU overload
    }
}

#include "ILI9341.h"
#include "systick.h"

#define MY_CLOCK (rcc_hse_25mhz_3v3[RCC_CLOCK_3V3_168MHZ])

#define LED_PORT GPIOA
#define LED_PIN GPIO8
#define LED_RCC_PORT RCC_GPIOA


ILI9341_t3 my_ILI(0, 0, 0, 0, 0, 0);

static void heartbeat(uint32_t msec_time)
{
    if (msec_time & 0x400)
        gpio_set(LED_PORT, LED_PIN);
    else
        gpio_clear(LED_PORT, LED_PIN);
}

static void setup_heartbeat(void)
{
    rcc_periph_clock_enable(LED_RCC_PORT);
    register_systick_handler(heartbeat);
}

static void setup(void)
{
    rcc_clock_setup_hse_3v3(&MY_CLOCK);

    setup_systick(MY_CLOCK.ahb_frequency);

    setup_heartbeat();

    my_ILI.begin();
    my_ILI.begin();
}

static uint16_t colors[] = {
    // ILI9341_BLACK,
    ILI9341_NAVY,
    ILI9341_DARKGREEN,
    ILI9341_DARKCYAN,
    ILI9341_MAROON,
    ILI9341_PURPLE,
    ILI9341_OLIVE,
    ILI9341_LIGHTGREY,
    ILI9341_DARKGREY,
    ILI9341_BLUE,
    ILI9341_GREEN,
    ILI9341_CYAN,
    ILI9341_RED,
    ILI9341_MAGENTA,
    ILI9341_YELLOW,
    ILI9341_WHITE,
    ILI9341_ORANGE,
    ILI9341_GREENYELLOW,
    ILI9341_PINK,
};
static size_t color_count = (&colors)[1] - colors;

static uint16_t buf[320/2][240];

static void fill_rect(int x0, int y0, int x1, int y1, uint16_t color)
{
    for (int y = y0; y < y1; y++)
        for (int x = x0; x < x1; x++)
            buf[y][x] = color;
}

static void run()
{
    my_ILI.fillScreen(ILI9341_NAVY);
    my_ILI.fillRect(50, 50, 150, 100, ILI9341_YELLOW);

    fill_rect(0, 0, 240, 320/2, ILI9341_BLACK);

    int left = 50, top = 50;
    int width = 240 - 100, height = 320/2 - 100;
    int xinc = +1, yinc = +1;

    for (int ci = 0; ; ci = (ci + 1) % color_count) {
        uint16_t color = colors[ci];
        fill_rect(left, top, left + width, top + height, color);
        my_ILI.writeRect(0, 0, 240, 320/2, buf[0]);
        my_ILI.writeRect(0, 320/2, 240, 320/2, buf[0]);

        // erase edges
        fill_rect(left, top, left + 1, top + height, ILI9341_BLACK);
        fill_rect(left + width - 1, top, left + width, top + height, ILI9341_BLACK);
        fill_rect(left, top, left + width, top + 1, ILI9341_BLACK);
        fill_rect(left, top + height - 2, left + width, top + height, ILI9341_BLACK);
        
        // delay_msec(20);

        left += xinc;
        top += yinc;
        if (left + width >= 240)
            xinc = -1;
        if (left == 0)
            xinc = +1;
        if (top + height >= 320/2)
            yinc = -2;
        if (top <= 1)
            yinc = +1;
    }
}

int main(void)
{
    setup();
    run();
}

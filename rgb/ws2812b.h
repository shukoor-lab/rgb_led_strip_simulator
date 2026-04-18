#ifndef WS2812B_H_
#define WS2812B_H_

#include <stdint.h>

/* ===== USER CONFIG ===== */
#define LED_COUNT   81          // <<< Set number of LEDs here
#define WS_PORT     PORTB
#define WS_DDR      DDRB
#define WS_PIN      PB0
#define WS_PIN_MASK (1 << WS_PIN)

#define COLOR_RED       0xFF0000
#define COLOR_GREEN     0x00FF00
#define COLOR_BLUE      0x0000FF
#define COLOR_YELLOW    0xFFFF00
#define COLOR_CYAN      0x00FFFF
#define COLOR_MAGENTA   0xFF00FF


/* ===== API ===== */
void ws2812_init(void);
void ws2812_set_pixel(uint16_t index, uint8_t r, uint8_t g, uint8_t b);
void ws2812_clear(void);
void ws2812_show(void);

#endif
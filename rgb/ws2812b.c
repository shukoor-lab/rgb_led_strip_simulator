#include "ws2812b.h"
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#define N 81

typedef struct {
    uint8_t r, g, b;
} RGB;

RGB strip[N];

void ws2812_init(void)
{
    
}


void ws2812_set_pixel(uint16_t index, uint8_t r, uint8_t g, uint8_t b) {
    if (index < N) {
        strip[index].r = r;
        strip[index].g = g;
        strip[index].b = b;
    }
}

void ws2812_show(void) {
    putchar('S');      // Sync Byte
    putchar((uint8_t)N); // Send the count N as a single byte
    fwrite(strip, sizeof(RGB), N, stdout);
    fflush(stdout);
}

void ws2812_clear() {
    for (int i = 0; i < N; i++) {
        strip[i].r = 0;
        strip[i].g = 0;
        strip[i].b = 0;
    }
}
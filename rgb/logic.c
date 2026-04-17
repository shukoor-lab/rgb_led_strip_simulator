#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#define N 81

typedef struct {
    uint8_t r, g, b;
} RGB;

RGB red = {255, 0, 0};
RGB green = {0, 255, 0};
RGB blue = {0, 0, 255};
RGB orange = {255, 165, 0};
RGB purple = {128, 0, 128};
RGB white = {255, 255, 255};
RGB yellow = {255, 255, 0};

RGB strip[N];

void set_pixel(uint8_t index, uint8_t r, uint8_t g, uint8_t b) {
    if (index < N) {
        strip[index].r = r;
        strip[index].g = g;
        strip[index].b = b;
    }
}

void rgb_flush() {
    putchar('S');      // Sync Byte
    putchar((uint8_t)N); // Send the count N as a single byte
    fwrite(strip, sizeof(RGB), N, stdout);
    fflush(stdout);
}

void clear_strip() {
    for (int i = 0; i < N; i++) {
        strip[i].r = 0;
        strip[i].g = 0;
        strip[i].b = 0;
    }
    rgb_flush();
}

void fill(uint8_t start_idx, uint8_t end_idx, RGB color) {
    if(start_idx >= N || end_idx >= N || start_idx > end_idx) {
        return; // Invalid indices
    }
    for (uint8_t i = start_idx; i <= end_idx && i < N; i++) {
        strip[i] = color;
    }
    rgb_flush();
}

void delay_ms(int milliseconds) {
    usleep(milliseconds * 1000); // Convert ms to microseconds
}


int main() {
    uint8_t frame = 0;
    while (1) {
        fill(0, 80, red); // Fill first 4 pixels with red
        delay_ms(1000); // 1 second delay
        
        fill(0, 80, green); // Fill next 3 pixels with green
        delay_ms(1000); // 1 second delay

        fill(0, 80, blue); // Fill last 3 pixels with blue
        delay_ms(1000); // 1 second delay

        clear_strip(); // Clear the strip after one cycle
        delay_ms(1000); // 1 second delay before next cycle

    }
    return 0;
}
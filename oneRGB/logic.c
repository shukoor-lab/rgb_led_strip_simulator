#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

// This mimics your LED strip memory
uint8_t current_r = 0;
uint8_t current_g = 0;
uint8_t current_b = 0;

void set_pixel(uint8_t r, uint8_t g, uint8_t b) {
    current_r = r;
    current_g = g;
    current_b = b;
}

void rgb_flush() {
    // We send a 1-byte header 'C' (for Color) so Python stays synced
    // Then we send the 3 raw bytes
    putchar('C');
    putchar(current_r);
    putchar(current_g);
    putchar(current_b);
    fflush(stdout); 
}

void delay_ms(int milliseconds) {
    usleep(milliseconds * 1000); // Convert ms to microseconds
}

int main() {
    while (1) {
        for(int i = 0; i < 256; i++) {
            set_pixel(i, 0, 0); // Red gradient
            rgb_flush();
            delay_ms(10);
        }
        for(int i = 0; i < 256; i++) {
            set_pixel(255, i, 0); // Green gradient
            rgb_flush();
            delay_ms(10);
        }
        for(int i = 255; i >= 0; i--) {
            set_pixel(i, 255, 0); // Red fade out
            rgb_flush();
            delay_ms(10);
        }
        for(int i = 0; i < 256; i++) {
            set_pixel(0, 255, i); // Blue gradient
            rgb_flush();
            delay_ms(10);
        }
        for(int i = 255; i >= 0; i--) {
            set_pixel(0, i, 255); // Green fade out
            rgb_flush();
            delay_ms(10);
        }
        for(int i = 0; i < 256; i++) {
            set_pixel(i, 0, 255); // Red gradient
            rgb_flush();
            delay_ms(10);
        }
        for(int i = 255; i >= 0; i--) {
            set_pixel(i, 0, 255); // Red fade out
            rgb_flush();
            delay_ms(10);
        }
    }
    return 0;
}
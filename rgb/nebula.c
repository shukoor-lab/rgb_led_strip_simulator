#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <math.h>

#define N 81

typedef struct { uint8_t r, g, b; } RGB;
RGB strip[N];

void set_pixel(uint8_t index, uint8_t r, uint8_t g, uint8_t b) {
    if (index < N) {
        strip[index].r = r;
        strip[index].g = g;
        strip[index].b = b;
    }
}

void rgb_flush() {
    putchar('S');      
    putchar((uint8_t)N); 
    fwrite(strip, sizeof(RGB), N, stdout);
    fflush(stdout);
}

// Helper for smooth room colors
// This creates a "Nebula" palette (Purples, Blues, Cyans)
RGB get_nebula_color(float phase) {
    // We use sine waves to oscillate R, G, and B independently
    uint8_t r = (uint8_t)((sin(phase) * 127) + 128);
    uint8_t g = (uint8_t)((sin(phase + 2.0) * 127) + 128);
    uint8_t b = (uint8_t)((sin(phase + 4.0) * 127) + 128);
    
    // Dim it down for better "ambient" feel (0.3 = 30% brightness)
    float brightness = 0.4;
    return (RGB){(uint8_t)(r * brightness), (uint8_t)(g * brightness), (uint8_t)(b * brightness)};
}

int main() {
    float offset = 0.0;

    while (1) {
        for (int i = 0; i < N; i++) {
            // This creates a moving wave across the strip
            // The 0.1 controls how "wide" the color bands are
            // The offset controls the movement speed
            float pixel_phase = offset + (i * 0.1);
            RGB col = get_nebula_color(pixel_phase);
            
            set_pixel(i, col.r, col.g, col.b);
        }

        rgb_flush();
        
        // Speed of the drift (smaller = slower/more relaxing)
        offset += 0.02; 
        
        // 33ms = 30fps for smooth motion
        usleep(33333); 
    }
    return 0;
}
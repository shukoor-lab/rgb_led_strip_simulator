#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#define N 81
#define TAIL_END 40
#define MAX_CONCURRENT 4  // Lowered to prevent overcrowding

typedef struct { uint8_t r, g, b; } RGB;

typedef struct {
    int active;
    int frame;
    RGB color;
} Cracker;

RGB strip[N];
Cracker crackers[MAX_CONCURRENT];

uint16_t get_random(uint16_t min, uint16_t max) {
    return (rand() % (max - min + 1)) + min;
}

void set_pixel(uint8_t index, uint8_t r, uint8_t g, uint8_t b) {
    if (index < N) {
        // Additive mixing
        uint16_t new_r = strip[index].r + r;
        uint16_t new_g = strip[index].g + g;
        uint16_t new_b = strip[index].b + b;
        strip[index].r = (new_r > 255) ? 255 : new_r;
        strip[index].g = (new_g > 255) ? 255 : new_g;
        strip[index].b = (new_b > 255) ? 255 : new_b;
    }
}

void rgb_flush() {
    putchar('S');      
    putchar((uint8_t)N); 
    fwrite(strip, sizeof(RGB), N, stdout);
    fflush(stdout);
}

void fade_strip(float factor) {
    for (int i = 0; i < N; i++) {
        strip[i].r = (uint8_t)(strip[i].r * factor);
        strip[i].g = (uint8_t)(strip[i].g * factor);
        strip[i].b = (uint8_t)(strip[i].b * factor);
    }
}

void spawn_cracker() {
    for (int i = 0; i < MAX_CONCURRENT; i++) {
        if (!crackers[i].active) {
            crackers[i].active = 1;
            crackers[i].frame = 0;
            // High brightness colors
            crackers[i].color = (RGB){get_random(180, 255), get_random(100, 255), get_random(50, 200)};
            break; 
        }
    }
}

int main() {
    srand(time(NULL));
    for(int i=0; i<MAX_CONCURRENT; i++) crackers[i].active = 0;

    while (1) {
        fade_strip(0.82); 

        // --- NEW SPAWN PROBABILITY ---
        // 5 in 1000 chance (0.5% per frame)
        // At 30 FPS, this averages one cracker every ~6.5 seconds.
        if (get_random(0, 1000) < 5) { 
            spawn_cracker();
        }

        for (int i = 0; i < MAX_CONCURRENT; i++) {
            if (!crackers[i].active) continue;

            // FUSE STAGE: 3 logical frames per LED (Very slow & deliberate)
            if (crackers[i].frame <= TAIL_END * 3) {
                int phys_idx = crackers[i].frame / 3; 
                float dim = 1.0 - ((float)phys_idx / TAIL_END * 0.75);
                
                // Ember-like flicker
                uint8_t flicker = get_random(0, 40);
                set_pixel(phys_idx, (uint8_t)(210*dim)+flicker, (uint8_t)(100*dim), (uint8_t)(20*dim));
                crackers[i].frame++;
            } 
            // EXPLOSION STAGE
            else if (crackers[i].frame == (TAIL_END * 3) + 1) {
                for (int j = 41; j < N; j++) {
                    set_pixel(j, crackers[i].color.r, crackers[i].color.g, crackers[i].color.b);
                }
                crackers[i].frame++;
            }
            // DECAY STAGE
            else if (crackers[i].frame < (TAIL_END * 3) + 45) {
                for (int j = 0; j < 2; j++) {
                    int spark = get_random(41, 80);
                    set_pixel(spark, crackers[i].color.r, crackers[i].color.g, crackers[i].color.b);
                }
                crackers[i].frame++;
            }
            else {
                crackers[i].active = 0;
            }
        }

        rgb_flush();
        usleep(33333); // 30 FPS
    }
    return 0;
}
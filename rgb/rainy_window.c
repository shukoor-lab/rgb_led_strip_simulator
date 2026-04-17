#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

// --- GEOMETRY SETTINGS ---
#define N 81
#define TAIL_END 40
#define BOTTOM_POINT 63

// --- SPAWN PROBABILITY (0 to 100) ---
#define SPAWN_CHANCE_AREA_1 8   // Flat surface (0-35)
#define SPAWN_CHANCE_AREA_2 12  // Circle Left (40-50)
#define SPAWN_CHANCE_AREA_3 12   // Circle Right (70-80) -> Lowered this for you!

// --- PHYSICS SPEEDS (0 to 10) ---
#define TAIL_SPREAD_CHANCE 3    // How likely water moves on flat part
#define CIRCLE_GRAVITY_CHANCE 8 // How fast drops race to the bottom

// --- DYNAMICS & FADING ---
#define GLOBAL_EVAPORATION 0.95 // How fast everything dries (lower = faster)
#define TRAIL_RETENTION 0.05    // How much "tail" is left behind (0.0 to 1.0)
#define POOL_EVAPORATION 0.30   // How fast the puddle at 63 dries up

// --- COLOR MIX (0 to 255) ---
#define COLOR_R 40
#define COLOR_G 120
#define COLOR_B 255
#define BG_GLOW 10              // Subtle blue light for the whole strip

typedef struct { uint8_t r, g, b; } RGB;

int get_random(int min, int max);
float rain_layer[N]; 
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

int main() {
    srand(time(NULL));
    for(int i=0; i<N; i++) rain_layer[i] = 0.0;

    while (1) {
        // --- 1. ZONED SPAWN LOGIC ---
        if (get_random(0, 100) < SPAWN_CHANCE_AREA_1) rain_layer[get_random(0, 35)] = 1.0;
        if (get_random(0, 100) < SPAWN_CHANCE_AREA_2) rain_layer[get_random(40, 50)] = 1.0;
        if (get_random(0, 100) < SPAWN_CHANCE_AREA_3) rain_layer[get_random(70, 80)] = 1.0;

        float next_frame[N];
        for(int i=0; i<N; i++) next_frame[i] = rain_layer[i] * GLOBAL_EVAPORATION; 

        // --- 2. PHYSICS LOGIC ---
        for (int i = 0; i < N; i++) {
            if (rain_layer[i] < 0.01) continue;

            if (i <= TAIL_END) {
                if (get_random(0, 10) < TAIL_SPREAD_CHANCE) { 
                    if (i > 0) next_frame[i-1] += rain_layer[i] * 0.35;
                    if (i < TAIL_END) next_frame[i+1] += rain_layer[i] * 0.35;
                    next_frame[i] *= 0.4; 
                }
            } 
            else {
                if (i == BOTTOM_POINT) {
                    next_frame[i] = rain_layer[i] * POOL_EVAPORATION;
                } else {
                    if (get_random(0, 10) < CIRCLE_GRAVITY_CHANCE) { 
                        int direction = (i < BOTTOM_POINT) ? 1 : -1;
                        next_frame[i + direction] += rain_layer[i] * 0.8;
                        next_frame[i] = rain_layer[i] * TRAIL_RETENTION; 
                    }
                }
            }
        }

        for(int i=0; i<N; i++) {
            rain_layer[i] = (next_frame[i] > 1.0) ? 1.0 : next_frame[i];
        }

        // --- 3. RENDER ---
        for (int i = 0; i < N; i++) {
            set_pixel(i, 
                (uint8_t)(rain_layer[i] * COLOR_R), 
                (uint8_t)(rain_layer[i] * COLOR_G), 
                (uint8_t)(rain_layer[i] * COLOR_B + BG_GLOW)
            );
        }

        rgb_flush();
        usleep(30000); 
    }
    return 0;
}

int get_random(int min, int max) {
    if (max - min + 1 <= 0) return min;
    return (rand() % (max - min + 1)) + min;
}
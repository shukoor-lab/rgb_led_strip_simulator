#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include "display.h"

void delay_ms(int milliseconds) {
    usleep(milliseconds * 1000); // Convert ms to microseconds
}


int main() {
    display_init();
    display_set_animation(ANIM_TWILIGHT);

    while (1)
    {
        display_update();
        delay_ms(15);
    }
    
    return 0;
}
#include <stdio.h>
#include <unistd.h> // For usleep (simulated delay)

#define LED_PIN 0

void set_led_state(int state) {
    // In simulation, we print to stdout for Python to read
    if (state) {
        printf("LED_ON\n");
    } else {
        printf("LED_OFF\n");
    }
    fflush(stdout); // Crucial: forces data through the pipe immediately
}

void delay_ms(int milliseconds) {
    usleep(milliseconds * 1000); // Convert ms to microseconds
}

int main() {
    while (1) {
        set_led_state(1); // LED High
        delay_ms(200);    // 200ms delay
        
        set_led_state(0); // LED Low
        delay_ms(2000);    // 2000ms delay
    }
    return 0;
}
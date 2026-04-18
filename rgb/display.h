#ifndef DISPLAY_H
#define DISPLAY_H

/* ============================= */
/* Animation Type (Public)       */
/* ============================= */

typedef enum {
    ANIM_OFF = 0,
    ANIM_FILL,
    ANIM_UNFILL,
    ANIM_PACKETS,
    ANIM_BLINK,
    ANIM_RAINBOW,
    ANIM_THEATER,
    ANIM_FIRE,
    ANIM_WATER,
    ANIM_BLEED,
    ANIM_STARS,
    ANIM_MOON,
    ANIM_FIRECRACKER,
    ANIM_FIREFLIES,
    ANIM_RAIN,
    TOTAL_ANIMS
} anim_t;

#define ANIM_START_INDEX  4

extern float fill_progress;

/* ============================= */
/* Public API                    */
/* ============================= */

void display_init(void);

/* Call every 10ms */
void display_update(void);

/* Change current animation */
void display_set_animation(anim_t anim);

anim_t display_get_animation(void);

#endif
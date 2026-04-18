#include "display.h"
#include "ws2812b.h"

float fill_progress = 0.0f;

/* ============================= */
/* Private State                 */
/* ============================= */

static anim_t current_anim = ANIM_OFF;
static anim_t previous_anim = ANIM_OFF;

static uint16_t anim_tick = 0;

/* ============================= */
/* Function Declaration          */
/* ============================= */

static void render_anim_off(void);
static void render_anim_blink(void);
static void render_anim_rainbow(void);
static void render_anim_theater(void);
static void render_anim_fire(void);
static void render_anim_water(void);
static void render_anim_packets(void);
static void render_anim_bleed(void);
static void render_anim_stars(void);
static void render_anim_moon(void);
static void render_anim_firecracker(void);
static void render_anim_fill(void);
static void render_anim_unfill(void);

/* ============================= */
/* Animation Tick Lengths       */
/* ============================= */

#define OFF_TICK_LEN     1
#define BLINK_TICK_LEN   100   /* 100 * 10ms = 1 second */
#define RAINBOW_TICK_LEN 256
#define THEATER_TICK_LEN 30
#define FIRE_TICK_LEN 255
#define COLOR_BOUNCE_TICK_LEN 160
#define WATER_TICK_LEN 255
#define PACKET_TICK_LEN  238
#define BLEED_TICK_LEN           3000
#define STAR_TICK_LEN           60000

/* ============================= */
/* Public API                    */
/* ============================= */

void display_init(void) {
    ws2812_init();
    display_set_animation(ANIM_OFF);
}

void display_set_animation(anim_t anim)
{
    current_anim = anim;
    if(anim == ANIM_OFF) fill_progress = 0.0f;
}

anim_t display_get_animation(void)
{
    return current_anim;
}

void display_update(void)
{
    /* Detect animation change */
    if (current_anim != previous_anim)
    {
        anim_tick = 0;              /* Reset tick on animation change */
        previous_anim = current_anim;
    }

    /* Dispatch correct animation */
    switch (current_anim)
    {
        case ANIM_OFF:
            render_anim_off();
            break;

        case ANIM_BLINK:
            render_anim_blink();
            break;

        case ANIM_RAINBOW:
            render_anim_rainbow();
            break;

        case ANIM_THEATER:
            render_anim_theater();
            break;

        case ANIM_FIRE:
            render_anim_fire();
            break;

        case ANIM_WATER:
            render_anim_water();
            break;

        case ANIM_PACKETS:
            render_anim_packets();
            break;

        case ANIM_BLEED:
            render_anim_bleed();
            break;

        case ANIM_STARS:
            render_anim_stars();
            break;

        case ANIM_MOON:
            render_anim_moon();
            break;

        case ANIM_FIRECRACKER:
            render_anim_firecracker();
            break;

        case ANIM_FILL:
            render_anim_fill();
            break;

        case ANIM_UNFILL:
            render_anim_unfill();
            break;

        default:
            render_anim_off();
            break;
    }
    ws2812_show();
}

/* ============================= */
/* Animation Renderers           */
/* ============================= */

static void render_anim_off(void)
{
    ws2812_clear();

    anim_tick++;
    if (anim_tick >= OFF_TICK_LEN)
    {
        anim_tick = 0;
    }
}

static void render_anim_blink(void)
{
    /* Blink red ON for half period, OFF for half period */

    for(uint8_t i=0; i<LED_COUNT; i++) ws2812_set_pixel(i, 0, 0, 0);

    if (anim_tick < (BLINK_TICK_LEN / 2))
    {
        ws2812_set_pixel(0, 255, 0, 0); // Red
    }

    anim_tick++;
    if (anim_tick >= BLINK_TICK_LEN)
    {
        anim_tick = 0;
    }
}

static uint32_t wheel(uint8_t pos)
{
    uint8_t r, g, b;

    pos = 255 - pos;

    if (pos < 85)
    {
        r = 255 - pos * 3;
        g = 0;
        b = pos * 3;
    }
    else if (pos < 170)
    {
        pos -= 85;
        r = 0;
        g = pos * 3;
        b = 255 - pos * 3;
    }
    else
    {
        pos -= 170;
        r = pos * 3;
        g = 255 - pos * 3;
        b = 0;
    }

    return ((uint32_t)r << 16) |
           ((uint32_t)g << 8)  |
           ((uint32_t)b);
}

static void render_anim_rainbow(void)
{
    for (uint8_t i = 0; i < LED_COUNT; i++)
    {
        uint8_t color_index = (i * 256 / LED_COUNT + anim_tick) & 0xFF;
        uint32_t c = wheel(color_index);

        ws2812_set_pixel(i,
                         (c >> 16) & 0xFF,
                         (c >> 8) & 0xFF,
                         c & 0xFF);
    }

    anim_tick++;
    if (anim_tick >= RAINBOW_TICK_LEN)
        anim_tick = 0;
}

static void render_anim_theater(void)
{
    ws2812_clear();

    uint8_t phase = anim_tick % 3;

    for (uint8_t i = phase; i < LED_COUNT; i += 3)
        ws2812_set_pixel(i, 255, 255, 255);

    anim_tick++;
    if (anim_tick >= THEATER_TICK_LEN)
        anim_tick = 0;
}

/* --------------------------------------------------------------------------------------------------------- */

static uint16_t lfsr = 0xACE1u;

static uint8_t fast_rand(void)
{
    lfsr ^= lfsr << 7;
    lfsr ^= lfsr >> 9;
    lfsr ^= lfsr << 8;
    return (uint8_t)lfsr;
}


#define FIRE_BASE_HEIGHT     30
#define FIRE_MAX_HEIGHT      40

#define SECOND_CENTER        65
#define SECOND_HALF_WIDTH    8
#define SECOND_EXTRA_HEIGHT  7

static uint8_t main_flame_level = FIRE_BASE_HEIGHT;
static uint8_t second_flame_level = SECOND_HALF_WIDTH;

static void render_anim_fire(void)
{
    /* ===============================
       NATURAL MAIN FLAME DRIFT
    =============================== */

    if ((anim_tick & 0x07) == 0)   // update slowly (every ~80ms)
    {
        int8_t change = (fast_rand() & 0x03) - 1;  // -1,0,+1

        int16_t new_level = main_flame_level + change;

        if (new_level < FIRE_BASE_HEIGHT)
            new_level = FIRE_BASE_HEIGHT;

        if (new_level > FIRE_MAX_HEIGHT)
            new_level = FIRE_MAX_HEIGHT;

        main_flame_level = new_level;
    }

    /* ===============================
       NATURAL SECOND FLAME DRIFT
    =============================== */

    if ((anim_tick & 0x0F) == 0)   // slower drift
    {
        int8_t change2 = (fast_rand() & 0x03) - 1;

        int16_t new_level2 = second_flame_level + change2;

        uint8_t max2 = SECOND_HALF_WIDTH + SECOND_EXTRA_HEIGHT;

        if (new_level2 < SECOND_HALF_WIDTH)
            new_level2 = SECOND_HALF_WIDTH;

        if (new_level2 > max2)
            new_level2 = max2;

        second_flame_level = new_level2;
    }

    /* ===============================
       RENDER LOOP
    =============================== */

    for (uint8_t i = 0; i < LED_COUNT; i++)
    {
        uint16_t total_heat = 0;

        /* ---------- MAIN FIRE ---------- */
        if (i < main_flame_level)
        {
            uint8_t base_heat = 255 - (i * 6);
            uint8_t flicker = fast_rand() & 0x3F;

            uint8_t heat = (base_heat > flicker) ? base_heat - flicker : 0;
            total_heat += heat;
        }

        /* ---------- SECOND FIRE ---------- */

        int8_t dist = i - SECOND_CENTER;
        if (dist < 0) dist = -dist;

        if (dist <= second_flame_level)
        {
            uint8_t local = (second_flame_level - dist) << 4;
            uint8_t flicker2 = fast_rand() & 0x1F;

            uint8_t heat2 = (local > flicker2) ? local - flicker2 : 0;
            total_heat += heat2;
        }

        if (total_heat > 255)
            total_heat = 255;

        /* ---------- COLOR MAP ---------- */

        uint8_t r = total_heat;
        uint8_t g = (total_heat > 100) ? (total_heat - 100) : 0;

        ws2812_set_pixel(i, r, g, 0);
    }

    anim_tick++;
    if (anim_tick >= FIRE_TICK_LEN)
        anim_tick = 0;
}

/* --------------------------------------------------------------------------------------------------------- */

static uint8_t water_energy = 10;   // small dynamic variation

static uint8_t triwave(uint8_t x)
{
    if (x & 0x80)
        return 255 - ((x & 0x7F) << 1);
    else
        return (x & 0x7F) << 1;
}

static void render_anim_water(void)
{
    /* ===============================
       Slowly vary wave energy
       (natural sea breathing)
    =============================== */
    if ((anim_tick & 0x1F) == 0)   // every ~320ms
    {
        int8_t delta = (fast_rand() & 0x03) - 1;
        int8_t new_energy = water_energy + delta;

        if (new_energy < 6)  new_energy = 6;
        if (new_energy > 22) new_energy = 22;

        water_energy = new_energy;
    }

    for (uint8_t i = 0; i < LED_COUNT; i++)
    {
        uint8_t brightness = 0;

        /* =========================
           LINEAR SHORE (0–40)
        ========================= */
        if (i <= 40)
        {
            uint8_t wave_pos = (i * 5) + (anim_tick * 2);
            uint8_t wave = triwave(wave_pos);

            brightness = (wave >> 2) + water_energy;
        }

        /* =========================
           CIRCULAR RIPPLE (41–80)
        ========================= */
        else if (i >= 41 && i <= 80)
        {
            uint8_t ring_pos = i - 41;   // 0..39

            uint8_t ripple = triwave((ring_pos * 7) + (anim_tick * 3));

            brightness = ripple >> 2;
        }

        /* Subtle shimmer */
        brightness += fast_rand() & 0x05;

        if (brightness > 255)
            brightness = 255;

        /* =========================
           NATURAL OCEAN COLOR MAP
        ========================= */

        uint8_t r, g, b;

        if (brightness > 180)
        {
            /* Wave foam */
            uint8_t foam = brightness - 180;
            foam += fast_rand() & 0x03;  // slight sparkle

            r = 170 + foam;
            g = 200 + foam;
            b = 220 + foam;
        }
        else
        {
            /* Deep teal water */
            r = brightness >> 5;             // very low red
            g = (brightness * 3) >> 2;       // teal bias
            b = brightness;                  // blue body
        }

        ws2812_set_pixel(i, r, g, b);
    }

    anim_tick++;
    if (anim_tick >= WATER_TICK_LEN)
        anim_tick = 0;
}

/* --------------------------------------------------------------------------------------------------------- */

#define PACKET_LEN       3

static void render_anim_packets(void)
{
    uint8_t pointer[3] = {0, 14, 28};

    if(anim_tick == 0){
        ws2812_clear();
    }

    uint8_t idx = anim_tick / 2.5;

    if(idx < 81){
        for (uint8_t j = 0; j < 3; j++)
        {
            for(int j = 0; j < 3; j++){
                if(idx - pointer[j] >= 0 && idx - pointer[j] < LED_COUNT) {
                    ws2812_set_pixel(idx - pointer[j], 130, 90, 40);
                }
                if(idx - pointer[j] - PACKET_LEN >= 0 && idx - pointer[j] - PACKET_LEN < LED_COUNT) {
                    ws2812_set_pixel(idx - pointer[j] - PACKET_LEN, 0, 0, 0);
                }
            }
        }
    }else{
        for(int j = 0; j < 3; j++){
            uint16_t pv_idx = idx - pointer[j];
            if (pv_idx >= LED_COUNT) {
                pv_idx -= 42;
            }
            if(pv_idx >= 40 && pv_idx < LED_COUNT) {
                ws2812_set_pixel(pv_idx, 130, 90, 40);
            }
            pv_idx = idx - pointer[j] - PACKET_LEN;
            if (pv_idx >= LED_COUNT) {
                pv_idx -= 42;
            }
            if(pv_idx >= 0 && pv_idx < LED_COUNT) {
                ws2812_set_pixel(pv_idx, 0, 0, 0);
            }
        }
    }
    

    anim_tick++;
    if (anim_tick >= PACKET_TICK_LEN)
    {
        // ws2812_clear();  // Clear strip at end of cycle
        anim_tick = 203;
    }
}

/* --------------------------------------------------------------------------------------------------------- */

#define BLEED_MAX_GROW_LEN       30

#define BLEED_GROW_STEP_FP       10     // 8/256 per tick (~0.03 LED per tick)
                                       // smaller = slower smoother growth
#define BLEED_WAIT_FULL_MIN      200   // 1 second
#define BLEED_WAIT_FULL_MAX      500   // 2 seconds

#define BLEED_WAIT_EMPTY_MIN     300   // 2 second
#define BLEED_WAIT_EMPTY_MAX     600   // 5 seconds

static uint8_t  bleed_center = 0;
static uint16_t bleed_radius_fp = 0;   // 8.8 fixed point
static uint8_t  bleed_target = 0;
static uint8_t  bleed_phase = 3;       // 0=grow,1=wait_full,2=shrink,3=wait_empty
static uint16_t bleed_wait = 0;

static void render_anim_bleed(void)
{
    ws2812_clear();

    /* ------------ STATE MACHINE ------------ */

    switch (bleed_phase)
    {
        case 3: // wait empty
            if (bleed_wait > 0)
            {
                bleed_wait--;
            }
            else
            {
                bleed_center = fast_rand() % LED_COUNT;
                bleed_target = 4 + (fast_rand() % BLEED_MAX_GROW_LEN);
                bleed_radius_fp = 0;
                bleed_phase = 0;
            }
            break;

        case 0: // grow
            bleed_radius_fp += BLEED_GROW_STEP_FP;

            if (bleed_radius_fp >= ((uint16_t)bleed_target << 8))
            {
                bleed_radius_fp = ((uint16_t)bleed_target << 8);
                bleed_phase = 1;

                bleed_wait =
                    BLEED_WAIT_FULL_MIN +
                    (fast_rand() % (BLEED_WAIT_FULL_MAX - BLEED_WAIT_FULL_MIN));
            }
            break;

        case 1: // wait full
            if (bleed_wait > 0)
                bleed_wait--;
            else
                bleed_phase = 2;
            break;

        case 2: // shrink
            if (bleed_radius_fp > BLEED_GROW_STEP_FP)
            {
                bleed_radius_fp -= BLEED_GROW_STEP_FP;
            }
            else
            {
                bleed_radius_fp = 0;
                bleed_phase = 3;

                bleed_wait =
                    BLEED_WAIT_EMPTY_MIN +
                    (fast_rand() % (BLEED_WAIT_EMPTY_MAX - BLEED_WAIT_EMPTY_MIN));
            }
            break;
    }

    /* ------------ TRUE ANALOG RENDER ------------ */

    if (bleed_phase != 3)
    {
        uint32_t max_radius_fp = (uint32_t)bleed_target << 8;

        for (int16_t i = 0; i < LED_COUNT; i++)
        {
            int16_t dist = i - bleed_center;
            if (dist < 0) dist = -dist;

            uint32_t dist_fp = (uint32_t)dist << 8;

            if (dist_fp < bleed_radius_fp)
            {
                /* Fractional strength */
                uint32_t strength_fp = bleed_radius_fp - dist_fp;

                /* Normalize against max radius */
                uint32_t norm =
                    (strength_fp * 255) / max_radius_fp;

                /* Square for organic falloff */
                uint8_t brightness =
                    (uint32_t)norm * norm >> 8;

                ws2812_set_pixel(i, brightness, 0, 0);
            }
        }
    }

    anim_tick++;
    if (anim_tick >= BLEED_TICK_LEN)
        anim_tick = 0;
}

/* --------------------------------------------------------------------------------------------------------- */

#define STAR_MAX_COUNT          6

#define STAR_MAX_BRIGHTNESS     4    // overall star brightness
#define STAR_MIN_LIFE_TICKS     2500  // 25 sec
#define STAR_MAX_LIFE_TICKS     5000  // 50 sec

#define STAR_TWINKLE_SPEED      10     // small shimmer speed
#define STAR_TWINKLE_DEPTH      4     // very slight flicker (keep small)

#define STAR_SPAWN_CHANCE       1     // low probability per frame

#define SHOOTING_STAR_BRIGHTNESS    2
#define SHOOTING_STAR_SPEED_TICKS   1     // move every 1 ticks (10ms)
#define SHOOTING_STAR_TAIL_LEN      1

#define SHOOTING_STAR_MIN_COOLDOWN  30000   // 300s = 5 min
#define SHOOTING_STAR_MAX_COOLDOWN  36000   // 360s = 6 min

typedef struct
{
    uint8_t  active;
    uint8_t  pos;
    uint16_t age;
    uint16_t life;
    uint16_t phase;
    uint8_t  base_brightness;
} star_t;

typedef struct
{
    uint8_t active;
    int8_t  direction;      // +1 or -1
    int16_t pos;
    uint16_t move_tick;
    uint16_t cooldown;
} shooting_star_t;

static shooting_star_t shoot = {0};

static star_t stars[STAR_MAX_COUNT];

static uint8_t star_position_is_free(uint8_t pos)
{
    for (uint8_t i = 0; i < STAR_MAX_COUNT; i++)
    {
        if (!stars[i].active)
            continue;

        int16_t diff = stars[i].pos - pos;
        if (diff < 0) diff = -diff;

        if (diff <= 2)
            return 0;   // too close
    }

    return 1;
}

static uint8_t wave8(uint16_t x)
{
    x &= 0xFF;
    return (x < 128) ? x : 255 - x;
}

static void render_anim_stars(void)
{
    ws2812_clear();  // pitch black always

    /* ---------- RANDOM SPAWN ---------- */
    if ((fast_rand() % 1000) < STAR_SPAWN_CHANCE)
    {
        for (uint8_t i = 0; i < STAR_MAX_COUNT; i++)
        {
            if (!stars[i].active)
            {
                uint8_t new_pos = fast_rand() % LED_COUNT;

                if (!star_position_is_free(new_pos))
                    break;   // reject spawn

                stars[i].active = 1;
                stars[i].pos = new_pos;
                stars[i].age = 0;

                stars[i].life =
                    STAR_MIN_LIFE_TICKS +
                    (fast_rand() %
                    (STAR_MAX_LIFE_TICKS - STAR_MIN_LIFE_TICKS));

                stars[i].phase = fast_rand();

                stars[i].base_brightness =
                    STAR_MAX_BRIGHTNESS *
                    (70 + (fast_rand() % 30)) / 100;

                break;
            }
        }
    }

    /* ---------- UPDATE STARS ---------- */
    for (uint8_t i = 0; i < STAR_MAX_COUNT; i++)
    {
        if (!stars[i].active)
            continue;

        stars[i].age++;

        if (stars[i].age >= stars[i].life)
        {
            stars[i].active = 0;   // disappear instantly
            continue;
        }

        /* ---- slight twinkle (never off) ---- */
        stars[i].phase += STAR_TWINKLE_SPEED;

        uint8_t flicker = wave8(stars[i].phase);

        uint8_t dip = (flicker * STAR_TWINKLE_DEPTH) / 255;

        uint8_t brightness =
            stars[i].base_brightness - dip;

        ws2812_set_pixel(stars[i].pos,
                         brightness,
                         brightness,
                         brightness);
    }

    /* ---------- SHOOTING STAR COOLDOWN ---------- */
    if (!shoot.active)
    {
        if (shoot.cooldown > 0)
        {
            shoot.cooldown--;
        }
        else
        {
            /* spawn shooting star */
            shoot.active = 1;
            shoot.direction = (fast_rand() & 1) ? 1 : -1;

            if (shoot.direction == 1)
                shoot.pos = 0;
            else
                shoot.pos = 40;

            shoot.move_tick = 0;

            shoot.cooldown =
                SHOOTING_STAR_MIN_COOLDOWN +
                (fast_rand() %
                (SHOOTING_STAR_MAX_COOLDOWN - SHOOTING_STAR_MIN_COOLDOWN));
        }
    }

    /* ---------- SHOOTING STAR ACTIVE ---------- */
    if (shoot.active)
    {
        shoot.move_tick++;

        if (shoot.move_tick >= SHOOTING_STAR_SPEED_TICKS)
        {
            shoot.move_tick = 0;
            shoot.pos += shoot.direction;
        }

        /* out of linear region? */
        if (shoot.pos < 0 || shoot.pos > 40)
        {
            shoot.active = 0;
        }
        else
        {
            /* draw head */
            ws2812_set_pixel(shoot.pos,
                            SHOOTING_STAR_BRIGHTNESS,
                            SHOOTING_STAR_BRIGHTNESS,
                            SHOOTING_STAR_BRIGHTNESS);

            /* draw tail */
            for (uint8_t t = 1; t <= SHOOTING_STAR_TAIL_LEN; t++)
            {
                int16_t tail_pos =
                    shoot.pos - (t * shoot.direction);

                if (tail_pos >= 0 && tail_pos <= 40)
                {
                    uint8_t tail_brightness =
                        SHOOTING_STAR_BRIGHTNESS *
                        (SHOOTING_STAR_TAIL_LEN - t) /
                        SHOOTING_STAR_TAIL_LEN;

                    ws2812_set_pixel(tail_pos,
                                    tail_brightness,
                                    tail_brightness,
                                    tail_brightness);
                }
            }
        }
    }

    anim_tick++;
    if (anim_tick >= STAR_TICK_LEN)
        anim_tick = 0;
}

/* --------------------------------------------------------------------------------------------------------- */

#define MOON_TICK_LEN       65535
#define MOON_SPEED          1
#define MOON_MAX_BRIGHT     20

static uint8_t soft_wave(uint16_t x)
{
    x &= 0xFF;
    return (x < 128) ? x : 255 - x;
}

static void render_anim_moon(void)
{
    ws2812_clear();

    for (uint8_t i = 0; i < LED_COUNT; i++)
    {
        uint16_t drift = (i * 3) + (anim_tick * MOON_SPEED);

        uint8_t wave = soft_wave(drift);

        uint8_t brightness = (wave >> 4);

        if (brightness > MOON_MAX_BRIGHT)
            brightness = MOON_MAX_BRIGHT;

        /* Slight shimmer */
        brightness += fast_rand() & 0x01;

        uint8_t r = brightness >> 2;
        uint8_t g = brightness >> 1;
        uint8_t b = brightness;

        ws2812_set_pixel(i, r, g, b);
    }

    anim_tick++;
}

/* --------------------------------------------------------------------------------------------------------- */

#define FIRECRACKER_TICK_LEN        65535

#define ROCKET_SPEED_TICKS          2
#define ROCKET_BRIGHTNESS           60

#define SPARK_COUNT                 8
#define SPARK_MAX_RADIUS            10
#define SPARK_GROW_SPEED            1
#define SPARK_FADE_SPEED            3

#define FIRECRACKER_MIN_DELAY       200
#define FIRECRACKER_MAX_DELAY       500 

typedef struct
{
    uint8_t state;     // 0 idle, 1 rocket, 2 explode
    uint16_t delay;

    int16_t rocket_pos;
    uint16_t move_tick;

    uint8_t radius;
    uint8_t max_radius;
    uint8_t brightness;

    int8_t spark_dir[SPARK_COUNT];   // small directions
} firecracker_t;

static firecracker_t fire = {0};

static void render_anim_firecracker(void)
{
    ws2812_clear();

    /* ---------- IDLE ---------- */
    if (fire.state == 0)
    {
        if (fire.delay > 0)
        {
            fire.delay--;
        }
        else
        {
            fire.state = 1;
            fire.rocket_pos = fast_rand() % 40;   // start in linear strip
            fire.move_tick = 0;
        }
    }

    /* ---------- ROCKET ---------- */
    else if (fire.state == 1)
    {
        fire.move_tick++;

        if (fire.move_tick >= ROCKET_SPEED_TICKS)
        {
            fire.move_tick = 0;
            fire.rocket_pos++;
        }

        /* draw rocket */
        if (fire.rocket_pos >= 0 && fire.rocket_pos <= 80)
        {
            ws2812_set_pixel(fire.rocket_pos,
                             ROCKET_BRIGHTNESS,
                             ROCKET_BRIGHTNESS / 2,
                             0);
        }

        /* reached circle */
        if (fire.rocket_pos >= 41)
        {
            fire.state = 2;

            fire.radius = 0;
            fire.max_radius = 4 + (fast_rand() % SPARK_MAX_RADIUS);
            fire.brightness = 100;

            /* random spark directions */
            for (uint8_t i = 0; i < SPARK_COUNT; i++)
            {
                int8_t dir = (fast_rand() % 5) - 2;  // -2 to +2
                if (dir == 0) dir = 1;
                fire.spark_dir[i] = dir;
            }
        }
    }

    /* ---------- EXPLOSION (SPARKLE) ---------- */
    else if (fire.state == 2)
    {
        int16_t center = fire.rocket_pos;

        /* draw sparks */
        for (uint8_t s = 0; s < SPARK_COUNT; s++)
        {
            int16_t spark_pos =
                center + (fire.spark_dir[s] * fire.radius);

            /* wrap inside circle region (41–80) */
            if (spark_pos < 41)
                spark_pos = 41 + (spark_pos - 41 + 40) % 40;

            if (spark_pos > 80)
                spark_pos = 41 + (spark_pos - 41) % 40;

            if (spark_pos >= 41 && spark_pos <= 80)
            {
                uint8_t r = fire.brightness;
                uint8_t g = fire.brightness >> 3;
                uint8_t b = 0;

                ws2812_set_pixel(spark_pos, r, g, b);
            }
        }

        /* grow outward */
        if (fire.radius < fire.max_radius)
        {
            fire.radius += SPARK_GROW_SPEED;
        }
        else
        {
            /* fade */
            if (fire.brightness > SPARK_FADE_SPEED)
            {
                fire.brightness -= SPARK_FADE_SPEED;
            }
            else
            {
                fire.state = 0;
                fire.delay = FIRECRACKER_MIN_DELAY +
                    (fast_rand() %
                    (FIRECRACKER_MAX_DELAY - FIRECRACKER_MIN_DELAY));
            }
        }
    }

    anim_tick++;
}

/* --------------------------------------------------------------------------------------------------------- */


#define FILL_TOTAL_TICKS      400
#define FILL_FADE_WIDTH       10.0f
#define FILL_MAX_BRIGHTNESS   80

static const float fill_step = 1.0f / FILL_TOTAL_TICKS;

static uint8_t clamp_u8(int16_t v)
{
    if (v < 0) return 0;
    if (v > 255) return 255;
    return (uint8_t)v;
}

void render_anim_fill(void)
{
    /* advance progress */
    fill_progress += fill_step;
    if (fill_progress > 1.0f)
        fill_progress = 1.0f;

    float front = fill_progress * LED_COUNT;

    for (uint8_t i = 0; i < LED_COUNT; i++)
    {
        float dist = front - i;
        float intensity = 0.0f;

        if (dist >= 0)
        {
            intensity = dist / FILL_FADE_WIDTH;
            if (intensity > 1.0f)
                intensity = 1.0f;
        }

        uint8_t brightness = (uint8_t)(FILL_MAX_BRIGHTNESS * intensity);

        uint8_t r = brightness;
        uint8_t g = brightness * 0.7f;
        uint8_t b = brightness * 0.3f;

        ws2812_set_pixel(i,
                         clamp_u8(r),
                         clamp_u8(g),
                         clamp_u8(b));
    }
}

void render_anim_unfill(void)
{
    /* retreat progress */
    fill_progress -= fill_step * 4;
    if (fill_progress < 0.0f)
        fill_progress = 0.0f;

    float front = fill_progress * LED_COUNT;

    for (uint8_t i = 0; i < LED_COUNT; i++)
    {
        float dist = front - i;
        float intensity = 0.0f;

        if (dist >= 0)
        {
            intensity = dist / FILL_FADE_WIDTH;
            if (intensity > 1.0f)
                intensity = 1.0f;
        }

        uint8_t brightness = (uint8_t)(FILL_MAX_BRIGHTNESS * intensity);

        uint8_t r = brightness;
        uint8_t g = brightness * 0.7f;
        uint8_t b = brightness * 0.3f;

        ws2812_set_pixel(i,
                         clamp_u8(r),
                         clamp_u8(g),
                         clamp_u8(b));
    }
}



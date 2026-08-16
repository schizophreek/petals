#include "SDL/SDL.h"
#include "SDL/SDL_mixer.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "time.h"

#ifdef BUILD_A30
  #include "fcntl.h"
  #include "unistd.h"
  #include "sys/ioctl.h"
  #include "linux/fb.h"
  #define SCREEN_W 480
  #define SCREEN_H 640
#else
  #define SCREEN_W 640
  #define SCREEN_H 480
#endif

#define CANVAS_WIDTH 640
#define CANVAS_HEIGHT 480
#define DICE_SIZE 80

enum GameState {
    STATE_INTRO,
    STATE_GAME
};

static const unsigned char font5x7[][5] = {
    {0x00, 0x00, 0x00, 0x00, 0x00}, {0x00, 0x00, 0x5f, 0x00, 0x00},
    {0x00, 0x07, 0x00, 0x07, 0x00}, {0x14, 0x7f, 0x14, 0x7f, 0x14},
    {0x24, 0x2a, 0x7f, 0x2a, 0x12}, {0x23, 0x13, 0x08, 0x64, 0x62},
    {0x36, 0x49, 0x55, 0x22, 0x50}, {0x00, 0x05, 0x03, 0x00, 0x00},
    {0x00, 0x1c, 0x22, 0x41, 0x00}, {0x00, 0x41, 0x22, 0x1c, 0x00},
    {0x14, 0x08, 0x3e, 0x08, 0x14}, {0x08, 0x08, 0x3e, 0x08, 0x08},
    {0x00, 0x50, 0x30, 0x00, 0x00}, {0x08, 0x08, 0x08, 0x08, 0x08},
    {0x00, 0x60, 0x60, 0x00, 0x00}, {0x20, 0x10, 0x08, 0x04, 0x02},
    {0x3e, 0x51, 0x49, 0x45, 0x3e}, {0x00, 0x42, 0x7f, 0x40, 0x00},
    {0x42, 0x61, 0x51, 0x49, 0x46}, {0x21, 0x41, 0x45, 0x4b, 0x31},
    {0x18, 0x14, 0x12, 0x7f, 0x10}, {0x27, 0x45, 0x45, 0x45, 0x39},
    {0x3c, 0x4a, 0x49, 0x49, 0x30}, {0x01, 0x71, 0x09, 0x05, 0x03},
    {0x36, 0x49, 0x49, 0x49, 0x36}, {0x06, 0x49, 0x49, 0x29, 0x1e},
    {0x00, 0x36, 0x36, 0x00, 0x00}, {0x00, 0x56, 0x36, 0x00, 0x00},
    {0x08, 0x14, 0x22, 0x41, 0x00}, {0x14, 0x14, 0x14, 0x14, 0x14},
    {0x00, 0x41, 0x22, 0x14, 0x08}, {0x02, 0x01, 0x51, 0x09, 0x06},
    {0x32, 0x49, 0x79, 0x41, 0x3e}, {0x7e, 0x11, 0x11, 0x11, 0x7e},
    {0x7f, 0x49, 0x49, 0x49, 0x36}, {0x3e, 0x41, 0x41, 0x41, 0x22},
    {0x7f, 0x41, 0x41, 0x22, 0x1c}, {0x7f, 0x49, 0x49, 0x49, 0x41},
    {0x7f, 0x09, 0x09, 0x09, 0x01}, {0x3e, 0x41, 0x49, 0x49, 0x7a},
    {0x7f, 0x08, 0x08, 0x08, 0x7f}, {0x00, 0x41, 0x7f, 0x41, 0x00},
    {0x20, 0x40, 0x41, 0x3f, 0x01}, {0x7f, 0x08, 0x14, 0x22, 0x41},
    {0x7f, 0x40, 0x40, 0x40, 0x40}, {0x7f, 0x02, 0x0c, 0x02, 0x7f},
    {0x7f, 0x04, 0x08, 0x10, 0x7f}, {0x3e, 0x41, 0x41, 0x41, 0x3e},
    {0x7f, 0x09, 0x09, 0x09, 0x06}, {0x3e, 0x41, 0x51, 0x21, 0x5e},
    {0x7f, 0x09, 0x19, 0x29, 0x46}, {0x46, 0x49, 0x49, 0x49, 0x31},
    {0x01, 0x01, 0x7f, 0x01, 0x01}, {0x3f, 0x40, 0x40, 0x40, 0x3f},
    {0x1f, 0x20, 0x40, 0x20, 0x1f}, {0x3f, 0x40, 0x38, 0x40, 0x3f},
    {0x63, 0x14, 0x08, 0x14, 0x63}, {0x07, 0x08, 0x70, 0x08, 0x07},
    {0x61, 0x51, 0x49, 0x45, 0x43}, {0x00, 0x7f, 0x41, 0x41, 0x00},
    {0x02, 0x04, 0x08, 0x10, 0x20}, {0x00, 0x41, 0x41, 0x7f, 0x00}
};

static const unsigned char crown_art[10][12] = {
    {1,0,0,0,1,0,0,1,0,0,0,1}, {1,1,0,1,1,1,1,1,1,0,1,1},
    {1,1,0,1,1,1,1,1,1,0,1,1}, {1,1,1,1,1,1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1}, {1,3,1,2,1,3,1,2,1,3,1,1},
    {1,1,1,1,1,1,1,1,1,1,1,1}, {1,1,1,1,1,1,1,1,1,1,1,1},
    {0,1,1,1,1,1,1,1,1,1,1,0}, {0,1,1,1,1,1,1,1,1,1,1,0}
};

static const unsigned char rose_art[12][12] = {
    {0,0,0,1,1,1,1,0,0,0,0,0}, {0,0,1,2,2,1,1,1,0,0,0,0},
    {0,1,2,1,1,2,2,1,1,0,0,0}, {0,1,1,2,2,1,1,2,1,0,0,0},
    {0,0,1,1,1,1,2,1,0,0,0,0}, {0,0,0,2,1,2,1,0,0,0,0,0},
    {0,0,3,0,1,1,0,0,0,0,0,0}, {0,3,3,3,0,3,0,0,0,0,0,0},
    {0,0,3,3,3,3,0,0,0,0,0,0}, {0,0,0,3,0,3,3,0,0,0,0,0},
    {0,0,0,0,3,0,0,0,0,0,0,0}, {0,0,0,0,3,0,0,0,0,0,0,0}
};

enum GameState current_state = STATE_INTRO;
int has_started_game = 0;

int dice[5];
int display_dice[5];
int user_guess = 0;
int revealed = 0;
int streak = 0;

int is_animating = 0;
int anim_frames = 0;
int active_rendered_frames = 0;

int audio_enabled = 0;
Mix_Chunk *snd_roll = NULL;
Mix_Chunk *snd_wrong = NULL;
Mix_Chunk *snd_correct = NULL;
Mix_Chunk *snd_win = NULL;

void play_sound(Mix_Chunk *chunk) {
    if (audio_enabled && chunk) {
        Mix_PlayChannel(-1, chunk, 0);
    }
}

int calculate_petals(int d[]) {
    int total = 0;
    for (int i = 0; i < 5; i++) {
        if (d[i] == 3) total += 2;
        else if (d[i] == 5) total += 4;
    }
    return total;
}

void roll_dice() {
    for (int i = 0; i < 5; i++) {
        dice[i] = (rand() % 6) + 1;
    }
    user_guess = 0;
    revealed = 0;
    is_animating = 1;
    anim_frames = 20;
    play_sound(snd_roll);
}

void draw_rect(SDL_Surface *surface, int x, int y, int w, int h, Uint32 color) {
    SDL_Rect rect = {x, y, w, h};
    SDL_FillRect(surface, &rect, color);
}

void draw_char(SDL_Surface *surface, int x, int y, char c, Uint32 color, int scale) {
    if (c < 32 || c > 93) return;
    int idx = c - 32;
    for (int col = 0; col < 5; col++) {
        unsigned char b = font5x7[idx][col];
        for (int row = 0; row < 7; row++) {
            if (b & (1 << row)) {
                draw_rect(surface, x + (col * scale), y + (row * scale), scale, scale, color);
            }
        }
    }
}

void draw_string(SDL_Surface *surface, int x, int y, const char *str, Uint32 color, int scale) {
    int cur_x = x;
    while (*str) {
        draw_char(surface, cur_x, y, *str, color, scale);
        cur_x += (6 * scale);
        str++;
    }
}

void draw_string_centered(SDL_Surface *surface, int y, const char *str, Uint32 color, int scale) {
    int len = strlen(str);
    int str_w = len * 6 * scale;
    int x = (CANVAS_WIDTH - str_w) / 2;
    draw_string(surface, x, y, str, color, scale);
}

void draw_crown(SDL_Surface *surface, int x, int y, int scale) {
    Uint32 gold  = SDL_MapRGB(surface->format, 255, 215, 0);
    Uint32 red   = SDL_MapRGB(surface->format, 220, 20, 60);
    Uint32 white = SDL_MapRGB(surface->format, 255, 255, 255);
    for(int r = 0; r < 10; r++) {
        for(int c = 0; c < 12; c++) {
            unsigned char val = crown_art[r][c];
            if(val == 1) draw_rect(surface, x + c*scale, y + r*scale, scale, scale, gold);
            else if(val == 2) draw_rect(surface, x + c*scale, y + r*scale, scale, scale, red);
            else if(val == 3) draw_rect(surface, x + c*scale, y + r*scale, scale, scale, white);
        }
    }
}

void draw_rose(SDL_Surface *surface, int x, int y, int scale) {
    Uint32 red      = SDL_MapRGB(surface->format, 230, 30, 60);
    Uint32 dark_red = SDL_MapRGB(surface->format, 140, 10, 30);
    Uint32 green    = SDL_MapRGB(surface->format, 50, 180, 50);
    for(int r = 0; r < 12; r++) {
        for(int c = 0; c < 12; c++) {
            unsigned char val = rose_art[r][c];
            if(val == 1) draw_rect(surface, x + c*scale, y + r*scale, scale, scale, red);
            else if(val == 2) draw_rect(surface, x + c*scale, y + r*scale, scale, scale, dark_red);
            else if(val == 3) draw_rect(surface, x + c*scale, y + r*scale, scale, scale, green);
        }
    }
}

void draw_pip(SDL_Surface *surface, int cx, int cy, Uint32 color, int size) {
    draw_rect(surface, cx - size/2, cy - size/2, size, size, color);
}

void draw_die_custom(SDL_Surface *surface, int x, int y, int size, int pip_size, int value) {
    Uint32 white = SDL_MapRGB(surface->format, 240, 240, 240);
    Uint32 dark  = SDL_MapRGB(surface->format, 30, 30, 35);

    draw_rect(surface, x, y, size, size, white);

    int center_x = x + size / 2;
    int center_y = y + size / 2;
    int left     = x + (size / 4);
    int right    = x + size - (size / 4);
    int top      = y + (size / 4);
    int bottom   = y + size - (size / 4);

    if (value % 2 == 1) draw_pip(surface, center_x, center_y, dark, pip_size);
    if (value > 1) {
        draw_pip(surface, left, top, dark, pip_size);
        draw_pip(surface, right, bottom, dark, pip_size);
    }
    if (value > 3) {
        draw_pip(surface, right, top, dark, pip_size);
        draw_pip(surface, left, bottom, dark, pip_size);
    }
    if (value == 6) {
        draw_pip(surface, left, center_y, dark, pip_size);
        draw_pip(surface, right, center_y, dark, pip_size);
    }
}

void draw_die(SDL_Surface *surface, int x, int y, int value) {
    draw_die_custom(surface, x, y, DICE_SIZE, 16, value);
}

int main(int argc, char* argv[]) {
    setbuf(stdout, NULL);
    srand(time(NULL));

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0) return 1;

    if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 1024) == 0) {
        audio_enabled = 1;
        snd_roll    = Mix_LoadWAV("roll.wav");
        snd_wrong   = Mix_LoadWAV("wrong.wav");
        snd_correct = Mix_LoadWAV("correct.wav");
        snd_win     = Mix_LoadWAV("win.wav");
    }

    SDL_Surface* screen = SDL_SetVideoMode(SCREEN_W, SCREEN_H, 32, SDL_SWSURFACE);
    if (!screen) return 1;

#ifdef BUILD_A30
    int fb = open("/dev/fb0", O_RDWR);
    if (fb >= 0) {
        struct fb_var_screeninfo vinfo;
        if (ioctl(fb, FBIOGET_VSCREENINFO, &vinfo) == 0) {
            vinfo.xoffset = 0;
            vinfo.yoffset = 0;
            vinfo.activate = FB_ACTIVATE_NOW | FB_ACTIVATE_FORCE;
            ioctl(fb, FBIOGET_VSCREENINFO, &vinfo);
            ioctl(fb, FBIOPAN_DISPLAY, &vinfo);
        }
        close(fb);
    }

    SDL_Surface* canvas = SDL_CreateRGBSurface(SDL_SWSURFACE, CANVAS_WIDTH, CANVAS_HEIGHT, 32,
                                               0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
#else
    SDL_Surface* canvas = SDL_CreateRGBSurface(SDL_SWSURFACE, CANVAS_WIDTH, CANVAS_HEIGHT, 32,
                                               screen->format->Rmask, screen->format->Gmask,
                                               screen->format->Bmask, screen->format->Amask);
#endif
    if (!canvas) return 1;

    SDL_ShowCursor(SDL_DISABLE);
    SDL_JoystickOpen(0);

    SDL_Event flush_event;
    while (SDL_PollEvent(&flush_event));

    roll_dice();

    int running = 1;
    SDL_Event event;

    Uint32 text_white   = SDL_MapRGB(canvas->format, 240, 240, 240);
    Uint32 text_yellow  = SDL_MapRGB(canvas->format, 255, 215, 0);
    Uint32 text_green   = SDL_MapRGB(canvas->format, 50, 205, 50);
    Uint32 text_red     = SDL_MapRGB(canvas->format, 220, 20, 60);

    char buffer[64];

    while (SDL_PollEvent(&flush_event));

    while (running) {
        active_rendered_frames++;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_RETURN:
                        if (current_state == STATE_INTRO) {
                            current_state = STATE_GAME;
                            has_started_game = 1;
                        } else if (current_state == STATE_GAME) {
                            current_state = STATE_INTRO;
                        }
                        break;
                    case SDLK_UP:
                    case SDLK_RIGHT:
                        if (current_state == STATE_GAME && !revealed && !is_animating) {
                            user_guess += 2;
                            if (user_guess > 20) user_guess = 20;
                        }
                        break;
                    case SDLK_DOWN:
                    case SDLK_LEFT:
                        if (current_state == STATE_GAME && !revealed && !is_animating) {
                            user_guess -= 2;
                            if (user_guess < 0) user_guess = 0;
                        }
                        break;
                    case SDLK_SPACE:
                    case SDLK_LCTRL:
                        if (current_state == STATE_INTRO) {
                            current_state = STATE_GAME;
                            has_started_game = 1;
                        } else if (current_state == STATE_GAME && !is_animating) {
                            if (!revealed) {
                                revealed = 1;
                                if (user_guess == calculate_petals(dice)) {
                                    streak++;
                                    if (streak >= 6) {
                                        play_sound(snd_win);
                                    } else {
                                        play_sound(snd_correct);
                                    }
                                } else {
                                    streak = 0;
                                    play_sound(snd_wrong);
                                }
                            } else {
                                roll_dice();
                            }
                        }
                        break;
                    case SDLK_LALT:
                        if (current_state == STATE_GAME && !is_animating) roll_dice();
                        break;
                    case SDLK_ESCAPE:
                        if (active_rendered_frames > 60) {
                            running = 0;
                        }
                        break;
                    default:
                        break;
                }
            } else if (event.type == SDL_QUIT) {
                if (active_rendered_frames > 60) {
                    running = 0;
                }
            }
        }

        if (is_animating) {
            anim_frames--;
            for (int i = 0; i < 5; i++) {
                display_dice[i] = (rand() % 6) + 1;
            }
            if (anim_frames <= 0) {
                is_animating = 0;
                for (int i = 0; i < 5; i++) {
                    display_dice[i] = dice[i];
                }
            }
        } else {
            for (int i = 0; i < 5; i++) {
                display_dice[i] = dice[i];
            }
        }

        int step = (streak > 6) ? 6 : streak;
        Uint8 r_val = 12 + (108 * step) / 6;
        Uint8 g_val = 12 + (3 * step) / 6;
        Uint8 b_val = 16 + (29 * step) / 6;
        Uint32 bg_color = SDL_MapRGB(canvas->format, r_val, g_val, b_val);

        SDL_FillRect(canvas, NULL, bg_color);

        if (current_state == STATE_INTRO) {
            draw_string_centered(canvas, 30, "PETALS AROUND THE ROSE", text_yellow, 3);
            draw_die_custom(canvas, 270, 85, 100, 20, 5);

            draw_string_centered(canvas, 210, "RULES OF THE GAME:", text_yellow, 2);
            draw_string_centered(canvas, 255, "1. THE NAME OF THE GAME IS IMPORTANT.", text_white, 2);
            draw_string_centered(canvas, 285, "2. THE SCORE IS ALWAYS ZERO OR AN EVEN NUMBER.", text_white, 2);
            draw_string_centered(canvas, 315, "3. IF YOU SOLVE THE SECRET, KEEP IT SACRED!", text_white, 2);

            if (!has_started_game) {
                draw_string_centered(canvas, 395, "PRESS START TO BEGIN", text_yellow, 2);
            } else {
                draw_string_centered(canvas, 395, "PRESS START TO CONTINUE", text_yellow, 2);
                snprintf(buffer, sizeof(buffer), "CURRENT STREAK: %d", streak);
                draw_string_centered(canvas, 435, buffer, text_white, 2);
            }

        } else {
            draw_string_centered(canvas, 30, "PETALS AROUND THE ROSE", text_yellow, 3);

            if (streak >= 6) {
                draw_crown(canvas, 30, 33, 2);
                draw_rose(canvas, 580, 31, 2);
                draw_string_centered(canvas, 75, "WELCOME TO THE FRATERNITY,", text_yellow, 2);
                draw_string_centered(canvas, 102, "FELLOW POTENTATE OF THE ROSE!", text_yellow, 2);
            }

            int start_x = 40;
            int spacing = 115;
            for (int i = 0; i < 5; i++) {
                draw_die(canvas, start_x + (i * spacing), 140, display_dice[i]);
            }

            if (is_animating) {
                draw_string_centered(canvas, 280, "ROLLING...", text_yellow, 3);
            } else if (!revealed) {
                snprintf(buffer, sizeof(buffer), "YOUR GUESS: %d", user_guess);
                draw_string_centered(canvas, 280, buffer, text_white, 3);
                draw_string_centered(canvas, 350, "DPAD: ADJUST   A/START: SUBMIT", text_yellow, 2);
            } else {
                int actual = calculate_petals(dice);
                if (user_guess == actual) {
                    snprintf(buffer, sizeof(buffer), "CORRECT! PETALS = %d", actual);
                    draw_string_centered(canvas, 280, buffer, text_green, 3);
                } else {
                    snprintf(buffer, sizeof(buffer), "WRONG! PETALS = %d", actual);
                    draw_string_centered(canvas, 280, buffer, text_red, 3);
                }
                draw_string_centered(canvas, 350, "PRESS A TO ROLL AGAIN", text_yellow, 2);
            }

            snprintf(buffer, sizeof(buffer), "STREAK: %d", streak);
            draw_string_centered(canvas, 410, buffer, text_white, 2);
        }

        Uint32* src = (Uint32*)canvas->pixels;
        Uint32* dst = (Uint32*)screen->pixels;

#ifdef BUILD_A30
        for (int y = 0; y < CANVAS_HEIGHT; y++) {
            for (int x = 0; x < CANVAS_WIDTH; x++) {
                dst[(CANVAS_WIDTH - 1 - x) * 480 + y] = src[y * CANVAS_WIDTH + x];
            }
        }
#else
        for (int y = 0; y < CANVAS_HEIGHT; y++) {
            for (int x = 0; x < CANVAS_WIDTH; x++) {
                dst[(CANVAS_HEIGHT - 1 - y) * CANVAS_WIDTH + (CANVAS_WIDTH - 1 - x)] = src[y * CANVAS_WIDTH + x];
            }
        }
#endif

        SDL_UpdateRect(screen, 0, 0, 0, 0);
        SDL_Flip(screen);
        SDL_Delay(16);
    }

    if (audio_enabled) {
        if (snd_roll) Mix_FreeChunk(snd_roll);
        if (snd_wrong) Mix_FreeChunk(snd_wrong);
        if (snd_correct) Mix_FreeChunk(snd_correct);
        if (snd_win) Mix_FreeChunk(snd_win);
        Mix_CloseAudio();
    }

    SDL_FreeSurface(canvas);
    SDL_Quit();
    return 0;
}

// ui.c
// this file is part of piano-term

#include "ui.h"
#include "audio.h"
#include "config.h"
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

typedef struct {
  char keyChar;
  const char *note;
  int visualHeight;
  bool isBlack;
  int whiteKeyIndex;

  bool isPressed;
  long long lastSeenTimeMs;
} kmap;

#define VISUALIZER_MAX_HEIGHT 15
#define PIANO_HEIGHT 12
#define RELEASE_TIMEOUT_MS 100 // time without input for a key to be considered released

static kmap keys[] = {
    // oct. 3 (lower)
    {'z', "C3", 0, false, 0, false, 0},
    {'s', "Db3", 0, true, 0, false, 0},
    {'x', "D3", 0, false, 1, false, 0},
    {'d', "Eb3", 0, true, 1, false, 0},
    {'c', "E3", 0, false, 2, false, 0},
    {'v', "F3", 0, false, 3, false, 0},
    {'g', "Gb3", 0, true, 3, false, 0},
    {'b', "G3", 0, false, 4, false, 0},
    {'h', "Ab3", 0, true, 4, false, 0},
    {'n', "A3", 0, false, 5, false, 0},
    {'j', "Bb3", 0, true, 5, false, 0},
    {'m', "B3", 0, false, 6, false, 0},

    // oct. 4 (upper)
    {'q', "C4", 0, false, 7, false, 0},
    {'2', "Db4", 0, true, 7, false, 0},
    {'w', "D4", 0, false, 8, false, 0},
    {'3', "Eb4", 0, true, 8, false, 0},
    {'e', "E4", 0, false, 9, false, 0},
    {'r', "F4", 0, false, 10, false, 0},
    {'5', "Gb4", 0, true, 10, false, 0},
    {'t', "G4", 0, false, 11, false, 0},
    {'6', "Ab4", 0, true, 11, false, 0},
    {'y', "A4", 0, false, 12, false, 0},
    {'7', "Bb4", 0, true, 12, false, 0},
    {'u', "B4", 0, false, 13, false, 0},

    // high C5
    {'i', "C5", 0, false, 14, false, 0},
};

static int num_keys = sizeof(keys) / sizeof(keys[0]);
static int num_white_keys = 15; // 0 to 14

static long long current_time_ms(void) {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (long long)(tv.tv_sec) * 1000 + (tv.tv_usec / 1000);
}

void ui_init(void) {
  // load config
  tpianoConfig cfg;
  load_config(&cfg);

  // map config to keys
  // low white (7K) indices:
  int lw_indices[] = {0, 2, 4, 5, 7, 9, 11};
  int lb_indices[] = {1, 3, 6, 8, 10};
  int uw_indices[] = {12, 14, 16, 17, 19, 21, 23}; // -> indices in original array
  int ub_indices[] = {13, 15, 18, 20, 22};

  for (int i = 0; i < 7; i++)
    if (cfg.low_white[i])
      keys[lw_indices[i]].keyChar = cfg.low_white[i];
  for (int i = 0; i < 5; i++)
    if (cfg.low_black[i])
      keys[lb_indices[i]].keyChar = cfg.low_black[i];
  for (int i = 0; i < 7; i++)
    if (cfg.top_white[i])
      keys[uw_indices[i]].keyChar = cfg.top_white[i];
  for (int i = 0; i < 5; i++)
    if (cfg.top_black[i])
      keys[ub_indices[i]].keyChar = cfg.top_black[i];

  initscr();
  cbreak();
  noecho();
  nodelay(stdscr, TRUE);
  curs_set(0);
  keypad(stdscr, TRUE);
  start_color();
  init_pair(1, COLOR_BLACK, COLOR_WHITE);
  init_pair(2, COLOR_WHITE, COLOR_BLACK);
  init_pair(3, COLOR_CYAN, COLOR_BLACK);
  init_pair(4, COLOR_WHITE, COLOR_BLACK);
  init_pair(5, COLOR_WHITE, COLOR_CYAN);
  init_pair(6, COLOR_BLACK, COLOR_CYAN);
}

void ui_shutdown(void) { endwin(); }

static void draw_visual_keyboard(void) {
  int w = COLS / num_white_keys;
  if (w < 3)
    w = 3;
  int total_width = w * num_white_keys;
  int start_x = (COLS - total_width) / 2;
  if (start_x < 0)
    start_x = 0;
  int start_y = LINES - PIANO_HEIGHT;

  // draw 'em keys
  for (int i = 0; i < num_keys; i++) {
    // white keys first...
    if (!keys[i].isBlack) {
      int color_pair = (keys[i].isPressed || keys[i].visualHeight > 12) ? 5 : 1;

      attron(COLOR_PAIR(color_pair));
      int x = start_x + (keys[i].whiteKeyIndex * w);

      for (int h = 0; h < PIANO_HEIGHT; h++) {
        mvhline(start_y + h, x, ' ', w);
        mvaddch(start_y + h, x, '|');
        mvaddch(start_y + h, x + w - 1, '|');
      }
      mvhline(start_y + PIANO_HEIGHT - 1, x, '_', w);
      mvprintw(start_y + PIANO_HEIGHT - 2, x + (w / 2), "%c", keys[i].keyChar);
      attroff(COLOR_PAIR(color_pair));
    }
  }

  // then black keys!
  int bw = w / 2 + (w % 2);
  if (bw < 2)
    bw = 2;
  int bh = (int)(PIANO_HEIGHT * 0.6);

  for (int i = 0; i < num_keys; i++) {
    if (keys[i].isBlack) {
      int color_pair = (keys[i].isPressed || keys[i].visualHeight > 12) ? 6 : 2;
      attron(COLOR_PAIR(color_pair));

      int center_x = start_x + ((keys[i].whiteKeyIndex + 1) * w);
      int x = center_x - (bw / 2);

      for (int h = 0; h < bh; h++) {
        mvhline(start_y + h, x, ' ', bw);
      }
      mvprintw(start_y + bh - 2, x + (bw / 2), "%c", keys[i].keyChar);
      attroff(COLOR_PAIR(color_pair));
    }
  }
}

static void do_onhit_draw(void) {
  int w = COLS / num_white_keys;
  if (w < 3)
    w = 3;
  int total_width = w * num_white_keys;
  int start_x = (COLS - total_width) / 2;
  if (start_x < 0)
    start_x = 0;

  int base_y = LINES - PIANO_HEIGHT - 1;

  attron(COLOR_PAIR(3));
  for (int i = 0; i < num_keys; i++) {
    if (keys[i].visualHeight > 0) {
      int height = keys[i].visualHeight;
      int x_center;

      if (!keys[i].isBlack) {
        x_center = start_x + (keys[i].whiteKeyIndex * w) + (w / 2);
      } else {
        x_center = start_x + ((keys[i].whiteKeyIndex + 1) * w);
      }

      for (int h = 0; h < height; h++) {
        if (base_y - h >= 0 && base_y - h < LINES) {
          mvaddch(base_y - h, x_center, '|');
          if (w > 4) {
            mvaddch(base_y - h, x_center - 1, '|');
            mvaddch(base_y - h, x_center + 1, '|');
          }
        }
      }
      if (base_y - height >= 0)
        mvaddch(base_y - height, x_center, 'o');
    }
  }
  attroff(COLOR_PAIR(3));
}

static void upd_np(void) {
  long long now = current_time_ms();
  for (int i = 0; i < num_keys; i++) {
    if (keys[i].isPressed) {
      if (now - keys[i].lastSeenTimeMs > RELEASE_TIMEOUT_MS) {
        keys[i].isPressed = false;
      }
    }

    if (keys[i].visualHeight > 0) {
      keys[i].visualHeight--;
    }
  }
}

void ui_run(void) {
  int ch;
  bool running = true;

  while (running) {
    long long now = current_time_ms();

    // read all inputs from buffer
    // handles chords / fast input
    while ((ch = getch()) != ERR) {
      if (ch == 27) { // ESC
        running = false;
        break;
      } else {
        for (int i = 0; i < num_keys; i++) {
          if (ch == keys[i].keyChar) {
            keys[i].lastSeenTimeMs = now;
            if (!keys[i].isPressed) {
              audio_play_note(keys[i].note);
              keys[i].visualHeight = VISUALIZER_MAX_HEIGHT;
              keys[i].isPressed = true;
            }
          }
        }
      }
    }
    if (!running)
      break;

    erase();
    // id/info text
    mvprintw(0, 0,
             "piano-term b. "
             "12/07/2025\n~/.config/piano/keybinds.conf\nEsc/Ctrl+C to quit");

    upd_np();
    do_onhit_draw();
    draw_visual_keyboard();

    refresh();
    napms(16);
  }
}

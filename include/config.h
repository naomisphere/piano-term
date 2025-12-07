// config.h
// piano-term

#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>

typedef struct {
  char low_white[8]; // 7k + null
  char low_black[6]; // 5k + null
  char top_white[8]; // 7k + null
  char top_black[6]; // 5k + null
} tpianoConfig;

// config file: ~/.config/piano/keybinds.conf
// check if it exists and handle loading
void load_config(tpianoConfig *config);

#endif

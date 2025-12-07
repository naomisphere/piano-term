// main.c
// this file is part of piano-term
// did you know that?

#include "audio.h"
#include "ui.h"
#include <stdio.h>

int main(void) {
  if (!audio_init()) {
    fprintf(stderr, "couldn't begin audio...\n");
    return 1;
  }

  ui_init();
  ui_run();
  ui_shutdown();

  audio_shutdown();
  return 0;
}

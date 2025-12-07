// audio.c
// this file is part of piano-term

#include "audio.h"
#include "miniaudio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SOUNDS 128

typedef struct {
  char note[8];
  ma_sound sound;
  bool loaded;
} NoteSound;

static ma_engine engine;
static NoteSound sounds[MAX_SOUNDS];
static int sound_count = 0;

// find sound by note name
static NoteSound *get_sound(const char *note) {
  for (int i = 0; i < sound_count; i++) {
    if (strcmp(sounds[i].note, note) == 0) {
      return &sounds[i];
    }
  }
  return NULL;
}

// load note
static void load_note(const char *note) {
  if (sound_count >= MAX_SOUNDS)
    return;
  if (get_sound(note))
    return;

  char path[256];
  snprintf(path, sizeof(path), "assets/%s.mp3", note);

  NoteSound *ns = &sounds[sound_count];

  // ** MA_SOUND_FLAG_DECODE ensures full load into memory
  ma_result result = ma_sound_init_from_file(
      &engine, path, MA_SOUND_FLAG_DECODE, NULL, NULL, &ns->sound);

  if (result != MA_SUCCESS) {
    return;
  }

  strncpy(ns->note, note, sizeof(ns->note) - 1);
  ns->loaded = true;
  sound_count++;
}

bool audio_init(void) {
  ma_result result = ma_engine_init(NULL, &engine);
  if (result != MA_SUCCESS) {
    fprintf(stderr, "Failed to initialize audio engine.\n");
    return false;
  }

  // load common octaves... blah
  // notes: C, Db, D, Eb, E, F, Gb, G, Ab, A, Bb, B
  const char *names[] = {"C",  "Db", "D",  "Eb", "E",  "F",
                         "Gb", "G",  "Ab", "A",  "Bb", "B"};
  int octaves[] = {3, 4, 5};

  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 12; j++) {
      char noteName[8];
      snprintf(noteName, sizeof(noteName), "%s%d", names[j], octaves[i]);
      load_note(noteName);
    }
  }
  // load C6 too
  load_note("C6");

  return true;
}

void audio_shutdown(void) {
  for (int i = 0; i < sound_count; i++) {
    if (sounds[i].loaded) {
      ma_sound_uninit(&sounds[i].sound);
    }
  }
  ma_engine_uninit(&engine);
}

void audio_play_note(const char *note) {
  NoteSound *ns = get_sound(note);
  if (ns && ns->loaded) {
    // restart sound if already playing, or start it
    ma_sound_seek_to_pcm_frame(&ns->sound, 0);
    ma_sound_start(&ns->sound);
  }
}

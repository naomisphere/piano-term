// audio.h
// this file is part of piano-term
// but you knew that

#ifndef AUDIO_H
#define AUDIO_H

#include <stdbool.h>

// begin audio engine and load sounds
bool audio_init(void);
// clean up audio
void audio_shutdown(void);
// play note p/ file, e.g. "B5.mp3", "Db6.mp3"
void audio_play_note(const char *note);

#endif

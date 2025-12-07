// config.c
// this file is part of piano-term
// you knew that.

#include "config.h"
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static void get_config_path(char *buffer, size_t size) {
  const char *home = getenv("HOME");
  if (!home) {
    struct passwd *pw = getpwuid(getuid());
    if (pw)
      home = pw->pw_dir;
  }
  if (!home)
    return;

  snprintf(buffer, size, "%s/.config/piano/keybinds.conf", home);
}

static void make_defconfig(const char *path) {
  char dir[1024];
  size_t len = strlen(path);
  char *last_slash = strrchr(path, '/');
  if (last_slash) {
    size_t dirlen = last_slash - path;
    strncpy(dir, path, dirlen);
    dir[dirlen] = '\0';
    char cmd[1100];
    snprintf(cmd, sizeof(cmd), "mkdir -p \"%s\"", dir);
    system(cmd);
  }

  FILE *f = fopen(path, "w");
  if (f) {
    fprintf(f, "# piano-term keybinds\n");
    fprintf(f, "low z,x,c,v,b,n,m\n");
    fprintf(f, "lob s,d,g,h,j\n");
    fprintf(f, "uow q,w,e,r,t,y,u\n");
    fprintf(f, "uob 2,3,5,6,7\n");
    fclose(f);
  }
}

static void parse_conf(char *dest, const char *line, int max_count) {
  while (*line == ' ' || *line == '\t')
    line++;

  int count = 0;
  while (*line && count < max_count) {
    if (*line != ',') {
      dest[count++] = *line;
      while (*line && *line != ',')
        line++;
    }
    if (*line == ',')
      line++;
  }
  dest[count] = '\0';
}

void load_config(tpianoConfig *config) {
  // Defaults
  strcpy(config->low_white, "zxcvbnm");
  strcpy(config->low_black, "sdghj");
  strcpy(config->top_white, "qwertyu");
  strcpy(config->top_black, "23567");

  char path[1024];
  get_config_path(path, sizeof(path));

  FILE *f = fopen(path, "r");
  if (!f) {
    make_defconfig(path);
    return;
  }

  char line[256];
  while (fgets(line, sizeof(line), f)) {
    line[strcspn(line, "\n")] = 0;

    if (strncmp(line, "low ", 4) == 0)
      parse_conf(config->low_white, line + 4, 7);
    else if (strncmp(line, "lob ", 4) == 0)
      parse_conf(config->low_black, line + 4, 5);
    else if (strncmp(line, "uow ", 4) == 0)
      parse_conf(config->top_white, line + 4, 7);
    else if (strncmp(line, "uob ", 4) == 0)
      parse_conf(config->top_black, line + 4, 5);
  }
  fclose(f);
}

CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2 -Iinclude
LDFLAGS = 

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	LDFLAGS += -lncurses -framework AudioToolbox -framework CoreAudio -framework CoreFoundation
else
	LDFLAGS += -lncurses -ldl -lpthread -lm
endif

SRC = src/main.c src/ui.c src/audio.c src/miniaudio_impl.c src/config.c
OBJ = $(SRC:.c=.o)
TARGET = piano-term

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(OBJ) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

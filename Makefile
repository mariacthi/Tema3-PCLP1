
# compiler setup
CC=gcc
CFLAGS=-Wall -Wextra -lm

# define targets
TARGETS=image_editor

build: $(TARGETS)

image_editor: image_editor.c
	$(CC) image_editor.c -o image_editor $(CFLAGS)

clean:
	rm -f $(TARGETS)

.PHONY: pack clean

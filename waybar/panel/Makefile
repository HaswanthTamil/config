CC = g++
CFLAGS = -O2 -Wall `pkg-config --cflags gtk+-3.0 gtk-layer-shell-0`
LIBS = `pkg-config --libs gtk+-3.0 gtk-layer-shell-0` -lm -lpthread

SRCS = main.c panel_style.c panel_left.c panel_center.c panel_right.c
TARGET = zone-panel

all: $(TARGET)

$(TARGET): $(SRCS) panel_common.h
	$(CC) $(CFLAGS) $(SRCS) $(LIBS) -o $(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all clean

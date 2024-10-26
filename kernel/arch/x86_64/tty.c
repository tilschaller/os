#include <kernel/memory.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <kernel/tty.h>

#include "vga.h"

static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;
static const uint16_t *VGA_MEMORY = (uint16_t *)0xffffff80000B8000;

static size_t terminal_row;
static size_t terminal_column;
static uint8_t terminal_color;
static uint16_t *terminal_buffer;

void terminal_initialize(void) {
  terminal_row = 0;
  terminal_column = 0;
  terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
  terminal_buffer = VGA_MEMORY;
  for (size_t y = 0; y < VGA_HEIGHT; y++) {
    for (size_t x = 0; x < VGA_WIDTH; x++) {
      const size_t index = y * VGA_WIDTH + x;
      terminal_buffer[index] = vga_entry(' ', terminal_color);
    }
  }
}

void terminal_setcolor(uint8_t color) { terminal_color = color; }

void terminal_write(const char *data, size_t size) {
  for (size_t i = 0; i < size; i++)
    terminal_putchar(data[i]);
}

void terminal_writestring(const char *data) {
  terminal_write(data, strlen(data));
}

void terminal_putentryat(char c, uint8_t color, size_t x, size_t y) {
  const size_t index = y * VGA_WIDTH + x;
  terminal_buffer[index] = vga_entry(c, color);
}

void terminal_putchar(char c) {
  if (c == '\n') {
    if (terminal_row + 1 == VGA_HEIGHT) {
      terminal_scroll();
      terminal_column = 0;
    } else {
      terminal_row++;
      terminal_column = 0;
    }
  } else {
    terminal_putentryat(c, terminal_color, terminal_column, terminal_row);

    if (++terminal_column == VGA_WIDTH) {
      terminal_column = 0;
      if (++terminal_row == VGA_HEIGHT) {
        terminal_scroll();
      }
    }
  }
}

void terminal_scroll() {
  // Scroll the buffer up by one row.
  const size_t buffer_size = (VGA_HEIGHT - 1) * VGA_WIDTH;

  memmove(terminal_buffer, terminal_buffer + VGA_WIDTH * 2, buffer_size * 2);

  // Clear the last row.
  const size_t last_row_offset = VGA_HEIGHT - 1;
  for (size_t x = 0; x < VGA_WIDTH; x++) {
    terminal_putentryat(' ', terminal_color, x, last_row_offset);
  }
}

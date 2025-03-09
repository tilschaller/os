#include <stdlib.h>
#include <stdio.h>

void abort() {
  asm("cli");
  printk("\nAn unrecoverable kernel error occured\nPlease reboot your computer");
  for (;;) {
    asm("hlt");
  }
}

void exit() {
  printk("\nThe process exited normally\nHalting until (timer) interrupt");
  for (;;) {
    asm("hlt");
  }
}

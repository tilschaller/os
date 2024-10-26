#!/bin/sh

./qemu.sh gdb &
gdb kernel/myos.kernel -ex 'target remote localhost:1234'
killall qemu-system-x86_64

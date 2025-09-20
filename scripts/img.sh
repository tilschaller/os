#!/bin/sh

set -e
. ./scripts/build.sh

touch nightc.img
dd if=sysroot/boot/bootloader.bin of=nightc.img bs=512 count=10 conv=notrunc
dd if=sysroot/boot/nightc.kernel of=nightc.img bs=512 seek=10 conv=notrunc
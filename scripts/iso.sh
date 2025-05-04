#!/bin/sh
set -e
. ./scripts/build.sh

mkdir -p isodir
mkdir -p isodir/boot
mkdir -p isodir/boot/grub

cp sysroot/boot/nightc.kernel isodir/boot/nightc.kernel
cat > isodir/boot/grub/grub.cfg << EOF
menuentry "nightc" {
	multiboot /boot/nightc.kernel
}
EOF
grub-mkrescue -o nightc.iso isodir

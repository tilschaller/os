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

# TODO: build without uefi stub only on i386 or higher architectures
# for now it builds the iso only for i386
# a simple check for arch should be enough

grub-mkrescue -o nightc.iso isodir \
  --modules="biosdisk part_msdos ext2 multiboot normal configfile" \
  --directory=/usr/lib/grub/i386-pc/

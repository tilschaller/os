#!/bin/sh
set -e
. ./iso.sh

if echo "$1" | grep -Eq 'gdb'; then
  qemu-system-$(./target-triplet-to-arch.sh $HOST) -cdrom myos.iso -s -S -m 512M
else
  qemu-system-$(./target-triplet-to-arch.sh $HOST) -cdrom myos.iso -m 512M
fi

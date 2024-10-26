#!/bin/sh
set -e
. ./iso.sh

if echo "$1" | grep -Eq 'gdb'; then
  qemu-system-$(./target-triplet-to-arch.sh $HOST) -cdrom myos.iso -s -S
else
  qemu-system-$(./target-triplet-to-arch.sh $HOST) -cdrom myos.iso
fi

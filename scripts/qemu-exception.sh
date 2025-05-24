#!/bin/sh
set -e
. ./scripts/iso.sh

qemu-system-$(./scripts/target-triplet-to-arch.sh $HOST) -cdrom nightc.iso -m 4G -d int -no-reboot

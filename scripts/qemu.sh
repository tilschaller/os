#!/bin/sh
set -e
. ./scripts/iso.sh

qemu-system-$(./scripts/target-triplet-to-arch.sh $HOST) -drive file=nightc.img,format=raw,media=disk -m 4G -debugcon stdio -M pc -cpu qemu64,+apic -smp 2

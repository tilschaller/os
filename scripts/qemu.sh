#!/bin/sh
set -e
. ./scripts/iso.sh

# TODO: add a script that return the qemu options and copy them in here and qemu-exception
qemu-system-$(./scripts/target-triplet-to-arch.sh $HOST) -cdrom nightc.iso -m 4G -debugcon stdio -M pc -cpu qemu64,+apic -smp 2

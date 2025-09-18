#!/bin/sh
set -e
. ./scripts/config.sh

for PROJECT in $PROJECTS; do
  (cd $PROJECT && $MAKE clean)
done

rm -rf sysroot
rm -rf nightc.img
rm -rf compile_commands.json
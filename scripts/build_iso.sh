#!/bin/bash

ISOFILE="$1"
ISODIR="$2"
CFGFILE="$3"
KERNEL="$4"

mkdir -p "$ISODIR/isofiles/boot/grub"
cp "$CFGFILE" "$ISODIR/isofiles/boot/grub/"
cp "$KERNEL" "$ISODIR/isofiles/boot/kernel.bin"
grub-mkrescue -o "$ISOFILE" "$ISODIR/isofiles"

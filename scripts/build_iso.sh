#!/bin/bash

ISOFILE="$1"
ISODIR="$2"
CFGFILE="$3"
KERNEL="$4"

mkdir -p "$ISODIR/files/boot/grub"
cp "$CFGFILE" "$ISODIR/files/boot/grub/"
cp "$KERNEL" "$ISODIR/files/boot/kernel.bin"
grub-mkrescue -o "$ISOFILE" "$ISODIR/files"
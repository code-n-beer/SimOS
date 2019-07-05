#!/bin/bash

QEMU_PATH=$1
ISO_PATH=$2

shift 2

/mnt/c/Windows/System32/cmd.exe '/c' copy '/y' $(wslpath -aw $ISO_PATH) '%TEMP%\simo.iso'
/mnt/c/Windows/System32/cmd.exe '/c' "$(wslpath -aw $QEMU_PATH) -cdrom %TEMP%/simo.iso $@"

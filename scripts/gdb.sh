#!/bin/bash

gdb -ex "file build/kernel.elf" -ex "target remote ${WINDOWS_IP:-localhost}:1234"

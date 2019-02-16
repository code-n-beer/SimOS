```
$ aursync x86_64-elf-binutils
$ sudo pacman -Sy x86_64-elf-binutils meson ninja
$ cd gcc/
$ makepkg -si
$ cd ..
$ meson setup build --cross-file crossfile.txt
$ ninja -C build run
```

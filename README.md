SimOS is an experiment in operating system design focusing on comedic value rather than
functionality or implementation correctness.

# Environment setup
Install dependencies:
```
$ sudo pacman -S mtools grub xorriso base-devel ninja meson
```

You can use either GCC (>= 9.1) or Clang (>= 9.0).

## GCC
Install `x86_64-elf-gcc` and `x86_64-elf-binutils` with the included scripts:
```
$ cd tools/
$ makepkg -sip PKGBUILD.binutils
$ makepkg -sip PKGBUILD.gcc
```

## Clang
Install `clang` and `lld`:
```
$ sudo pacman -S clang lld
```
Because Meson 0.52 does intelligent linker autodetection and thus ignores the linker
defined in the crossfile, you'll need at least `x86_64-elf-binutils` (and possibly GCC as well)
anyway. This should be fixed in Meson 0.53.

# Building
Set up the build tree:
```
$ meson setup build --cross-file crossfile-$COMPILER
```

Build it with ninja:
```
$ ninja -C build <target>
```

Useful build targets:
* `clean`: Cleans the build tree
* `kernel.elf`: Builds just the kernel
* `simo.iso`: Builds a bootable ISO image
* `run`: Builds the ISO and boots it in QEMU
* `run-debug`: Same as `run`, but also enables the GDB server and waits for the debugger to attach

# Building in Docker
(Not sure if these instructions even work anymore...)

Set up the build environment:
```
$ docker build -t simobuild .
$ ./scripts/run_build_env.sh   # run from repository root folder
```

Building (in the container):
```
$ cd /simos
$ meson setup build/ --cross-file crossfile.txt
$ ninja -C build/
```

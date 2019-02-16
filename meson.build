project('SimOS',
  'cpp', 'c',
  version: '0.1',
  default_options: [
    'warning_level=2', 
    'cpp_std=gnu++2a',
    'c_std=gnu11'
  ])

qemu = find_program('qemu-system-x86_64')
build_iso = find_program('scripts/build_iso.sh')

incdir = include_directories('include')

sources = [
  'src/crti.S',
  'src/utils.cpp',
  'src/kmain.cpp',
  'src/console.cpp',
  'src/interrupt.cpp',
  'src/gdt.cpp',
  'src/printf.c',
  'src/paging.cpp',
  'src/boot/boot.S',
  'src/boot/long_mode_init.S',
  'src/boot/multiboot.S',
  'src/crtn.S'
]

kernel = executable('kernel.elf',
  sources,
  include_directories: incdir)

iso = custom_target('iso',
  output: 'simo.iso',
  input: ['src/boot/grub.cfg', kernel],
  command: [build_iso, '@OUTPUT@', '@OUTDIR@', '@INPUT0@', '@INPUT1@'])

run_target('run',
  depends: iso,
  command: [qemu, '-m', '1G', '-cdrom', iso, '-cpu', 'Skylake-Client',
            '-no-reboot', '-no-shutdown', '-d', 'int', '-monitor', 'stdio'])
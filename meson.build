project('SimOS', 'cpp', 'c',
  version : '0.1',
  default_options : [
    'warning_level=2', 
    'cpp_std=gnu++2a',
    'c_std=gnu11'
  ])

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

executable('kernel.elf', sources, include_directories: incdir)

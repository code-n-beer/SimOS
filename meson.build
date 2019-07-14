project('SimOS',
  'cpp', 'c',
  version: '0.1',
  default_options: [
    'warning_level=2', 
    'cpp_std=gnu++2a',
    'c_std=gnu11'
  ])

qemu = find_program('qemu-system-x86_64', required: false)
qemu_win = find_program('/mnt/c/dev/qemu/qemu-system-x86_64.exe', required: false)

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
  'src/framemap.cpp',
  'src/boot/boot.S',
  'src/boot/long_mode_init.S',
  'src/boot/multiboot.S',
  'src/boot/bootsplash.cpp',
  'src/crtn.S'
]

test_sources = [
  'test/src/flags.test.cpp',
  'test/src/typetraits.test.cpp',
  'test/src/catch.hpp',
  'test/src/tuple.test.cpp',
  'test/src/main.cpp',
  'test/src/lambda.test.cpp',
]

test_exe = executable('tests',
  test_sources, 
  include_directories : 'include', 
  native : true,
  cpp_args : ['-std=gnu++2a', '-fconcepts'])
test('STL tests', test_exe)

kernel = executable('kernel.elf',
  sources,
  include_directories: incdir)

iso = custom_target('iso',
  output: 'simo.iso',
  input: ['src/boot/grub.cfg', kernel],
  command: [build_iso, '@OUTPUT@', '@OUTDIR@', '@INPUT0@', '@INPUT1@'])

qemu_common_args = [
  '-m', '1G', '-no-reboot', '-no-shutdown', '-monitor', 'stdio'
]

qemu_cmd = []

if qemu.found()
  qemu_cmd += [
    qemu, qemu_common_args, '-enable-kvm', '-cpu', 'host', '-cdrom', iso
  ]
elif qemu_win.found()
  qemu_wrapper = find_program('scripts/qemu_windows.sh')
  qemu_cmd += [
    qemu_wrapper, qemu_win.path(), iso, qemu_common_args
  ]
endif

if qemu.found() or qemu_win.found()
  run_target('run',
    depends: iso,
    command: qemu_cmd)

  run_target('run-debug',
    depends: iso,
    command: qemu_cmd + ['-s', '-S', '-d', 'int'])
endif
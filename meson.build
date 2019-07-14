project('SimOS',
  'cpp', 'c',
  version: '0.1',
  default_options: [
    'warning_level=2', 
    'cpp_std=gnu++2a',
    'c_std=gnu11'
  ])

cpp_compiler = meson.get_compiler('cpp').cmd_array()[0]

qemu = find_program('qemu-system-x86_64', required: false)
qemu_win = find_program('/mnt/c/dev/qemu/qemu-system-x86_64.exe', required: false)

build_iso = find_program('scripts/build_iso.sh')

kernel_sources = files([
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
])

crt_sources = files(['src/crti.S', 'src/crtn.S'])

crtbegin_obj = run_command(cpp_compiler, '-print-file-name=crtbegin.o').stdout().strip()
crtend_obj = run_command(cpp_compiler, '-print-file-name=crtend.o').stdout().strip()

# this is a gross hack to link crti, crtbegin, crtend, and crtn in a specific
# order (needed for global object constructors).
# basically we compile everything as a static library, then extract the objects and
# link the final binary with the GCC crtbegin/crtend objects.
kernel_lib = static_library('kernel.lib',
  kernel_sources, crt_sources,
  include_directories: 'include',
  pic : false)

kernel = executable('kernel.elf',
  objects : [
    kernel_lib.extract_objects('src/crti.S'),
    crtbegin_obj,
    kernel_lib.extract_objects(kernel_sources),
    crtend_obj,
    kernel_lib.extract_objects('src/crtn.S'),
  ])

iso = custom_target('iso',
  output: 'simo.iso',
  input: ['src/boot/grub.cfg', kernel],
  command: [build_iso, '@OUTPUT@', '@OUTDIR@', '@INPUT0@', '@INPUT1@'])

qemu_common_args = ['-m', '1G', '-no-reboot', '-no-shutdown', '-monitor', 'stdio']
qemu_cmd = []

if qemu.found()
  qemu_cmd += [qemu, qemu_common_args, '-enable-kvm', '-cpu', 'host', '-cdrom', iso]
elif qemu_win.found()
  qemu_wrapper = find_program('scripts/qemu_windows.sh')
  qemu_cmd += [qemu_wrapper, qemu_win.path(), iso, qemu_common_args]
endif

if qemu.found() or qemu_win.found()
  run_target('run',
    depends: iso,
    command: qemu_cmd)

  run_target('run-debug',
    depends: iso,
    command: qemu_cmd + ['-s', '-S', '-d', 'int'])
endif

subdir('test')
project('SimOS',
  'cpp', 'c',
  version: '0.1',
  default_options: [
    'warning_level=2', 
    'cpp_std=gnu++2a',
    'c_std=gnu11'
  ])

cpp_compiler = meson.get_compiler('cpp').cmd_array()[0]

qemu_win = find_program('/mnt/c/dev/qemu/qemu-system-x86_64.exe', required: false)
qemu = find_program('qemu-system-x86_64', required: false)

build_iso = find_program('scripts/build_iso.sh')

kernel_sources = files([
  'src/Utils.cpp',
  'src/KMain.cpp',
  'src/Console.cpp',
  'src/Interrupt.cpp',
  'src/GDT.cpp',
  'src/printf.c',
  'src/Paging.cpp',
  'src/FrameMap.cpp',
  'src/Serial.cpp',
  'src/boot/boot.S',
  'src/boot/long_mode_init.S',
  'src/boot/multiboot.S',
  'src/boot/bootsplash.cpp',
])

crt_sources = files(['src/crti.S', 'src/crtn.S'])

# TODO: currently these aren't used because -print-file-name=crtbegin/end.o doesn't do anything on clang
crtbegin_obj = run_command(cpp_compiler, '-print-file-name=crtbegin.o').stdout().strip()
crtend_obj = run_command(cpp_compiler, '-print-file-name=crtend.o').stdout().strip()

# this is a gross hack to link crti, crtbegin, crtend, and crtn in a specific
# order (needed for global object constructors).
# basically we compile everything as a static library, then extract the objects and
# link the final binary with the GCC crtbegin/crtend objects.
kernel_lib = static_library('kernel.lib',
  kernel_sources, crt_sources,
  include_directories: 'include',
  pic: false)

kernel = executable('kernel.elf',
  link_language: 'cpp',
  link_args: ['-T' + join_paths(meson.current_source_dir(), 'src', 'linker.ld')],
  objects: [
    kernel_lib.extract_objects('src/crti.S'),
    #crtbegin_obj,
    kernel_lib.extract_objects(kernel_sources),
    #crtend_obj,
    kernel_lib.extract_objects('src/crtn.S'),
  ])

iso = custom_target('iso',
  output: 'simo.iso',
  input: ['src/boot/grub.cfg', kernel],
  command: [build_iso, '@OUTPUT@', '@OUTDIR@', '@INPUT0@', '@INPUT1@'])

qemu_common_args = ['-m', '1G', '-no-reboot', '-no-shutdown', '-monitor', 'stdio']
qemu_cmd = []

qemu_found = false
if qemu_win.found()
  qemu_wrapper = find_program('scripts/qemu_windows.sh')
  qemu_cmd += [qemu_wrapper, qemu_win.path(), iso, qemu_common_args, '-serial', 'file:CON']
  qemu_found = true
elif qemu.found()
  qemu_cmd += [qemu, qemu_common_args, '-enable-kvm', '-cpu', 'host', '-cdrom', iso, '-serial', 'stdio']
  qemu_found = true
endif

if qemu_found
  run_target('run',
    depends: iso,
    command: qemu_cmd)

  run_target('run-debug',
    depends: iso,
    command: qemu_cmd + ['-s', '-S', '-d', 'int'])
endif

subdir('test')
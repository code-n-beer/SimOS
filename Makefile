arch ?= x86_64
kernel := build/kernel-$(arch).bin
iso := build/os-$(arch).iso
target ?= ${arch}-simos

linker_script := src/arch/$(arch)/linker.ld
grub_cfg := src/arch/$(arch)/grub.cfg
assembly_source_files := $(wildcard src/arch/$(arch)/*.asm)
assembly_object_files := $(patsubst src/arch/$(arch)/%.asm, \
    build/arch/$(arch)/%.o, $(assembly_source_files))

cpp_source_files := $(wildcard src/*.cpp)
cpp_object_files := $(patsubst src/%.cpp, \
    build/%.o, $(cpp_source_files))

CXXFLAGS += -g -ffreestanding -Wall -Wextra -std=c++17

.PHONY: all clean run iso kernel

all: $(kernel)

clean:
	@rm -r build

run: $(iso)
	@qemu-system-x86_64 -m 1G -cdrom $(iso)

iso: $(iso)

$(iso): $(kernel) $(grub_cfg)
	@mkdir -p build/isofiles/boot/grub
	@cp $(kernel) build/isofiles/boot/kernel.bin
	@cp $(grub_cfg) build/isofiles/boot/grub
	@grub-mkrescue -o $(iso) build/isofiles 2> /dev/null
	@rm -r build/isofiles

$(kernel): $(cpp_object_files) $(assembly_object_files) $(linker_script)
	@ld -n --gc-sections -T $(linker_script) -o $(kernel) \
		$(assembly_object_files) $(cpp_object_files)

$(cpp_object_files): $(cpp_source_files)
	@mkdir -p $(shell dirname $@)
	x86_64-elf-g++ $(CXXFLAGS) -c $< -o $@

# compile assembly files
build/arch/$(arch)/%.o: src/arch/$(arch)/%.asm
	@mkdir -p $(shell dirname $@)
	@nasm -felf64 $< -o $@

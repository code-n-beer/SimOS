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

CXXFLAGS += -g -ffreestanding -Wall -Wextra -std=c++17 -Iinclude/

.PHONY: all clean run iso kernel

all: $(kernel)

clean:
	rm -rf build

run: $(iso)
	qemu-system-x86_64 -m 1G -cdrom $(iso)

iso: $(iso)

build_dir:
	@mkdir -p build

$(iso): $(kernel) $(grub_cfg)
	@mkdir -p build/isofiles/boot/grub
	cp $(kernel) build/isofiles/boot/kernel.bin
	cp $(grub_cfg) build/isofiles/boot/grub
	grub-mkrescue -o $(iso) build/isofiles
	rm -r build/isofiles

$(kernel): build_dir $(cpp_object_files) $(assembly_object_files) $(linker_script)
	x86_64-elf-ld -n --gc-sections -T $(linker_script) -o $(kernel) \
		$(assembly_object_files) $(cpp_object_files)

build/%.o: src/%.cpp
	@mkdir -p $(shell dirname $@)
	x86_64-elf-g++ $(CXXFLAGS) -c $< -o $@

build/arch/$(arch)/%.o: src/arch/$(arch)/%.asm
	@mkdir -p $(shell dirname $@)
	nasm -felf64 $< -o $@

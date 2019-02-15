ARCH ?= x86_64

CXX := $(ARCH)-elf-g++
CC := $(ARCH)-elf-gcc
LD := $(ARCH)-elf-ld

KERNEL := build/kernel-$(ARCH).bin
ISO := build/os-$(ARCH).iso

LINKER_SCRIPT := src/linker.ld
GRUB_CFG := src/boot/grub.cfg

ASM_SRCS := $(wildcard src/boot/*.S)
ASM_OBJS := $(patsubst src/boot/%.S, build/boot/%.o, $(ASM_SRCS))

CXX_SRCS := $(wildcard src/*.cpp)
CXX_OBJS := $(patsubst src/%.cpp, build/%.o, $(CXX_SRCS))

C_SRCS := $(wildcard src/*.c)
C_OBJS := $(patsubst src/%.c, build/%.o, $(C_SRCS))

SRCS = $(C_SRCS) $(CXX_SRCS)

# TODO: fix shit so we don't need to use -mgeneral-regs-only or disable float support in printf
COMMON_CFLAGS := -g -ffreestanding -Wall -Wextra -Iinclude/ -mno-red-zone -mcmodel=kernel -flto -fno-strict-aliasing -m64 -O2 -mgeneral-regs-only -DPRINTF_DISABLE_SUPPORT_FLOAT
CXXFLAGS += $(COMMON_CFLAGS) -std=gnu++2a -fno-exceptions -fno-rtti -fconcepts
CFLAGS += $(COMMON_CFLAGS) -std=gnu11
LDFLAGS += -z max-page-size=0x1000 -flto

CRTI_OBJ := build/crti.o
CRTBEGIN_OBJ := $(shell $(CC) $(CFLAGS) -print-file-name=crtbegin.o)
CRTEND_OBJ := $(shell $(CC) $(CFLAGS) -print-file-name=crtend.o)
CRTN_OBJ := build/crtn.o

OBJS := $(CXX_OBJS) $(C_OBJS) $(ASM_OBJS) $(ASM2_OBJS)
ALL_OBJS := $(CRTI_OBJ) $(CRTBEGIN_OBJ) $(OBJS) $(CRTEND_OBJ) $(CRTN_OBJ)

DEPFLAGS = -MT $@ -MMD -MP -MF build/$*.Td
POSTCOMPILE = @mv -f build/$*.Td build/$*.d && touch $@

.PHONY: all clean run iso kernel test

all: $(KERNEL)

clean:
	rm -rf build

run: $(ISO)
	qemu-system-x86_64 -m 1G -cdrom $(ISO) -cpu Skylake-Client -no-reboot -no-shutdown -d int -monitor stdio

iso: $(ISO)

build_dir:
	@mkdir -p build

$(ISO): $(KERNEL) $(GRUB_CFG)
	@mkdir -p build/isofiles/boot/grub
	@cp $(KERNEL) build/isofiles/boot/kernel.bin
	@cp $(GRUB_CFG) build/isofiles/boot/grub
	@grub-mkrescue -o $(ISO) build/isofiles
	@rm -r build/isofiles

$(KERNEL): build_dir $(ALL_OBJS) $(LINKER_SCRIPT)
	@echo "Linking..."
	@$(CC) $(LDFLAGS) -T $(LINKER_SCRIPT) -o $(KERNEL) $(ALL_OBJS) -nostdlib -lgcc

build/%.o: src/%.cpp build/%.d
	@mkdir -p $(shell dirname $@)
	@echo "[CXX]  $<"
	@$(CXX) $(DEPFLAGS) $(CXXFLAGS) -c $< -o $@
	$(POSTCOMPILE)

build/%.o: src/%.c build/%.d
	@mkdir -p $(shell dirname $@)
	@echo "[CC]   $<"
	@$(CC) $(DEPFLAGS) $(CFLAGS) -c $< -o $@
	$(POSTCOMPILE)

build/%.o: src/%.S
	@mkdir -p $(shell dirname $@)
	@echo "[CC]   $<"
	@$(CC) $(DEPFLAGS) $(CFLAGS) -c $< -o $@
	$(POSTCOMPILE)

test:
	$(MAKE) -C test/ run

build/%.d: ;
.PRECIOUS: build/%.d

include $(wildcard $(patsubst src/%,build/%.d,$(basename $(SRCS))))
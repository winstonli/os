CC := clang
CXX := clang++
LD := ld
AS := nasm

MODULES := kernel.bin
COMMON_OBJFILES += src/common/common.o src/common/string.o \
                   src/common/interrupts.o

COMMON_FLAGS += -fPIC --target=x86_64-pc-none-elf -ffreestanding -fno-builtin \
                -fno-exceptions -fno-rtti \
                -MMD -mno-sse -mno-mmx -Wall -Wextra \
                -pedantic -Wshadow -Wpointer-arith -Wcast-align \
                -Wwrite-strings -Wredundant-decls \
                -Winline -Wno-long-long -Wuninitialized
CFLAGS += $(COMMON_FLAGS) -std=c11
CXXFLAGS += $(COMMON_FLAGS) -std=c++14 -Isrc/ -I src/modules/kernel

HFILES = $(shell find src/ -type f -name '*.h')
CFILES = $(shell find src/ -type f -name '*.c')
CXXFILES = $(shell find src/ -type f -name '*.cpp')
DEPFILES = $(CFILES:.c=.d) $(CXXFILES:.cpp=.d)

LDFLAGS := -nostdlib

# always run clang-format regardless
$(shell clang-format -i $(CFILES) $(CXXFILES) $(HFILES))

all: kernel.iso
.PHONY: all

-include $(DEPFILES)

qemu: kernel.iso
	qemu-system-x86_64 $<
.PHONY: qemu

bochs: kernel.iso bochsrc.txt
	bochs
.PHONY: qemu

kernel.iso: start grub.cfg $(MODULES)
	mkdir -p iso/boot/grub
	cp start iso/boot/start
	cp grub.cfg iso/boot/grub/grub.cfg
	cp $(MODULES) iso/boot/
	grub-mkrescue -o $@ iso

start: start.ld src/start.o
	$(LD) -T $^ $(LDFLAGS) -o $@

%.o: %.s
	$(AS) -f elf64 $^ -o $@

%.bin: %.elf
	objcopy -O binary $< $@

kernel.elf: module.ld src/modules/kernel/entry.o src/modules/kernel/main.o \
            src/modules/kernel/terminal.o src/modules/kernel/idt.o \
            src/modules/kernel/interrupt_stubs.o src/modules/kernel/isr.o \
            src/modules/kernel/irq.o \
            src/modules/kernel/pic.o \
            src/modules/kernel/register.o \
            src/modules/kernel/boot/multiboot_info.o \
            src/modules/kernel/util/string_util.o \
            src/modules/kernel/vm/page_table.o \
            src/modules/kernel/vm/pallocator.o \
            src/modules/kernel/vm/pml4e.o \
            src/modules/kernel/vm/pdpe.o \
            src/modules/kernel/vm/pde.o \
            src/modules/kernel/vm/pte.o \
            src/modules/kernel/vm/vm.o \
            $(COMMON_OBJFILES)
	$(LD) -T $^ $(LDFLAGS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	find src/ -type f \( -name '*.o' -o -name '*.d' \) | xargs rm -fv
	rm -fv *.iso *.elf *.bin
	rm -fv start
	rm -rfv iso/
.PHONY: clean

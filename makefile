CC := clang
CXX := clang++
LD := ld
AS := nasm

MODULES := kernel.mod
COMMON_OBJFILES += src/common/common.o src/common/string.o \
                   src/common/interrupts.o

COMMON_FLAGS += -fpic --target=x86_64-pc-none-elf -ffreestanding -fno-builtin \
                -nostdlib -nostdinc -fno-exceptions -fno-rtti \
                -Wimplicit-fallthrough -MMD -mno-sse -mno-mmx -Wall -Wextra \
                -pedantic -Wshadow -Wpointer-arith -Wcast-align \
                -Wwrite-strings -Wmissing-declarations -Wredundant-decls \
                -Wnested-externs -Winline -Wno-long-long -Wuninitialized
CFLAGS += $(COMMON_FLAGS) -std=c11
CXXFLAGS += $(COMMON_FLAGS) -std=c++14 -Isrc/

HFILES = $(shell find . -type f -name '*.h')
CFILES = $(shell find . -type f -name '*.c')
CXXFILES = $(shell find . -type f -name '*.cpp')
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

kernel.mod: module.ld src/modules/kernel/entry.o src/modules/kernel/main.o src/modules/kernel/terminal.o $(COMMON_OBJFILES)
	$(LD) --gc-sections -shared -fpie -T $^ $(LDFLAGS) -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	find . -type f \( -name '*.o' -o -name '*.mod' -o -name '*.d' \) | xargs rm -fv
	rm -fv *.iso
	rm -fv start
	rm -rfv iso/
.PHONY: clean

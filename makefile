MODULES := kernel.mod

ASMFILES := src/start.s
OBJFILES := $(ASMFILES:.s=.o)

COMMON_FLAGS += --target=x86_64-pc-none-elf -ffreestanding -fno-builtin -nostdlib -nostdinc -fno-exceptions -fno-rtti -MMD
CFLAGS += $(COMMON_FLAGS)
CXXFLAGS += $(COMMON_FLAGS)

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

kernel.iso: loader grub.cfg $(MODULES)
	mkdir -p iso/boot/grub
	cp loader iso/boot/loader
	cp grub.cfg iso/boot/grub/grub.cfg
	cp $(MODULES) iso/boot/
	grub-mkrescue -o $@ iso

loader: loader.ld src/start.o
	ld -T $^ $(LDFLAGS) -o $@

%.o: %.s
	nasm -f elf64 $^ -o $@

kernel.mod: module.ld src/modules/kernel/entry.o src/modules/kernel/main.o src/common/common.o
	ld --gc-sections -shared -fpie -T module.ld $^ $(LDFLAGS) -o $@

%.o: %.c
	clang $(CFLAGS) -c -o $@ $<

%.o: %.cpp
	clang++ $(CXXFLAGS) -c -o $@ $<

clean:
	find . -type f \( -name '*.o' -o -name '*.mod' -o -name '*.d' \) | xargs rm -fv
	rm -fv kernel.iso
	rm -fv kernel
	rm -rfv iso/
.PHONY: clean

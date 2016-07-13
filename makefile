MODULES := hello.mod

ASMFILES := $(shell find . -type f -name '*.s')
OBJFILES := $(ASMFILES:.s=.o)

CXXFLAGS += --target=i686-pc-none-elf -march=i686 -I/home/george/grub/include/ -I/home/george/grub/

LDFLAGS := -melf_i386 -nostdlib

all: kernel.iso
.PHONY: all

qemu: kernel.iso
	qemu-system-x86_64 $<
.PHONY: qemu

bochs: kernel.iso bochsrc.txt
	bochs
.PHONY: qemu

kernel.iso: kernel grub.cfg $(MODULES)
	mkdir -p iso/boot/grub
	cp kernel iso/boot/kernel
	cp grub.cfg iso/boot/grub/grub.cfg
	cp $(MODULES) iso/boot/grub/
	grub-mkrescue -o $@ iso

kernel: linker.ld $(OBJFILES)
	ld $(LDFLAGS) -T linker.ld $(OBJFILES) -o $@

%.o: %.s
	nasm -f elf $^ -o $@

%.mod: %.o
	ld $(LDFLAGS) -r -o $@ $^

%.o: %.c
	clang $(CXXFLAGS) -c -o $@ $<

clean:
	find . -type f -name '*.o' -o -name '*.mod' | xargs rm -f
	rm -f kernel.iso
	rm -f kernel
	rm -rf iso/
.PHONY: clean

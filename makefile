MODULES := hello.mod

ASMFILES := src/start.s
OBJFILES := $(ASMFILES:.s=.o)

COMMON_FLAGS += --target=x86_64-pc-none-elf -ffreestanding -fno-builtin -nostdlib -nostdinc -fno-exceptions -fno-rtti
CFLAGS += $(COMMON_FLAGS)
CXXFLAGS += $(COMMON_FLAGS) -nostdinc++

LDFLAGS := -nostdlib

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
	cp $(MODULES) iso/boot/
	grub-mkrescue -o $@ iso

kernel: kernel.ld $(OBJFILES)
	ld $(LDFLAGS) -T kernel.ld $(OBJFILES) -o $@

%.o: %.s
	nasm -f elf64 $^ -o $@

hello.mod: hello.o hello2.o
	ld $(LDFLAGS) --gc-sections -shared -fpie -T module.ld -o $@ $^

%.o: %.c
	clang $(CFLAGS) -c -o $@ $<

%.o: %.cpp
	clang++ $(CXXFLAGS) -c -o $@ $<

clean:
	find . -type f -name '*.o' -o -name '*.mod' | xargs rm -f
	rm -f kernel.iso
	rm -f kernel
	rm -rf iso/
.PHONY: clean

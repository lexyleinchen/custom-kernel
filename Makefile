TARGET = x86_64-elf

CC = $(TARGET)-gcc
CXX = $(TARGET)-g++
LD = $(TARGET)-ld
AS = nasm

CFLAGS = -ffreestanding \
	-fno-stack-protector \
	-fno-pie \
	-mno-red-zone \
	-Wall \
	-Wextra

CXXFLAGS = -ffreestanding \
	-fno-stack-protector \
	-fno-rtti \
	-fno-pie \
	-mno-red-zone \
	-Wall \
	-Wextra

LDFLAGS = -T src/linker.ld

KERNEL = build/kernel.bin

all: $(KERNEL)

build:
	mkdir -p build

build/kernel.o: src/kernel/kernel.c | build
	$(CC) $(CFLAGS) -Isrc/kernel -Isrc/os -c $< -o $@

build/log.o: src/kernel/log.c | build
	$(CC) $(CFLAGS) -Isrc/kernel -c $< -o $@

build/framebuffer.o: src/kernel/framebuffer.c | build
	$(CC) $(CFLAGS) -Isrc/kernel -c $< -o $@

build/multiboot.o: src/kernel/multiboot.c | build
	$(CC) $(CFLAGS) -Isrc/kernel -c $< -o $@

build/ps2.o: src/kernel/ps2.c | build
	$(CC) $(CFLAGS) -Isrc/kernel -c $< -o $@

build/pci.o: src/kernel/pci.c | build
	$(CC) $(CFLAGS) -Isrc/kernel -c $< -o $@

build/usb.o: src/kernel/usb.c | build
	$(CC) $(CFLAGS) -Isrc/kernel -c $< -o $@

build/mouse.o: src/kernel/mouse.c | build
	$(CC) $(CFLAGS) -Isrc/kernel -c $< -o $@

build/boot.o: src/boot/boot.asm | build
	$(AS) -f elf64 $< -o $@

build/os.o: src/os/os.cpp | build
	$(CXX) $(CXXFLAGS) -Isrc/os -Isrc/kernel -c $< -o $@

build/graphics.o: src/os/graphics.cpp | build
	$(CXX) $(CXXFLAGS) -Isrc/os -c $< -o $@

build/ui.o: src/os/ui.cpp | build
	$(CXX) $(CXXFLAGS) -Isrc/os -c $< -o $@

build/font.o: src/os/font.cpp | build
	$(CXX) $(CXXFLAGS) -Isrc/os -c $< -o $@

build/taskbar.o: src/os/taskbar.cpp | build
	$(CXX) $(CXXFLAGS) -Isrc/os -c $< -o $@

build/terminal.o: src/os/terminal.cpp | build
	$(CXX) $(CXXFLAGS) -Isrc/os -c $< -o $@

build/os_mouse.o: src/os/os_mouse.cpp | build
	$(CXX) $(CXXFLAGS) -Isrc/os -c $< -o $@

$(KERNEL): build/boot.o \
		build/kernel.o \
		build/log.o \
		build/framebuffer.o \
		build/multiboot.o \
		build/ps2.o \
		build/pci.o \
		build/usb.o \
		build/mouse.o \
		build/os.o \
		build/graphics.o \
		build/ui.o \
		build/font.o \
		build/taskbar.o \
		build/terminal.o \
		build/os_mouse.o
	$(LD) $(LDFLAGS) -o $@ $^

iso: $(KERNEL)
	mkdir -p iso/boot/grub
	cp $(KERNEL) iso/boot/kernel.bin
	grub2-mkrescue -o build/PrintOS.iso iso

clean:
	rm -rf build
	rm -rf iso/boot/kernel.bin

.PHONY: all iso clean

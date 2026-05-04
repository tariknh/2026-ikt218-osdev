########################################
# The University of Agder Operating System: UiAOS
# Languages: C, C++, and NASM Assembly
########################################

set(CMAKE_C_COMPILER_WORKS 1)
set(CMAKE_CXX_COMPILER_WORKS 1)

cmake_minimum_required(VERSION 3.22.1)

project(UiAOS LANGUAGES C CXX ASM_NASM)

file(LOCK ${CMAKE_SOURCE_DIR} DIRECTORY GUARD FILE)

include(FetchContent)

set(OS_ARCH_TARGET "i386")
set(OS_NAME "UiA Operating System")
set(OS_KERNEL_NAME "uiaos")
set(OS_KERNEL_BINARY "kernel.bin")
set(OS_KERNEL_IMAGE "kernel.iso")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 99)

set(CMAKE_ASM_NASM_SOURCE_FILE_EXTENSIONS "s;S;asm")
if(OS_ARCH_TARGET STREQUAL "i386")
	set(CMAKE_ASM_NASM_OBJECT_FORMAT "elf32")
elseif(OS_ARCH_TARGET STREQUAL "x86_64")
	set(CMAKE_ASM_NASM_OBJECT_FORMAT "elf64")
endif()

set(CMAKE_ASM_NASM_COMPILE_OBJECT
    "<CMAKE_ASM_NASM_COMPILER> <DEFINES> <INCLUDES> <FLAGS> -f ${CMAKE_ASM_NASM_OBJECT_FORMAT} -o <OBJECT> <SOURCE>")

set(OS_KERNEL_LINKER "${CMAKE_CURRENT_SOURCE_DIR}/src/arch/${OS_ARCH_TARGET}/linker.ld")

# Assignment 6: full kernel + interactive shell with RTC clock and command dispatch
add_executable(uiaos-kernel
    src/multiboot2.asm
    src/kernel.c
    src/lib/gdt.c
    src/lib/gdt.asm
    src/lib/idt.c
    src/lib/idt.asm
    src/lib/isr.c
    src/lib/isr.asm
    src/lib/irq.c
    src/lib/irq.asm
    src/lib/keyboard.c
    src/lib/terminal.c
    src/lib/memory.c
    src/lib/paging.c
    src/lib/pit.c
    src/lib/song_player.c
    src/lib/songs.c
    src/lib/rtc.c
    src/lib/shell.c
    src/lib/cpp_support.cpp
    src/libc/stdio.c
    src/libc/string.c
)

target_include_directories(uiaos-kernel PUBLIC include)

target_compile_options(uiaos-kernel PRIVATE
    $<$<COMPILE_LANGUAGE:C>:-Wall -Wextra -nostdinc -nostdlib -fno-builtin -fno-stack-protector -fno-stack-check -fno-lto -fPIE -m32 -march=i386 -mno-mmx -mno-sse -mno-sse2 -mno-red-zone -Wno-main -g>
    $<$<COMPILE_LANGUAGE:CXX>:-Wall -Wextra -nostdinc -nostdlib -fno-builtin -fno-stack-protector -fno-stack-check -fno-lto -fno-exceptions -fno-rtti -fPIE -m32 -march=i386 -mno-mmx -mno-sse -mno-sse2 -mno-red-zone -g>
    $<$<OR:$<COMPILE_LANGUAGE:C>,$<COMPILE_LANGUAGE:CXX>>:-m32 -march=i386 -Wno-unused-variable -Wno-unused-parameter>
)

target_link_options(uiaos-kernel PUBLIC
    $<$<OR:$<COMPILE_LANGUAGE:C>,$<COMPILE_LANGUAGE:CXX>>:-ffreestanding -nostdlib -fno-builtin -static -pie -O0 -T${OS_KERNEL_LINKER} -g>
)

target_link_options(
	uiaos-kernel
	PUBLIC "-ffreestanding"
	PUBLIC "-nostdlib"
	PUBLIC "-static"
	PUBLIC "-pie"
	PUBLIC "-T${OS_KERNEL_LINKER}"
)

set_target_properties(uiaos-kernel PROPERTIES
    OUTPUT_NAME "${OS_KERNEL_BINARY}"
)

set(DISK_IMAGE "${CMAKE_CURRENT_BINARY_DIR}/disk.iso")

add_custom_target(
    create-fat32-disk
    COMMAND dd if=/dev/zero of=${DISK_IMAGE} bs=1M count=32
    COMMAND mkfs.fat -F 32 ${DISK_IMAGE}
    VERBATIM
)

set(ISO_DIR ${CMAKE_CURRENT_BINARY_DIR}/iso)
set(LIMINE_CONFIG_DIR ${CMAKE_SOURCE_DIR})
set(LIMINE_DIR /usr/local/limine)
add_custom_target(
	uiaos-create-image
	COMMAND rm -rf ${ISO_DIR}
	COMMAND mkdir -p ${ISO_DIR}
	COMMAND cp -v $<TARGET_FILE:uiaos-kernel>
	 	${LIMINE_CONFIG_DIR}/limine.cfg ${LIMINE_DIR}/limine-bios.sys ${LIMINE_DIR}/limine-bios-cd.bin
		${LIMINE_DIR}/limine-uefi-cd.bin ${ISO_DIR}/
	COMMAND mkdir -p ${ISO_DIR}/EFI/BOOT
	COMMAND cp -v ${LIMINE_DIR}/BOOTX64.EFI ${ISO_DIR}/EFI/BOOT/
	COMMAND cp -v ${LIMINE_DIR}/BOOTIA32.EFI ${ISO_DIR}/EFI/BOOT/
	COMMAND xorriso -as mkisofs -b limine-bios-cd.bin
	 	-no-emul-boot -boot-load-size 4 -boot-info-table
	 	--efi-boot limine-uefi-cd.bin
	 	-efi-boot-part --efi-boot-image --protective-msdos-label
	 	${ISO_DIR} -o ${CMAKE_CURRENT_BINARY_DIR}/kernel.iso
	COMMAND ${LIMINE_DIR}/limine bios-install ${CMAKE_CURRENT_BINARY_DIR}/kernel.iso
	DEPENDS create-fat32-disk uiaos-kernel
	VERBATIM
)

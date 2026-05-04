########################################
# The University of Agder Operating System: UiAOS
# Fast minimal setup for Assignment 2
########################################

set(CMAKE_C_COMPILER_WORKS 1)
set(CMAKE_CXX_COMPILER_WORKS 1)

cmake_minimum_required(VERSION 3.22.1)

project(UiAOS LANGUAGES C CXX ASM_NASM)

file(LOCK ${CMAKE_SOURCE_DIR} DIRECTORY GUARD FILE)

########################################
# Variables
########################################
set(OS_ARCH_TARGET "i386")
set(OS_KERNEL_BINARY "kernel.bin")

########################################
# Compiler configuration
########################################
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 20)

########################################
# NASM configuration
########################################
set(CMAKE_ASM_NASM_SOURCE_FILE_EXTENSIONS "asm")
if(OS_ARCH_TARGET STREQUAL "i386")
    set(CMAKE_ASM_NASM_OBJECT_FORMAT "elf32")
elseif(OS_ARCH_TARGET STREQUAL "x86_64")
    set(CMAKE_ASM_NASM_OBJECT_FORMAT "elf64")
endif()

set(CMAKE_ASM_NASM_COMPILE_OBJECT
    "<CMAKE_ASM_NASM_COMPILER> <DEFINES> <INCLUDES> <FLAGS> -f ${CMAKE_ASM_NASM_OBJECT_FORMAT} -o <OBJECT> <SOURCE>"
)

########################################
# Kernel target
########################################
set(OS_KERNEL_LINKER "${CMAKE_CURRENT_SOURCE_DIR}/src/arch/${OS_ARCH_TARGET}/linker.ld")

add_executable(uiaos-kernel
    src/multiboot2.asm
    src/gdt_flush.asm
    src/kernel.c
    src/irq_rush.c
    src/keyboard.c
    src/irq.c
    src/ports.c
    src/idt.c
    src/isr.c
    src/interrupts.asm
    src/memory/malloc.c
    src/memory/memory.c
    src/memory/memutils.c
    src/cpp/new.cpp
    src/pit.c
    src/song/SongPlayer.c
)

target_include_directories(uiaos-kernel PUBLIC include)

target_compile_options(uiaos-kernel PRIVATE
    $<$<COMPILE_LANGUAGE:C>:
        -Wall
        -Wextra
        -ffreestanding
        -nostdinc
        -fno-builtin
        -fno-stack-protector
        -fno-stack-check
        -fno-lto
        -fno-pie
        -m32
        -march=i386
        -mno-mmx
        -mno-sse
        -mno-sse2
        -Wno-main
        -g
    >
    $<$<COMPILE_LANGUAGE:CXX>:
        -Wall
        -Wextra
        -ffreestanding
        -nostdinc
        -fno-builtin
        -fno-stack-protector
        -fno-stack-check
        -fno-lto
        -fno-pie
        -fno-exceptions
        -fno-rtti
        -fno-use-cxa-atexit
        -m32
        -march=i386
        -mno-mmx
        -mno-sse
        -mno-sse2
        -g
    >
)

target_link_options(uiaos-kernel PRIVATE
    -ffreestanding
    -nostdlib
    -static
    -no-pie
    -m32
    -T${OS_KERNEL_LINKER}
    -g
)

set_target_properties(uiaos-kernel PROPERTIES
    OUTPUT_NAME "${OS_KERNEL_BINARY}"
)

########################################
# Create empty FAT32 disk image
########################################
set(DISK_IMAGE "${CMAKE_CURRENT_BINARY_DIR}/disk.iso")

add_custom_target(
    create-fat32-disk
    COMMAND dd if=/dev/zero of=${DISK_IMAGE} bs=1M count=32
    COMMAND mkfs.fat -F 32 ${DISK_IMAGE}
    VERBATIM
)

########################################
# Create bootable ISO
########################################
set(ISO_DIR ${CMAKE_CURRENT_BINARY_DIR}/iso)
set(LIMINE_CONFIG_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(LIMINE_DIR /usr/local/limine)

add_custom_target(
    uiaos-create-image
    COMMAND rm -rf ${ISO_DIR}
    COMMAND mkdir -p ${ISO_DIR}
    COMMAND cp -v
            $<TARGET_FILE:uiaos-kernel>
            ${LIMINE_CONFIG_DIR}/limine.cfg
            ${LIMINE_DIR}/limine-bios.sys
            ${LIMINE_DIR}/limine-bios-cd.bin
            ${LIMINE_DIR}/limine-uefi-cd.bin
            ${ISO_DIR}/
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

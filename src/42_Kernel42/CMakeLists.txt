########################################
# The University of Agder Operating System: Kernel42
# Languages: C, C++, and NASM Assembly
# Tip: Use Ctrl+Shift+P in Visual Studio Code to get started with CMake.
########################################

# Skip compiler self-tests (saves time, avoids errors with some cross compilers)
set(CMAKE_C_COMPILER_WORKS 1)
#set(CMAKE_CXX_COMPILER_WORKS 1)

# Minimum required CMake version
cmake_minimum_required(VERSION 3.22.1)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)


# Project name and languages used
project(Kernel42 LANGUAGES C ASM_NASM)

# Create a lock file to prevent parallel runs of CMake
file(LOCK ${CMAKE_SOURCE_DIR} DIRECTORY GUARD FILE)

########################################
# Kernel42: Variables
########################################
set(OS_ARCH_TARGET "i386")  # x86_64 
set(OS_NAME "Kernel42 Operating System")
set(OS_KERNEL_NAME "Kernel42")
set(OS_KERNEL_BINARY "kernel.bin")
set(OS_KERNEL_IMAGE "kernel.iso")

########################################
# Compiler Configuration
########################################
set(CMAKE_C_STANDARD 99)

########################################
# Assembly Configuration
########################################
set(CMAKE_ASM_NASM_SOURCE_FILE_EXTENSIONS "s;S;asm")
if (OS_ARCH_TARGET STREQUAL "i386")
    set(CMAKE_ASM_NASM_FLAGS "${CMAKE_ASM_NASM_FLAGS} -felf32 -g -F dwarf")
    set(CMAKE_ASM_NASM_OBJECT_FORMAT "elf32")
elseif (OS_ARCH_TARGET STREQUAL "x86_64")
    set(CMAKE_ASM_NASM_FLAGS "${CMAKE_ASM_NASM_FLAGS} -felf64 -g -F dwarf")
    set(CMAKE_ASM_NASM_OBJECT_FORMAT "elf64")
endif ()


# Command to compile NASM files
set(CMAKE_ASM_NASM_OBJECT_FORMAT "elf32")
set(CMAKE_ASM_NASM_COMPILE_OBJECT
        "<CMAKE_ASM_NASM_COMPILER> <DEFINES> <INCLUDES> <FLAGS> -f ${CMAKE_ASM_NASM_OBJECT_FORMAT} -o <OBJECT> <SOURCE>")


########################################
# OS Target
########################################
set(OS_KERNEL_LINKER "${CMAKE_CURRENT_SOURCE_DIR}/src/arch/${OS_ARCH_TARGET}/linker.ld")

# Add executable target for the kernel
add_executable(Kernel42-kernel
        src/kernel/main.c
        src/kernel/util.c
        src/kernel/syscall.c
        src/kernel/util/ring_buffer8.c
        src/kernel/filesystem/vfs.c
        src/kernel/filesystem/ramfs.c

        src/boot/multiboot2.asm

        src/drivers/video/vga_terminal.c

        src/libc/stdio.c
        src/libc/string.c


        src/shell/shell.c
        src/shell/shell_command.c
        src/shell/commands/cat.c
        src/shell/commands/cd.c
        src/shell/commands/clear.c
        src/shell/commands/echo.c
        src/shell/commands/help.c
        src/shell/commands/keyboard_logger.c
        src/shell/commands/loadkeys.c
        src/shell/commands/ls.c
        src/shell/commands/mkdir.c
        src/shell/commands/music_player.c
        src/shell/commands/print_memory.c
        src/shell/commands/pwd.c
        src/shell/commands/test_syscalls.c
        src/shell/commands/test_malloc.c
        src/shell/commands/run_userspace.c
        src/shell/commands/timer_test.c
        src/shell/commands/touch.c
        src/shell/commands/write.c
        src/shell/commands/vfs_test.c


        src/drivers/input/keyboard.c
        src/drivers/input/keymap.c
        src/drivers/input/keymap_us.c
        src/drivers/input/keymap_no.c

        src/arch/i386/cpu/gdt.c
        src/arch/i386/cpu/gdt_routines.asm
        src/arch/i386/cpu/user_mode_asm.asm
        src/arch/i386/cpu/idt.c
        src/arch/i386/cpu/interrupts.asm
        src/arch/i386/cpu/isr.c
        src/arch/i386/cpu/ports.c
        src/arch/i386/cpu/tss.c
        src/arch/i386/cpu/paging_asm.asm
        src/kernel/log.c
        src/kernel/panic.c
        src/libc/stdlib.c
        src/kernel/pmm.c
        src/kernel/paging.c
        src/kernel/page_fault.c
        src/kernel/user_mode.c
        src/kernel/memory.c
        src/kernel/pit.c
        src/drivers/audio/PCSPK.c
        include/drivers/video/vga_text.h

        src/shell/shell_elf.c

)

set_target_properties(Kernel42-kernel PROPERTIES LINKER_LANGUAGE C)

# Build userspace shell ELF and generate C array
set(SHELL_SRC ${CMAKE_SOURCE_DIR}/src/shell/userspace_shell.c)
set(SHELL_ELF ${CMAKE_SOURCE_DIR}/src/shell/shell_elf.elf)
set(SHELL_ELF_C ${CMAKE_SOURCE_DIR}/src/shell/shell_elf.c)
set(SHELL_ELF_SIZE_VAR "const uint32_t shell_elf_size = @SIZE@;")

add_custom_command(
        OUTPUT ${SHELL_ELF}
        COMMAND ${CMAKE_C_COMPILER} -m32 -nostdlib -static -ffreestanding -DUSERSPACE_ONLY -I${CMAKE_SOURCE_DIR}/include -Wl,--section-start=.text=0x08001000 -o ${SHELL_ELF} ${SHELL_SRC}
        DEPENDS ${SHELL_SRC}
        COMMENT "Building userspace shell ELF"
)

add_custom_command(
        OUTPUT ${SHELL_ELF_C}
        COMMAND ${CMAKE_SOURCE_DIR}/scripts/gen_shell_elf.sh ${SHELL_ELF} ${SHELL_ELF_C}
        DEPENDS ${SHELL_ELF}
        COMMENT "Generating shell ELF C array"
)

add_custom_target(shell_elf ALL DEPENDS ${SHELL_ELF_C})
set_property(SOURCE ${SHELL_ELF_C} PROPERTY GENERATED TRUE)

add_dependencies(Kernel42-kernel shell_elf)

# Include directories for the kernel target
target_include_directories(Kernel42-kernel PUBLIC include include/libc)


# Specify compile options for C and C++
target_compile_options(Kernel42-kernel PRIVATE
        $<$<COMPILE_LANGUAGE:C>:-Wall -Wextra -Werror -nostdinc -std=c11 -ffreestanding -Wno-unused-variable -Wno-unused-parameter -nostdlib -fno-builtin -fno-stack-protector -fno-stack-check -fno-lto -m32 -march=i386 -mno-mmx -mno-sse -mno-sse2 -mno-red-zone -Wno-main -g>
)

# Specify link options for C and C++
target_link_options(Kernel42-kernel PUBLIC
        $<$<OR:$<COMPILE_LANGUAGE:C>,$<COMPILE_LANGUAGE:CXX>>: -ffreestanding -nostdinc -nostdlib -fno-builtin -static -O0 -T${OS_KERNEL_LINKER} -g>
)

target_compile_definitions(Kernel42-kernel PRIVATE
        LOG_LEVEL_MAX=5
)


target_link_options(
        Kernel42-kernel
        PUBLIC "-m32"
        PUBLIC "-ffreestanding"
        PUBLIC "-nostdlib"
        PUBLIC "-static"
        PUBLIC "-T${OS_KERNEL_LINKER}"
)


# Set properties for the kernel target
set_target_properties(Kernel42-kernel PROPERTIES
        OUTPUT_NAME "${OS_KERNEL_BINARY}"
        #LINK_FLAGS "${OS_KERNEL_LINK_FLAGS}"
)

########################################
# Create Empty Fat32 Disk Image
########################################
set(DISK_IMAGE "${CMAKE_CURRENT_BINARY_DIR}/disk.iso")

# Custom target to create an empty FAT32 disk image of 32MB
add_custom_target(
        create-fat32-disk
        COMMAND dd if=/dev/zero of=${DISK_IMAGE} bs=1M count=32
        COMMAND mkfs.fat -F 32 ${DISK_IMAGE} || true
        VERBATIM
)

########################################
# OS-Image Target
########################################
set(ISO_DIR ${CMAKE_CURRENT_BINARY_DIR}/iso)
set(LIMINE_CONFIG_DIR ${CMAKE_SOURCE_DIR})
set(LIMINE_DIR /usr/local/limine)
add_custom_target(
        Kernel42-create-image
        COMMAND rm -rf ${ISO_DIR}
        COMMAND mkdir -p ${ISO_DIR}
        COMMAND cp -v $<TARGET_FILE:Kernel42-kernel>
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
        #COMMAND sudo rm -rf ${ISO_DIR}
        DEPENDS create-fat32-disk Kernel42-kernel
        VERBATIM
)
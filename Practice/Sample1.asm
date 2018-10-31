#===============================================================
# File: Sample1.s
# Assemble: gcc -c Sample1.s
# Link: ld Sample1.o –o Sample1
# Run: ./Sample1
#===============================================================
# Add the global directive so the symbol "_start" is made available
# in the object code export table.
# If the symbol is not in the export table at link time, the linker
# will not know about it.
# _start is the default entrypoint for an executable and if the linker
# can see it, it will use that as the Entrypoint.
# If you want a different entrypoint, you can use the -e link option.
# Eg. ld -e main Sample1.o
.global _start
# This is the section where the assembler assumes your code is located
.text
__start:
 # write(1, msgStr, msgStrLen)
# "1" is the sys code for write.
# Note the "$1" tells the assembler to use the value "1".
 mov $1, %rax
# "1" is the stdout file handle.
 mov $1, %rdi
# Address of string to output.
# Note here we are passing a variable prefixed by "$".
# The Assembler will replace $msgStr with the address of msgStr.
 mov $msgStr, %rsi
# Number of bytes in msgStr.
# Note here we are passing a constant.
# Invoke operating system to do the write.
 mov $msgStrLen, %rdx
# System call 60 is exit.
 syscall
 # exit(0)
# We want return code 0.
# Invoke operating system to exit.
 mov $60, %rax
 xor %rdi, %rdi
 syscall
# This is the section where the assembler expects initialized data
Version 1.3.3 Page 98 of 172
# Data types recognized include .byte (1 byte), .short (2 bytes), .long (4 bytes),
# .string or .ascii (length based on length of string). Constants are defined
# with the “=” sign.
.data
CR = 13
LF = 10
msgStr: .ascii "Hello World!\n"
msgStrpost: .byte CR, LF
msgStrLen = .-msgStr

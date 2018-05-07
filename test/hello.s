.text
.global _start
msg:
   .ascii "Hell World!\n"
msg_end:
   .equ len, msg_end - msg
   .equ SYS_write, 1
   .equ SYS_exit, 60
 _start:
    mov $0x04195376,%ebp
    mov $1, %rax    # system call number (sys_write)
    mov $1, %rdi            # file descriptor (stdout)
    mov $msg, %rsi          # message to write
    mov $len, %rdx          # message length.
    
    syscall                 # previous 'int $0x80' in i386

    mov $60, %rax     # system call number (sys_exit)
    mov $0, %rdi            # exit (0)
    syscall                 # previous 'int $0x80' in i386

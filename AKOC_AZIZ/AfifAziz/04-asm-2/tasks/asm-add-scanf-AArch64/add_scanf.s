  .text
  .global add_scanf

  .macro push Xn
    sub sp, sp, 16
    str \Xn, [sp]
  .endm

  .macro pop Xn
    ldr \Xn, [sp]
    add sp, sp, 16
  .endm

add_scanf:
    sub sp, sp, #48
    stp x29, x30, [sp, #0]
    mov x29, sp
    ldr x0, =scanf_format_string
    add x1, sp, #16
    add x2, sp, #24
    bl scanf

    ldr x8, [sp, #16]
    ldr x9, [sp, #24]
    add x0, x8, x9

    ldp x29, x30, [sp, #0]
    add sp, sp, #48

    ret

     .section .rodata

scanf_format_string:
  .string "%lld %lld"


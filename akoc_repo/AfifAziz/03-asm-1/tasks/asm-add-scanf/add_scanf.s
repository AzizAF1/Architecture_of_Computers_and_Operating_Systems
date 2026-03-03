  .intel_syntax noprefix

  .text
  .global add_scanf

add_scanf:
  push rbp
  mov rbp, rsp
  sub rsp, 16

  lea rdi, [scanf_format_string]
  lea rdx, [rsp + 8]
  mov rsi, rsp

  xor rax, rax
  call scanf

  mov rdi, [rsp + 8]
  mov rax, [rsp]
  add rax, rdi

  add rsp, 16

  pop rbp
  ret


  .section .rodata

scanf_format_string:
  .string "%lld %lld"

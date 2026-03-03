.intel_syntax noprefix
.text
.global add
add:
  push rdi
  push rsi

  mov rax, rdi
  mov rdx, rsi

  add rax, rdx

  pop rsi
  pop rdi

  ret

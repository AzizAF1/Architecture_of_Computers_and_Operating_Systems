  .intel_syntax noprefix

  .text
  .global my_memcpy

my_memcpy:
  mov r8, rdi
  mov ecx, edx
  shr ecx, 3

.copy:
  call copy_blocks
  mov ecx, edx
  and ecx, 7
  call copy_bytes
  mov rax, r8
  ret

copy_blocks:
  rep movsq
  ret

copy_bytes:
  rep movsb
  ret

[bits 32]

global memcpy
memcpy:
  push ebp
  mov ebp, esp

  push edi
  push esi
  push ecx

  mov edi, [ebp + 8]    ; Dest
  mov esi, [ebp + 12]   ; Src
  mov ecx, [ebp + 16]   ; Count

  rep movsb

  pop ecx
  pop esi
  pop edi

  mov esp, ebp
  pop ebp
  ret

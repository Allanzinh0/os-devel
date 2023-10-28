global OutB
OutB:
  [bits 32]
  mov dx, [esp + 4]
  mov al, [esp + 8]
  out dx, al
  ret

global InB
InB:
  [bits 32]
  mov dx, [esp + 4]
  xor eax, eax
  in al, dx
  ret

global Panic
Panic:
  [bits 32]
  cli
  hlt

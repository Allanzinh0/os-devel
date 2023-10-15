bits 32

global i686_IDT_Load
i686_IDT_Load:
  [bits 32]
  push ebp
  mov ebp, esp

  ; Load idt
  mov eax, [ebp + 8]
  lidt [eax]

  mov esp, ebp
  pop ebp
  ret

bits 32

global i686_GDT_Load
i686_GDT_Load:
  [bits 32]
  push ebp
  mov ebp, esp

  ; Load gdt
  mov eax, [ebp + 8]
  lgdt [eax]

  ; reload code segment
  mov eax, [ebp + 12]
  push eax
  push .reload_cs
  retf

.reload_cs:
  ; Reload data segments
  mov ax, [ebp + 16]
  mov ds, ax
  mov es, ax
  mov fs, ax
  mov gs, ax
  mov ss, ax

  mov esp, ebp
  pop ebp
  ret

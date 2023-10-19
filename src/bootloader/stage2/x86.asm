%macro x86_EnterRealMode 0
  [bits 32]

  ; 1 - Jump to 16-bit protected mode segment
  jmp word 18h:.pmode16

.pmode16:
  [bits 16]
  ; 2 - Disable protected mode bit in cr0
  mov eax, cr0
  and al, ~1
  mov cr0, eax

  ; 3 - Jump to real mode
  jmp word 00h:.rmode

.rmode:
  ; 4 - Setup segments
  mov ax, 0
  mov ds, ax
  mov ss, ax

  ; 5 - Enable interrupts
  sti

%endmacro


%macro x86_EnterProtectedMode 0
  cli

  ; 1 - Set protection enable flag in cr0
  mov eax, cr0
  or al, 1
  mov cr0, eax

  ; 2 - Far jump into protected mode
  jmp dword 08h:.pmode

.pmode:
  [bits 32]
  ; 3 - Setup segment registers
  mov ax, 0x10
  mov ds, ax
  mov ss, ax

%endmacro

; Convert linear address to segment:offset address
; Arguments:
;   1 - Linear address
;   2 - (out) target segment (e.g. es)
;   3 - Target 32-bit register to use (e.g. eax)
;   4 - Target lower 16-bit half of #3 (e.g. ax)
%macro x86_LinearToSegOffset 4
  mov %3, %1    ; Linear address to eax
  shr %3, 4
  mov %2, %4
  mov %3, %1
  and %3, 0xF
%endmacro

global x86_outb
x86_outb:
  [bits 32]
  mov dx, [esp + 4]
  mov al, [esp + 8]
  out dx, al
  ret

global x86_inb
x86_inb:
  [bits 32]
  mov dx, [esp + 4]
  xor eax, eax
  in al, dx
  ret

global x86_Disk_GetDriveParams
x86_Disk_GetDriveParams:
  [bits 32]
  push ebp
  mov ebp, esp
  
  x86_EnterRealMode

  push es
  push bx
  push esi
  push di

  mov dl, [bp + 8]
  mov ah, 08h
  mov di, 0
  mov es, di
  stc
  int 13h

  mov eax, 1
  sbb eax, 0
 
  ; Drive type
  x86_LinearToSegOffset [bp + 12], es, esi, si
  mov es:[si], bl

  ; Cylinders
  mov bl, ch
  mov bh, cl
  shr bh, 6
  inc bx
  x86_LinearToSegOffset [bp + 16], es, esi, si
  mov es:[si], bx

  ; Sectors
  xor ch, ch
  and cl, 3Fh
  x86_LinearToSegOffset [bp + 20], es, esi, si
  mov es:[si], cx

  ; Heads
  mov cl, dh
  inc cx
  x86_LinearToSegOffset [bp + 24], es, esi, si
  mov es:[si], cx

  pop di
  pop esi
  pop bx
  pop es

  push eax

  x86_EnterProtectedMode

  pop eax

  mov esp, ebp
  pop ebp
  ret

global x86_Disk_Reset
x86_Disk_Reset:
  [bits 32]
  push ebp
  mov ebp, esp

  x86_EnterRealMode

  mov ah, 0
  mov dl, [bp + 8]
  stc
  int 13h

  mov eax, 1
  sbb eax, 0
  push eax

  x86_EnterProtectedMode

  pop eax

  mov esp, ebp
  pop ebp
  ret

global x86_Disk_Read
x86_Disk_Read:
  [bits 32]
  push ebp
  mov ebp, esp

  x86_EnterRealMode

  push ebx
  push es

  mov dl, [bp + 8]

  mov ch, [bp + 12]
  mov cl, [bp + 13]
  shl cl, 6

  mov al, [bp + 16]
  and al, 3Fh
  or cl, al

  mov dh, [bp + 20]

  mov al, [bp + 24]

  x86_LinearToSegOffset [bp + 28], es, ebx, bx

  mov ah, 02h
  stc
  int 13h
  
  mov eax, 1
  sbb eax, 0

  pop es
  pop ebx

  push eax

  x86_EnterProtectedMode

  pop eax

  mov esp, ebp
  pop ebp
  ret

global x86_Video_GetVbeInfo
x86_Video_GetVbeInfo:
  [bits 32]
  push ebp
  mov ebp, esp

  x86_EnterRealMode

  x86_LinearToSegOffset [bp + 8], es, edi, di

  mov eax, 0x4f00
  int 10h

  push eax

  x86_EnterProtectedMode

  pop eax

  mov esp, ebp
  pop ebp
  ret

global x86_Video_GetModeInfo
x86_Video_GetModeInfo:
  [bits 32]
  push ebp
  mov ebp, esp

  x86_EnterRealMode

  push cx

  x86_LinearToSegOffset [bp + 12], es, edi, di

  mov eax, 0x4f01
  mov cx, [bp + 8]
  int 10h

  pop cx

  push eax

  x86_EnterProtectedMode

  pop eax

  mov esp, ebp
  pop ebp
  ret

global x86_Video_SetMode
x86_Video_SetMode:
  [bits 32]
  push ebp
  mov ebp, esp

  x86_EnterRealMode

  push edi
  push es
  push ebx

  mov ax, 0
  mov es, ax
  mov edi, 0
  mov eax, 0x4f02
  mov bx, [bp + 8]
  int 10h

  pop ebx
  pop es
  pop edi

  push eax

  x86_EnterProtectedMode

  pop eax

  mov esp, ebp
  pop ebp
  ret


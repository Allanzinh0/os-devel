%include "arch/i686/ProtectedMode.inc"

global i686_Disk_GetDriveParams
i686_Disk_GetDriveParams:
  [bits 32]
  push ebp
  mov ebp, esp
  
  i686_EnterRealMode

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
  i686_LinearToSegOffset [bp + 12], es, esi, si
  mov es:[si], bl

  ; Cylinders
  mov bl, ch
  mov bh, cl
  shr bh, 6
  inc bx
  i686_LinearToSegOffset [bp + 16], es, esi, si
  mov es:[si], bx

  ; Sectors
  xor ch, ch
  and cl, 3Fh
  i686_LinearToSegOffset [bp + 20], es, esi, si
  mov es:[si], cx

  ; Heads
  mov cl, dh
  inc cx
  i686_LinearToSegOffset [bp + 24], es, esi, si
  mov es:[si], cx

  pop di
  pop esi
  pop bx
  pop es

  push eax

  i686_EnterProtectedMode

  pop eax

  mov esp, ebp
  pop ebp
  ret

global i686_Disk_Reset
i686_Disk_Reset:
  [bits 32]
  push ebp
  mov ebp, esp

  i686_EnterRealMode

  mov ah, 0
  mov dl, [bp + 8]
  stc
  int 13h

  mov eax, 1
  sbb eax, 0
  push eax

  i686_EnterProtectedMode

  pop eax

  mov esp, ebp
  pop ebp
  ret

global i686_Disk_Read
i686_Disk_Read:
  [bits 32]
  push ebp
  mov ebp, esp

  i686_EnterRealMode

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

  i686_LinearToSegOffset [bp + 28], es, ebx, bx

  mov ah, 02h
  stc
  int 13h
  
  mov eax, 1
  sbb eax, 0

  pop es
  pop ebx

  push eax

  i686_EnterProtectedMode

  pop eax

  mov esp, ebp
  pop ebp
  ret

global i686_Disk_ExtensionsPresent
i686_Disk_ExtensionsPresent:
  push ebp
  mov ebp, esp

  i686_EnterRealMode

  push bx

  mov ah, 0x41
  mov bx, 55AAh
  mov dl, [bp + 8]

  stc
  int 13h

  jc .Error

  cmp bx, 0xAA55
  jne .Error

  mov eax, 1
  jmp .EndIf

.Error:
  mov eax, 0

.EndIf:
  pop bx

  push eax

  i686_EnterProtectedMode

  pop eax

  mov esp, ebp
  pop ebp
  ret

global i686_Disk_ExtendedRead
i686_Disk_ExtendedRead:
  push ebp
  mov ebp, esp

  i686_EnterRealMode

  push ds
  push esi

  mov ah, 0x42
  mov dl, [bp + 8]
  i686_LinearToSegOffset [bp + 12], ds, esi, si

  stc
  int 13h

  mov eax, 1
  sbb eax, 0

  pop esi
  pop ds

  push eax

  i686_EnterProtectedMode

  pop eax

  mov esp, ebp
  pop ebp
  ret

global i686_Disk_ExtendedGetDriveParams
i686_Disk_ExtendedGetDriveParams:
  push ebp
  mov ebp, esp

  i686_EnterRealMode

  push ds
  push esi

  mov ah, 0x48
  mov dl, [bp + 8]
  i686_LinearToSegOffset [bp + 12], ds, esi, si

  stc
  int 13h

  mov eax, 1
  sbb eax, 0

  pop esi
  pop ds

  push eax

  i686_EnterProtectedMode

  pop eax

  mov esp, ebp
  pop ebp
  ret

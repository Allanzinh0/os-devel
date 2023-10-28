bits 16

section .entry

extern __bss_start
extern __end
extern _init

extern Start
global entry

entry:
    cli

    ; Save boot drive
    mov [g_BootDrive], dl
    mov [g_BootPartitionOff], di
    mov [g_BootPartitionSeg], es

    mov ax, ds
    mov ss, ax
    mov sp, 0xFFF0
    mov bp, sp

    ; Switch to protected mode
    call EnableA20  ; 2 - Enable A20 gate
    call LoadGDT    ; 3 - Load GDT

    ; 4 - Set protection enable flag in CR0
    mov eax, cr0
    or al, 1
    mov cr0, eax

    ; 5 - Far jump into protected mode
    jmp dword 08h:.pmode

.pmode:
  [bits 32]

  mov ax, 0x10
  mov ds, ax
  mov ss, ax

  ; Clear bss (unitialized data)
  mov edi, __bss_start
  mov ecx, __end
  sub ecx, edi
  mov al, 0
  cld
  rep stosb

  call _init

  mov dx, [g_BootPartitionSeg]
  shl edx, 16
  mov dx, [g_BootPartitionOff]
  push edx

  xor edx, edx
  mov dl, [g_BootDrive]
  push edx
  call Start

  cli
  hlt


EnableA20:
  [bits 16]

  ; Disable keyboard
  call A20WaitInput
  mov al, KbdControllerDisableKeyboard
  out KbdControllerCommandPort, al

  ; Read control output port
  call A20WaitInput
  mov al, KbdControllerReadCtrlOutputPort
  out KbdControllerCommandPort, al

  call A20WaitOutput
  in al, KbdControllerDataPort
  push eax

  ; Write control output port
  call A20WaitInput
  mov al, KbdControllerWriteCtrlOutputPort
  out KbdControllerCommandPort, al

  call A20WaitInput
  pop eax
  or al, 2                                  ; bit 2 = A20 bit
  out KbdControllerDataPort, al

  ; Enable keyboard
  call A20WaitInput
  mov al, KbdControllerEnableKeyboard
  out KbdControllerCommandPort, al

  call A20WaitInput
  ret

A20WaitInput:
  [bits 16]
  ; Wait until status bit 2 (input buffer) is 0
  ; by reading from command port, we read status byte
  in al, KbdControllerCommandPort
  test al, 2
  jnz A20WaitInput
  ret

A20WaitOutput:
  [bits 16]
  ; Wait until status bit 1 (output buffer) is 1 so it can be read
  in al, KbdControllerCommandPort
  test al, 1
  jz A20WaitInput
  ret



LoadGDT:
  [bits 16]
  lgdt [g_GDTDesc]
  ret


KbdControllerDataPort             equ 0x60
KbdControllerCommandPort          equ 0x64
KbdControllerDisableKeyboard      equ 0xAD
KbdControllerEnableKeyboard       equ 0xAE
KbdControllerReadCtrlOutputPort   equ 0xD0
KbdControllerWriteCtrlOutputPort  equ 0xD1

g_GDT:
  ; NULL descriptor
  dq 0

  ; 32-bit code segment
  dw 0FFFFh             ; Limit (bits 0-15) = 0xFFFFF for full 32-bit range
  dw 0                  ; Base (bits 0-15) = 0x0
  db 0                  ; Base (bits 16-23)
  db 10011010b          ; Access (Present, ring 0, code segment, executable, direction 0, readable)
  db 11001111b          ; Granularity (4k pages, 32-bit pmode) + limit (bits 16-19)
  db 0                  ; Base high

  ; 32-bit data segment
  dw 0FFFFh             ; Limit (bits 0-15) = 0xFFFFF for full 32-bit range
  dw 0                  ; Base (bits 0-15) = 0x0
  db 0                  ; Base (bits 16-23)
  db 10010010b          ; Access (Present, ring 0, data segment, executable, direction 0, writable)
  db 11001111b          ; Granularity (4k pages, 32-bit pmode) + limit (bits 16-19)
  db 0                  ; Base high

  ; 16-bit code segment
  dw 0FFFFh             ; Limit (bits 0-15) = 0xFFFFF for full 32-bit range
  dw 0                  ; Base (bits 0-15) = 0x0
  db 0                  ; Base (bits 16-23)
  db 10011010b          ; Access (Present, ring 0, code segment, executable, direction 0, readable)
  db 00001111b          ; Granularity (1b pages, 16-bit pmode) + limit (bits 16-19)
  db 0                  ; Base high

  ; 16-bit data segment
  dw 0FFFFh             ; Limit (bits 0-15) = 0xFFFFF for full 32-bit range
  dw 0                  ; Base (bits 0-15) = 0x0
  db 0                  ; Base (bits 16-23)
  db 10010010b          ; Access (Present, ring 0, data segment, executable, direction 0, writable)
  db 00001111b          ; Granularity (1b pages, 16-bit pmode) + limit (bits 16-19)
  db 0                  ; Base high

g_GDTDesc:  
  dw g_GDTDesc - g_GDT - 1  ; Limit size of GDT
  dd g_GDT                  ; Address of GDT

g_BootDrive:  db 0
g_BootPartitionSeg: dw 0
g_BootPartitionOff: dw 0

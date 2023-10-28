%ifndef __PROTECTED_MODE__INC__
%define __PROTECTED_MODE__INC__

%macro i686_EnterRealMode 0
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


%macro i686_EnterProtectedMode 0
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
%macro i686_LinearToSegOffset 4
  mov %3, %1    ; Linear address to eax
  shr %3, 4
  mov %2, %4
  mov %3, %1
  and %3, 0xF
%endmacro

%endif

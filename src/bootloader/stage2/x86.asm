bits 16

section _TEXT class=CODE

;
; int 10h ah=0Eh
; args: character, page
;
global _x86_Video_WriteCharTeletype
_x86_Video_WriteCharTeletype:
    push bp
    mov bp, sp

    push bx

    mov ah, 0Eh
    mov al, [bp + 4]
    mov bh, [bp + 6]
    int 10h

    pop bx

    mov sp, bp
    pop bp
    ret

global _x86_div64_32
_x86_div64_32:
    push bp
    mov bp, sp

    push bx

    mov eax, [bp + 8]   ; eax <- Upper 32 bits of dividend
    mov ecx, [bp + 12]  ; ecx <- Divisor
    xor edx, edx
    div ecx             ; eax <- quot, edx <- remainder
    
    mov bx, [bp + 16]
    mov [bx + 4], eax

    mov eax, [bp + 4]   ; eax <- Lower 32 bits of dividend
                        ; edx <- Old remainder
    div ecx

    mov [bx], eax
    mov bx, [bp + 18]
    mov [bx], edx

    pop bx

    mov sp, bp
    pop bp
    ret


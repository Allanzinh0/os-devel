bits 16

section _TEXT class=CODE

;
; U4D
; Operation:  Unsigned 4 byte divide
; Inputs:     DX:AX Dividend
;             CX:BX Divisor
; Outputs:    DX:AX Quocient
;             CX:BX Remainder
global __U4D
__U4D:
    shl edx, 16         ; dx to upperhalf of edx
    mov dx, ax          ; edx - dividend
    mov eax, edx        ; eax - dividend
    xor edx, edx

    shl ecx, 16         ; cx to upperhalf of ecx
    mov cx, bx          ; ecx - divisor
    
    div ecx             ; eax - quot, edx - remainder
    mov ebx, edx
    mov ecx, edx
    shr ecx, 16

    mov edx, eax
    shr edx, 16

    ret

;
; U4M
; Operation:  Integer four byte multiply
; Inputs:     DX:AX integer M1
;             CX:BX integer M2
; Outputs:    DX:AX product
; Volatile:   CX, BX Destroyed
;
global __U4M
__U4M:
    shl edx, 16
    mov dx, ax
    mov eax, edx

    shl ecx, 16
    mov cx, bx

    mul ecx
    mov edx, eax
    shr edx, 16

    ret

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


; void _cdecl x86_Disk_Reset(uint8_t drive);
global _x86_Disk_Reset
_x86_Disk_Reset:
    push bp
    mov bp, sp

    mov ah, 0
    mov dl, [bp + 4]    ; dl - Disk drive
    stc
    int 13h

    mov ax, 1
    sbb ax, 0           ; 1 on success, 0 on failure

    mov sp, bp
    pop bp
    ret

; void _cdecl x86_Disk_Read(uint8_t drive, uint16_t cylinder, uint16_t sector, uint16_t head, uint8_t count, uint8_t far *dataOut);
global _x86_Disk_Read
_x86_Disk_Read:
    push bp
    mov bp, sp

    push bx
    push es

    mov dl, [bp + 4]    ; dl - Disk drive
    mov ch, [bp + 6]    ; ch - Cylinder (lower 8 bits)
    mov cl, [bp + 7]    ; bits 6-7
    shl cl, 6


    mov al, [bp + 8]
    and al, 3Fh
    or cl, al           ; cl - Sector to bits 0-5

    mov dh, [bp + 10]    ; dh - Head

    mov al, [bp + 12]   ; al - Count

    mov bx, [bp + 16]
    mov es, bx
    mov bx, [bp + 14]

    mov ah, 02h
    stc
    int 13h

    ; Set return value
    mov ax, 1
    sbb ax, 0           ; 1 on success, 0 on failure

    pop es
    pop bx

    mov sp, bp
    pop bp
    ret

; void _cdecl x86_Disk_GetDriveParams(uint8_t drive, uint8_t *driveTypeOut, uint16_t *cylindersOut, uint16_t *sectorsOut, uint16_t *headsOut);
global _x86_Disk_GetDriveParams
_x86_Disk_GetDriveParams:
    push bp
    mov bp, sp

    push es
    push bx
    push si
    push di

    mov dl, [bp + 4]    ; dl - Disk drive
    mov ah, 08h
    mov di, 0           ; es:di 0 0000:0000
    mov es, di
    stc
    int 13h

    ; Return
    mov ax, 1
    sbb ax, 0           ; 1 on success, 0 on failure

    ; Out Params
    mov si, [bp + 6]    ; Drive type from bl
    mov [si], bl

    mov bl, ch          ; Lower bits in ch
    mov bh, cl          ; Upper bits in cl (6-7)
    shl bh, 6
    mov si, [bp + 8]    ; Cylinders out
    mov [si], bx

    xor ch, ch          ; Sectors - lower 5 bits in cl
    and cl, 3Fh
    mov si, [bp + 10]
    mov [si], cx        ; Sectors out

    mov cl, dh
    mov si, [bp + 12]
    mov [si], cx        ; Heads out


    pop di
    pop si
    pop bx
    pop es

    mov sp, bp
    pop bp
    ret


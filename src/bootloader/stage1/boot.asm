org 0x7C00
bits 16

%define ENDL 0x0D, 0x0A

;
; FAT12 Header
;
jmp short start
nop

bdb_oem:                    db 'MSWIN4.1'           ; 8 bytes
bdb_bytes_per_sector:       dw 512
bdb_sectors_per_cluster:    db 1
bdb_reserved_sectors:       dw 1
bdb_fat_count:              db 2
bdb_dir_entries_count:      dw 0E0h
bdb_total_sectors:          dw 2880                 ; 2880 * 512 = 1.44 MB
bdb_media_descriptor_type:  db 0F0h                 ; F0 = 3.5" floppy disk
bdb_sectors_per_fat:        dw 9                    ; 9 sectors/fat
bdb_sectors_per_track:      dw 18
bdb_heads:                  dw 2
bdb_hidden_sectors:         dd 0
bdb_large_sector_count:     dd 0

; extended boot record
ebr_drive_number:           db 0                    ; 0x00 floppy, 0x80 hdd, useless
                            db 0                    ; reserved
ebr_signature:              db 29h
ebr_volume_id:              db 12h, 34h, 56h, 78h   ; serial number, value doesn't matter
ebr_volume_label:           db 'OS DEVELOP '        ; 11 bytes, padded with spaces
ebr_system_id:              db 'FAT12   '           ; 8 bytes, padded with spaces

;
; Code goes here
;

start:
    ; setup data segments
    mov ax, 0           ; Can't write ti ds/es directly
    mov ds, ax
    mov es, ax

    ; setup stack
    mov ss, ax
    mov sp, 0x7C00      ; Stack grows downwards from where we are loaded in memory

    ; Some BIOSes might start us at 07C0:0000 instead of 0000:7C00, make sure we are in the
    ; expected location
    push es
    push word .after
    retf
.after:
    ; read something from floppy disk
    ; BIOS should set DL to drive number
    mov [ebr_drive_number], dl

    ; print loading message
    mov si, msg_loading
    call puts

    ; read drive parameters (sectors per track and head count)
    ; instead of relying on data on formatted disk
    push es
    mov ah, 08h
    int 13h
    jc floppy_error
    pop es

    and cl, 0x3F                        ; Remove top 2 bits
    xor ch, ch
    mov [bdb_sectors_per_track], cx     ; Sector count

    inc dh
    mov [bdb_heads], dh                 ; Head count

    ; Compute LBA of root directory = reserved + fats * sectors_per_fat
    mov ax, [bdb_sectors_per_fat]
    mov bl, [bdb_fat_count]
    xor bh, bh
    mul bx                              ; ax = (fats * sectors_per_fat)
    add ax, [bdb_reserved_sectors]      ; ax = LBA of root directory
    push ax

    ; Compute size of root directory = (32 * number_of_entries) / bytes_per_sector
    mov ax, [bdb_sectors_per_fat]
    shl ax, 5                           ; ax *= 32
    xor dx, dx                          ; dx = 0
    div word [bdb_bytes_per_sector]     ; number of sectors we need to read

    test dx, dx                         ; if dx != 0, add 1
    jz .root_dir_after
    inc ax                              ; division remainder != 0, add 1

.root_dir_after:
    ; read root directory
    mov cl, al                          ; cl = number of sectors to read = size of root directory
    pop ax                              ; ax = LBA of root directory
    mov dl, [ebr_drive_number]          ; dl = drive number (we saved it previously!)
    mov bx, buffer                      ; es:bx = buffer
    call disk_read

    ; search for the kernel.bin
    xor bx, bx
    mov di, buffer

.search_stage2:
    mov si, file_stage2_bin
    mov cx, 11                          ; compare up to 11 characters
    push di
    repe cmpsb
    pop di
    je .found_stage2

    add di, 32
    inc bx
    cmp bx, [bdb_dir_entries_count]
    jl .search_stage2

    ; Stage 2 not found
    jmp stage2_not_found_error

.found_stage2:
    ; di should have the address to the entry
    mov ax, [di + 26]                   ; First logical cluster field (offset 26)
    mov [stage2_cluster], ax 
    
    ; load FAT from disk into memory
    mov ax, [bdb_reserved_sectors]
    mov bx, buffer
    mov cl, [bdb_sectors_per_fat]
    mov dl, [ebr_drive_number]
    call disk_read

    ; read stage 2 and process FAT chain
    mov bx, STAGE_2_LOAD_SEGMENT
    mov es, bx
    mov bx, STAGE_2_LOAD_OFFSET
.load_stage2_loop:
    ; Read next cluster
    mov ax, [stage2_cluster]
    ; [TODO] remove the hardcoded value
    add ax, 31                          ; First cluster = (kernel_cluster - 2) * sectors_per_cluster + start_sector
                                        ; Start sector = reserved + fats + root directory size = 1 + 18 + 134 = 33

    mov cl, 1
    mov dl, [ebr_drive_number]
    call disk_read

    add bx, [bdb_bytes_per_sector]
    
    ; Compute location of next cluster
    mov ax, [stage2_cluster]
    mov cx, 3
    mul cx
    mov cx, 2
    div cx                              ; ax = index of entry in FAT, dx = cluster mod 2

    mov si, buffer
    add si, ax
    mov ax, [ds:si]                     ; Read entry from FAT table at index ax

    or dx, dx
    jz .even

.odd:
    shr ax, 4
    jmp .next_cluster_after
.even:
    and ax, 0x0FFF
.next_cluster_after:
    cmp ax, 0x0FF8
    jae .read_finish
    
    mov [stage2_cluster], ax
    jmp .load_stage2_loop

.read_finish:
    ; jump to our stage 2
    mov dl, [ebr_drive_number]          ; boot device in dl
    mov ax, STAGE_2_LOAD_SEGMENT         ; Set segment registers
    mov ds, ax
    mov es, ax

    jmp STAGE_2_LOAD_SEGMENT:STAGE_2_LOAD_OFFSET
    
    jmp wait_key_and_reboot             ; Should never happen

    cli                                 ; Disable interrupts, this way we can't get out of "halt" state
    hlt

;
; Error handlers
;

floppy_error:
    ; print message
    mov si, msg_read_failed
    call puts
    jmp wait_key_and_reboot

stage2_not_found_error:
    ; print message
    mov si, msg_stage2_not_found
    call puts
    jmp wait_key_and_reboot

wait_key_and_reboot:
    mov ah, 0
    int 16h             ; Wait for keypress
    jmp 0FFFFh:0        ; Jump to beginning of BIOS, should reboot

.halt:
    cli                 ; Disable interrupts, this way we can't get out of "halt" state
    hlt

;
; Print a string to the screen
; Params:
;   - ds:si points to string
puts:
    ; save registers we will modify
    push si
    push ax

.loop:
    lodsb               ; Loads next character in al
    or al, al           ; verify if next character is null
    jz .done
    
    mov ah, 0x0e
    mov bh, 0
    int 0x10

    jmp .loop
.done:
    pop ax
    pop si
    ret

;
; Disk routines
;

;
; Converts an LBA address to a CHS address
; Parameters:
;  - ax: LBA address
; Returns:
;  - cx (bits 0-5): sector number
;  - cx (bits 6-15): cylinder
;  - dh: head

lba_to_chs:
    push ax
    push dx

    xor dx, dx                          ; dx = 0
    div word [bdb_sectors_per_track]    ; ax = LBA / SectorsPerTrack
                                        ; dx = LBA % SectorsPerTrack

    inc dx                              ; dx = (LBA % SectorsPerTrack + 1) = sector
    mov cx, dx                          ; cx = sector


    xor dx, dx                          ; dx = 0
    div word [bdb_heads]                ; ax = (LBA / SectorsPerTrack) / Heads = cylinder
                                        ; dx = (LBA / SectorsPerTrack) % Heads = head
    mov dh, dl                          ; dh = head
    mov ch, al                          ; ch = cylinder (lower 8 bits)
    shl ah, 6
    or cl, ah                           ; put upper 2 bit of cylinder in CL

    pop ax
    mov dl, al                          ; Restore DL
    pop ax
    ret

;
; Reads sectors from a disk
; Parameters:
;  - ax: LBA address
;  - cl: number of sectors to read (up to 128)
;  - dl: drive number
;  - ex:bx: memory address where to store read data
;
disk_read:
    push ax                             ; Save registers we will modify
    push bx
    push cx
    push dx
    push di

    push cx                             ; temporarily save CL (number of sectors to read)
    call lba_to_chs                     ; compute CHS
    pop ax                              ; AL = number of sectors to read

    mov ah, 02h
    mov di, 3                           ; retry count

.retry:
    pusha                               ; Save all registers, we don't know what bios modifies
    stc                                 ; Set carry flag, some BIOS'es don't set it
    int 13h                             ; Carry flag cleared = success
    jnc .done                           ; jump if carry not set

    ; read failed
    popa
    call disk_reset

    dec di
    test di, di
    jnz .retry

.fail:
    ; all attempts are exhausted
    jmp floppy_error

.done:
    popa

    pop di                             ; Restore registers modified
    pop dx
    pop cx
    pop bx
    pop ax
    ret

;
; Resets disk controller
; Parameters:
;  - dl: Drive number
disk_reset:
    pusha
    mov ah, 0
    stc
    int 13h
    jc floppy_error
    popa
    ret


msg_loading: db 'Loading...', ENDL, 0
msg_read_failed: db 'Read from disk failed!', ENDL, 0
msg_stage2_not_found: db 'STAGE2.BIN file not found!', ENDL, 0
file_stage2_bin: db 'STAGE2  BIN'
stage2_cluster: dw 0

STAGE_2_LOAD_SEGMENT: equ 0x0
STAGE_2_LOAD_OFFSET: equ 0x500
times 510 - ($ - $$) db 0
dw 0AA55h

buffer:
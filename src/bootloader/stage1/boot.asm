[bits 16]

%define ENDL 0x0D, 0x0A
%define fat12 12
%define fat16 16
%define fat32 32
%define ext2 2
;
; FAT12 Header
;
section .fsjump
  jmp short start
  nop

section .fsheaders
%if (FILESYSTEM==fat12) || (FILESYSTEM==fat16) || (FILESYSTEM==fat32)
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
%if (FILESYSTEM==fat32)
  fat32_sectors_per_fat:      dd 0
  fat32_flags:                dw 0
  fat32_vertion_number:       dw 0
  fat32_rootdir_cluster:      dd 0
  fat32_fsinfo_sector:        dw 0
  fat32_backup_boot_sector:   dw 0
  fat32_reserved              times 12 db 0
%endif

  ebr_drive_number:           db 0                    ; 0x00 floppy, 0x80 hdd, useless
                              db 0                    ; reserved
  ebr_signature:              db 29h
  ebr_volume_id:              db 12h, 34h, 56h, 78h   ; serial number, value doesn't matter
  ebr_volume_label:           db 'OS DEVELOP '        ; 11 bytes, padded with spaces
  ebr_system_id:              db 'FAT12   '           ; 8 bytes, padded with spaces

%endif

;
; Code goes here
;
section .entry
  global start
  start:
    ; Move partition entry from MBR
    mov ax, PARTITION_ENTRY_SEGMENT
    mov es, ax
    mov di, PARTITION_ENTRY_OFFSET
    mov cx, 16
    rep movsb

    mov di, ds

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

    ; Check extensions present
    mov ah, 0x41
    mov bx, 0x55AA
    stc
    int 13h

    jc .no_disk_extensions
    cmp bx, 0xAA55
    jne .no_disk_extensions

    ; Extensions are present
    mov byte [have_extensions], 1
    jmp .after_disk_extensions_check

  .no_disk_extensions:
    mov byte [have_extensions], 0

  .after_disk_extensions_check:
    ; Load stage 2
    mov si, stage2_location

    mov ax, STAGE_2_LOAD_SEGMENT
    mov es, ax

    mov bx, STAGE_2_LOAD_OFFSET
  .loop:
    mov eax, [si]
    add si, 4
    mov cl, [si]
    inc si

    cmp eax, 0
    je .read_finish

    call disk_read

    xor ch, ch
    shl cx, 5
    mov di, es
    add di, cx
    mov es, di

    
    jmp .loop

  .read_finish:
    ; Jump to stage 2
    mov dl, [ebr_drive_number]
    mov si, PARTITION_ENTRY_OFFSET
    mov di, PARTITION_ENTRY_SEGMENT

    mov ax, STAGE_2_LOAD_SEGMENT
    mov ds, ax
    mov es, ax

    jmp STAGE_2_LOAD_SEGMENT:STAGE_2_LOAD_OFFSET
    
    jmp wait_key_and_reboot             ; Should never happen

    cli                                 ; Disable interrupts, this way we can't get out of "halt" state
    hlt

;
; Error handlers
;
section .text
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
;  - eax: LBA address
;  - cl: number of sectors to read (up to 128)
;  - dl: drive number
;  - ex:bx: memory address where to store read data
;
  disk_read:
    push eax                             ; Save registers we will modify
    push bx
    push cx
    push dx
    push si
    push di

    cmp byte [have_extensions], 1
    jne .no_disk_extensions

    ; With extensions
    mov [extensions_dap.lba], eax
    mov [extensions_dap.segment], es
    mov [extensions_dap.offset], bx
    mov [extensions_dap.count], cl

    mov ah, 0x42
    mov si, extensions_dap
    mov di, 3
    jmp .retry

  .no_disk_extensions:
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
    pop si
    pop dx
    pop cx
    pop bx
    pop eax
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

section .rodata
  msg_read_failed: db 'Read failed!', ENDL, 0
  msg_stage2_not_found: db 'STAGE2.BIN file not found!', ENDL, 0
  file_stage2_bin: db 'STAGE2  BIN'

section data
  have_extensions: db 0
  extensions_dap:
    .size:  db 10h
            db 0
    .count: dw 0
    .offset: dw 0
    .segment: dw 0
    .lba: dq 0

  STAGE_2_LOAD_SEGMENT: equ 0x0
  STAGE_2_LOAD_OFFSET: equ 0x500

  PARTITION_ENTRY_SEGMENT: equ 0x2000
  PARTITION_ENTRY_OFFSET: equ 0x0

  global stage2_location
  stage2_location: times 30 db 0


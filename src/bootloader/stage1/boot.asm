[bits 16]

%include "config.inc"

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

  ; The MBR will pass us the following information
  ; DL = drive number
  ; DS:SI = partition table entry that we booted from
  start:
    ;
    ; Copy partition entry from MBR
    ;
    mov ax, STAGE1_SEGMENT
    mov es, ax
    mov di, partition_entry
    mov cx, 16
    rep movsb

    ;
    ; Relocate itself
    ;
    mov ax, 0
    mov ds, ax
    mov si, 0x7c00
    mov di, STAGE1_OFFSET
    mov cx, 512
    rep movsb

    jmp STAGE1_SEGMENT:.relocated

  .relocated:
    ;
    ; Initialization
    ;

    ; setup data segments
    mov ax, STAGE1_SEGMENT  ; Can't write ti ds/es directly
    mov ds, ax
    mov es, ax

    ; setup stack
    mov ss, ax
    mov sp, STAGE1_OFFSET   ; Stack grows downwards from where we are loaded in memory

    ; Save drive number from dl
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
    ;
    ; Load boot_table_lba which contains the boot table
    ;
    mov eax, [boot_table_lba]
    mov cx, 1
    mov bx, boot_table
    call disk_read

    ;
    ; Parse boot table and load
    ;
    mov si, boot_table

    ; Parse entry point
    mov eax, [si]
    mov [entry_point], eax
    add si, 4

  .readloop:
    ; Read until lba=0
    cmp dword [si + boot_table_entry.lba], 0
    je .endreadloop

    mov eax, [si + boot_table_entry.lba]
    mov bx, [si + boot_table_entry.load_seg]
    mov es, bx
    mov bx, [si + boot_table_entry.load_off]
    mov cx, [si + boot_table_entry.count]
    call disk_read

    ; Go to next entry
    add si, boot_table_entry_size
    jmp .readloop
    
  .endreadloop:
    ; Jump to stage 2
    mov dl, [ebr_drive_number]
    mov di, partition_entry             ; ES:DI points to partition entry

    mov ax, [entry_point.seg]
    mov ds, ax

    ; Do far jump
    push ax
    push word [entry_point.off]
    retf

  .never:
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
    mov [extensions_dap.count], cx

    mov ah, 0x42
    mov si, extensions_dap
    mov di, 3

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

  .done:
    popa
    jmp .function_end

  .no_disk_extensions:
    mov esi, eax                        ; Save lba to esi
    mov di, cx                          ; Save number of sectors to di
    
  .outer_loop:
    mov eax, esi
    call lba_to_chs                     ; compute CHS
    mov al, 1

    push di
    mov di, 3                           ; retry count
    mov ah, 02h

  .inner_retry:
    pusha                               ; Save all registers, we don't know what bios modifies
    stc                                 ; Set carry flag, some BIOS'es don't set it
    int 13h                             ; Carry flag cleared = success
    jnc .inner_done                           ; jump if carry not set

    ; read failed
    popa
    call disk_reset

    dec di
    test di, di
    jnz .inner_retry

  .fail:
    ; all attempts are exhausted
    jmp floppy_error

  .inner_done:
    popa
    pop di

    cmp di, 0
    je .function_end

    inc esi
    dec di
    mov ax, es
    add ax, 512 / 16
    mov es, ax

    jmp .outer_loop

  .function_end:
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

section .data
  extensions_dap:
    .size:  db 10h
            db 0
    .count: dw 0
    .offset: dw 0
    .segment: dw 0
    .lba: dq 0

  struc boot_table_entry
    .lba            resd 1
    .load_off       resw 1
    .load_seg       resw 1
    .count          resw 1
  endstruc

section .data
  global boot_table_lba
  boot_table_lba:   dd 0

section .bss
  have_extensions:  resb 1
  buffer:           resb 512
  partition_entry:  resb 16
  boot_table:       resb 512
  entry_point:
    .off            resw 1
    .seg            resw 1

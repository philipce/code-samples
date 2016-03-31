;-------------------------------------------------------------------------
; Interrupt Service Routines
;   - Description: ISRs referenced by vector table in clib.s
;-------------------------------------------------------------------------

;---------------------------------
; YKResetISR:
;   - Terminate running program
;   - No context save necessary
;---------------------------------
YKResetISR:
    call YKResetHandler

;---------------------------------
; YKTickISR:
;   - Update tick count
;---------------------------------
YKTickISR:

    ; Save context
    push ax
    push bx
    push cx
    push dx
    push bp
    push si
    push di
    push ss
    push ds
    push es

    ; Inform OS that ISR has begun
    call YKEnterISR

    ; Enable interrupts
    sti

    ; Call handler
    call YKTickHandler

    ; Disable interrupts
    cli

    ; Inform PIC that the handler is finished
    mov	al, 0x20 ; load nonspecific EOI value (0x20) into register al
	out	0x20, al ; write EOI to PIC (port 0x20)

    ; Inform OS that ISR has finished
    call YKExitISR

    ; Restore registers
    pop es
    pop ds
    pop ss
    pop di
    pop si
    pop bp
    pop dx
    pop cx
    pop bx
    pop ax

    ; Return and pop IP, CS, FLAGS
    iret

;---------------------------------
; YKKeyISR:
;   - Handle keyboard input
;---------------------------------
YKKeyISR:

    ; Save context
    push ax
    push bx
    push cx
    push dx
    push bp
    push si
    push di
    push ss
    push ds
    push es

    ; Inform OS that ISR has begun
    call YKEnterISR

    ; Enable interrupts
    sti

    ; Call handler
    call YKKeyHandler

    ; Disable interrupts
    cli

    ; Inform PIC that the handler is finished
    mov	al, 0x20 ; load nonspecific EOI value (0x20) into register al
	out	0x20, al ; write EOI to PIC (port 0x20)

    ; Inform OS that ISR has finished
    call YKExitISR

    ; Restore registers
    pop es
    pop ds
    pop ss
    pop di
    pop si
    pop bp
    pop dx
    pop cx
    pop bx
    pop ax

    ; Return and pop IP, CS, FLAGS
    iret

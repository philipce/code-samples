;-------------------------------------------------------------------------
; YAK Kernel Code (Assembly)
;   - Description: Assembly library of YAK kernel functions
;-------------------------------------------------------------------------

;---------------------------------
; YKDispatcher
;   - Run the highest priority task
;   - Task to dispatch comes from the head of the ready queue
;   - Check interrupt nesting level to determine whether call is from ISR or task code
;   - Restore context
;   - Resume execution
;---------------------------------    
YKDispatcher:
    ; Save old BP and update new BP
    push bp
    mov bp, sp

    ; If dispatching from interrupt, context already saved--go to part 2
    cmp word[YKInterruptNestLevel], 0
    jne YKDispatcher2

    ; Save old task's registers on stack
    pushf
    push cs
    sub sp, 2 ; save a space for IP
    push ax
    push bx

    ; Save old task's IP on stack
    mov bx, [bp] ; put old BP in BX
    mov ax, [bx+2] ; put return address in AX
    mov  bx, sp
    add  bx, 4 ; address in BX is the space created for IP
    mov [bx], ax ; put IP on the stack
    
    ; Continue saving old task's registers on stack
    push cx
    push dx
    push bp
    push si
    push di
    push ss
    push ds
    push es

    ; Save old task's stack pointer in TCB
    mov bx, [YKCurrentTask]
    mov [bx], sp

YKDispatcher2:

    ; Switch YKCurrentTask to point to YKReadyHead
    mov bx, [YKReadyHead]
    mov [YKCurrentTask], bx

    ; Switch to new task's stack
    mov bx, [YKCurrentTask]
    mov sp, [bx]

    ; Restore registers (except CS, FLAGS, and IP)
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

    ; Note: iret will pop IP, CS, FLAGS in that order
    iret

;---------------------------------
; YKEnterMutexAsm
;   - Clear interrupts
;   - Return 1 if interrupts were enabled on entering, else 0
;---------------------------------
YKEnterMutex:
    
    ; Save CX
    push cx

    ; Load CX with index of IF in FLAGS
    mov cx, 9

    ; Load IF into AX
    pushf
    pop ax
    and ax, 0x0200 ; mask to get just IF
    shr ax, cl   

    ; Clear interrupts
    cli

    ; Restore CX
    pop cx

    ret

;---------------------------------
; YKExitMutexAsm
;   - Enable interrupts
;---------------------------------
YKExitMutex:
    sti

    ret

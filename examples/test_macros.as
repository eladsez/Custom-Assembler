

macro HELLO
    mov r1, r2
    add r1, #5
macroend

macro ADD_THREE
    inc r3
    inc r3
    inc r3
macroend

macro TWO
    prn r3
macroend

macro EMPTY_MACRO
macroend

; Macro calls below
HELLO

ADD_THREE

EMPTY_MACRO

; Some normal instruction
jmp END

; Call macro again to test reuse
HELLO

END:
    TWO

LABEL:  HELLO

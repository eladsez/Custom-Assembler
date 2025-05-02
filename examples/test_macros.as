

macro HELLO
    mov r1, r2
    add r1, #5
macroend

macro ADD_THREE
    inc r3
    inc r3
    inc r3
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
    stop 

LABEL:  HELLO

; should fail because d is unexpexted
macro ADD_TWO
    inc r3
    inc r1
macroend d

; should fail because ADD_ONE is unexpeted
macro g ADD_ONE
    inc r3
    inc r1
macroend

ADD_ONE

; should fail because bbb is unexpeted
bbb macro ADD
    inc r3
macroend

ADD

g

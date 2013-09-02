; Disassembled with spcdasm
; Source filename: ../DSPcode.bin
; Origin: $0002
; Input length: 16

p0002: MOV  $f2 , A        ; choose dsp register address (A)
p0004: CMP  A , $f4        
p0006: BNE  p0004          ; wait until Port0 equals A
p0008: MOV  $f3 , $f5      ; now we can copy the value in Port1 
                           ; to the selected dsp address
p000b: MOV  $f4 , A        ; say it's done by setting Port0 to the
                           ; dsp address
p000d: INC  A              ; increment A so we will expect the next address
p000e: BPL  p0002          ; while < 128, jump to p0002

; Jumps right after init code in rom, by underflowing
; the 16 bit program counter
p0010: BRA  pffc9          ; 2fb7


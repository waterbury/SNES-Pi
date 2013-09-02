; Disassembled with spcdasm
; Source filename: ../Bootcode.bin
; Origin: $0000
; Input length: 77
p0000: MOV  $0 , #$0       ; $0001: ram $0000
p0003: MOV  $1 , #$0       ; $0004: ram $0001
p0006: MOV  $fc , #$ff     ; $0007: Timer2
p0009: MOV  $fb , #$ff     ; $000a: Timer1
p000c: MOV  $fa , #$4f     ; $000d: Timer0
p000f: MOV  $f1 , #$31     ; $0010: SPC Control reg
p0012: MOV  X , #$53       ;   
p0014: MOV  $f4 , X        ; 
p0016: MOV  A , $f4        ; 
p0018: CMP  A , #$0        ; $0019: Port0
p001a: BNE  p0016          ; 
p001c: MOV  A , $f5        ; 
p001e: CMP  A , #$0        ; $001f: Port1
p0020: BNE  p001c          ; 
p0022: MOV  A , $f6        ; 
p0024: CMP  A , #$0        ; $0024: Port2
p0026: BNE  p0022          ; 
p0028: MOV  A , $f7        ; 
p002a: CMP  A , #$0        ; $002b: Port3
p002c: BNE  p0028          ; 
p002e: MOV  A , $fd        ; 
p0030: MOV  A , $fe        ; 
p0032: MOV  A , $ff        ; 
p0034: MOV  $f2 , #$6c     ; point to flg register
p0037: MOV  $f3 , #$0      ; $0038: DSP FLG register
p003a: MOV  $f2 , #$4c     ; point to kon register
p003d: MOV  $f3 , #$0      ; $003e: DSP KON register
p0040: MOV  $f2 , #$7f     ; $0041: SPC dsp reg addr.
p0043: MOV  X , #$f5       ; $0044: SPC stack pointer
p0045: MOV  SP , X         ;
p0046: MOV  A , #$ff       ; $0047: SPC A register 
p0048: MOV  Y , #$0        ; $0049: SPC Y register
p004a: MOV  X , #$0        ; $004b: SPC X register
p004c: RETI                ; 

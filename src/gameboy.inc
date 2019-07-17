;
; gameboy.inc
; Utilities for the hardware
;

include "hardware.inc"

vBgBank0 EQU _VRAM
vBgBank1 EQU _VRAM + $800
vBgBank2 EQU _VRAM + $1000
vSpriteData EQU _VRAM

SetData16: macro
    ld a, LOW(\2)
    ld [\1], a
    ld a, HIGH(\2)
    ld [(\1)+1], a
endm

SetData8: macro
    ld a, \2
    ld [\1], a
endm

SetDataFixed168: macro
    SetData8 \1, \3
    SetData16 (\1)+1, \2
endm

SetDataFixed88: macro
    SetData8 \1, \3
    SetData8 (\1)+1, \2
endm
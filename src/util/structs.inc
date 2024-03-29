
; MIT License
;
; Copyright (c) 2018-2019 Eldred Habert
; Originally hosted at https://github.com/ISSOtm/rgbds-structs
;
; Permission is hereby granted, free of charge, to any person obtaining a copy
; of this software and associated documentation files (the "Software"), to deal
; in the Software without restriction, including without limitation the rights
; to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
; copies of the Software, and to permit persons to whom the Software is
; furnished to do so, subject to the following conditions:
;
; The above copyright notice and this permission notice shall be included in all
; copies or substantial portions of the Software.
;
; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
; IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
; FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
; AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
; LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
; OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
; SOFTWARE.


; Yes I need this macro, for version checking.
; What did you expect from a macro pack that mostly relies on code generation at compile time?
; Also this is your last chance to turn back. It doesn't get any better.

; strreplace variable_name, original_char, new_char
strreplace: MACRO
DOT_POS = STRIN("{\1}", \2)
    IF DOT_POS != 0
TMP equs STRCAT(STRSUB("{\1}", 1, DOT_POS + (-1)), STRCAT(\3, STRSUB("{\1}", DOT_POS + 1, STRLEN("{\1}") - DOT_POS)))
        PURGE \1
\1 equs "{TMP}"
        PURGE TMP
        strreplace \1, \2, \3
    ENDC
    IF DEF(DOT_POS)
        PURGE DOT_POS
    ENDC
ENDM


; rgbds_structs_version version_string
; Call with the expected version string to ensure you're using a compatible version
; Example: rgbds_structs_version 1.0.0
rgbds_structs_version: MACRO
CURRENT_VERSION equs "1,2,0"
EXPECTED_VERSION equs "\1"
    strreplace EXPECTED_VERSION, ".", "\,"
check_ver: MACRO
    IF \1 != \4 || \2 > \5 || \3 > \6
        PURGE EXPECTED_VERSION
    ENDC
ENDM
CHECK_VER_CALL equs "check_ver {EXPECTED_VERSION},{CURRENT_VERSION}"
    CHECK_VER_CALL
    IF !DEF(EXPECTED_VERSION)
        strreplace CURRENT_VERSION, "\,", "."
        FAIL "RGBDS-structs version \1 is required, which is incompatible with current version {CURRENT_VERSION}"
    ENDC
    PURGE CHECK_VER_CALL
    PURGE check_ver
    PURGE CURRENT_VERSION
    PURGE EXPECTED_VERSION
ENDM


; struct struct_name
; Begins a struct declaration
struct: MACRO
    IF DEF(NB_FIELDS)
        FAIL "Please close struct definitions using `end_struct`"
    ENDC

STRUCT_NAME equs "\1"

NB_FIELDS = 0
    RSRESET
ENDM

; end_struct
; Ends a struct declaration
end_struct: MACRO
; Set nb of fields
STRUCT_NB_FIELDS equs "{STRUCT_NAME}_nb_fields"
STRUCT_NB_FIELDS = NB_FIELDS
    PURGE STRUCT_NB_FIELDS

; Set size of struct
STRUCT_SIZEOF equs "sizeof_{STRUCT_NAME}"
STRUCT_SIZEOF RB 0
    PURGE STRUCT_SIZEOF

    PURGE NB_FIELDS
    PURGE STRUCT_NAME
ENDM


; field_name_from_id field_id
; For internal use, please do not use externally
field_name_from_id: MACRO
FIELD_ID_STR equs "{\1}"
STRUCT_FIELD equs STRCAT("{STRUCT_NAME}_field", STRSUB("{FIELD_ID_STR}", 2, STRLEN("{FIELD_ID_STR}") - 1))
STRUCT_FIELD_NAME equs "{STRUCT_FIELD}_name"
STRUCT_FIELD_TYPE equs "{STRUCT_FIELD}_type"
STRUCT_FIELD_NBEL equs "{STRUCT_FIELD}_nb_el" ; Number of elements
STRUCT_FIELD_SIZE equs "{STRUCT_FIELD}_size" ; sizeof(type) * nb_el
ENDM


; new_field nb_elems, rs_type, field_name
; For internal use, please do not use externally
new_field: MACRO
    IF !DEF(STRUCT_NAME) || !DEF(NB_FIELDS)
        FAIL "Please start defining a struct, using `define_struct`"
    ENDC

    field_name_from_id NB_FIELDS
; Set field name
STRUCT_FIELD_NAME equs "\"\3\""
    PURGE STRUCT_FIELD_NAME

; Set field offset
STRUCT_FIELD \2 (\1)
; Alias this in a human-comprehensive manner
STRUCT_FIELD_NAME equs "{STRUCT_NAME}_\3"
STRUCT_FIELD_NAME = STRUCT_FIELD

; Calculate field size
CURRENT_RS RB 0
STRUCT_FIELD_SIZE = CURRENT_RS - STRUCT_FIELD

; Set properties
STRUCT_FIELD_NBEL = \1
STRUCT_FIELD_TYPE equs STRSUB("\2", 2, 1)

    PURGE FIELD_ID_STR
    PURGE STRUCT_FIELD
    PURGE STRUCT_FIELD_NAME
    PURGE STRUCT_FIELD_TYPE
    PURGE STRUCT_FIELD_NBEL
    PURGE STRUCT_FIELD_SIZE
    PURGE CURRENT_RS

NB_FIELDS = NB_FIELDS + 1
ENDM

; bytes nb_bytes, field_name
; Defines a field of N bytes
bytes: MACRO
    new_field \1, RB, \2
ENDM

; words nb_words, field_name
; Defines a field of N*2 bytes
words: MACRO
    new_field \1, RW, \2
ENDM

; longs nb_longs, field_name
; Defines a field of N*4 bytes
longs: MACRO
    new_field \1, RL, \2
ENDM


; dstruct struct_type, var_name[, ...]
; Allocates space for a struct in memory
; If no further arguments are supplied, the space is simply allocated (using `ds`)
; Otherwise, the data is written to memory using the appropriate types
; For example, a struct defined with `bytes 1, Field1` and `words 3, Field2` would have four extra arguments, one byte then three words.
dstruct: MACRO
NB_FIELDS equs "\1_nb_fields"
    IF !DEF(NB_FIELDS)
        FAIL "Struct \1 isn't defined!"
    ENDC
STRUCT_NAME equs "\1" ; Target this struct for `field_name_from_id`
VAR_NAME    equs "\2"

VAR_NAME:: ; Declare the struct's root

FIELD_ID = 0
    REPT NB_FIELDS

        field_name_from_id FIELD_ID
FIELD_NAME equs STRCAT("{VAR_NAME}_", STRUCT_FIELD_NAME)
FIELD_NAME::
        IF _NARG == 2 ; RAM definition, no data
            ds STRUCT_FIELD_SIZE
        ELSE
TMP equs STRCAT("\{", STRCAT("{STRUCT_FIELD_TYPE}", "\}")) ; Temp var for double deref because "{{STRUCT_FIELD_TYPE}}" is a syntax error
DATA_TYPE equs STRCAT("D", TMP)
            PURGE TMP
SHIFT_FIELDS equs ""
            REPT STRUCT_FIELD_NBEL
                DATA_TYPE \3
                SHIFT
                ; Stupid hack because RGBDS saves the macro arguments when entering REPT blocks
TMP equs "{SHIFT_FIELDS}\n\tSHIFT"
                PURGE SHIFT_FIELDS
SHIFT_FIELDS equs "{TMP}"
                PURGE TMP
            ENDR
            SHIFT_FIELDS
            PURGE SHIFT_FIELDS
            PURGE DATA_TYPE
        ENDC

        ; Clean up vars for next iteration
        PURGE FIELD_ID_STR
        PURGE STRUCT_FIELD
        PURGE STRUCT_FIELD_NAME
        PURGE STRUCT_FIELD_TYPE
        PURGE STRUCT_FIELD_NBEL
        PURGE STRUCT_FIELD_SIZE
        PURGE FIELD_NAME

FIELD_ID = FIELD_ID + 1
    ENDR


    ; Define variable's properties from struct's
\2_nb_fields = NB_FIELDS
sizeof_\2 = sizeof_\1


    ; Clean up
    PURGE NB_FIELDS
    PURGE STRUCT_NAME
    PURGE VAR_NAME
    PURGE FIELD_ID
ENDM


; dstructs struct_type, var_name
; Allocates space for an array of structs in memory
; Each struct will have the index appended to its name **as hex**
; (for example: `dstructs 32, NPC, wNPC` will define wNPC0, wNPC1, and so on until wNPC1F)
; This is a limitation because RGBASM does not provide an easy way to get the decimal representation of a number
; Does not support data declarations because I think each struct should be defined individually for that purpose
dstructs: MACRO
STRUCT_ID = 0
    REPT \1
STRUCT_DEF equs STRCAT("dstruct \2, \3", STRSUB("{STRUCT_ID}", 2, STRLEN("{STRUCT_ID}") - 1))
        STRUCT_DEF
        PURGE STRUCT_DEF
STRUCT_ID = STRUCT_ID + 1
    ENDR
    PURGE STRUCT_ID
ENDM

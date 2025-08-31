#include "version.h"

.section .oemhead.text,"ax"
.incbin "oemhead/oemhead.b"
.asciz SUB_VER

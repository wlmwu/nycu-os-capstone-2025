.section ".text"
.global _start
_start:
    mov x0, 0
1:
    add x0, x0, 1
    svc 0           // SuperVisor Call (P. 748). To generates a synchronous exception.
    cmp x0, 5
    blt 1b
1:
    b 1b
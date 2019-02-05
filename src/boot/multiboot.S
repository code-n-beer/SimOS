.section .multiboot
.align 8 /* multiboot requires the header to be 64-bit aligned */

header_start:
    .long 0xe85250d6                    /* multiboot 2 magic */
    .long 0                             /* arch: i386 */
    .long header_end - header_start     /* length */
    .long 0x100000000 - (0xe85250d6 + 0 + (header_end - header_start)) /* checksum */

    /* end tag */
    .word 0                             /* type */
    .word 0                             /* flags */
    .long 8                             /* size */
header_end:
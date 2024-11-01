#pragma once

#define P1_B 0x20 /* base address for PIC1's io */
#define P1_COM P1_B /* PIC1 command */
#define P1_DAT (P1_COM + 1) /* PIC1 data */

#define P2_B 0xA0 /* base address for PIC2's io */
#define P2_COM P2_B /* PIC2 command */
#define P2_DAT (P2_COM + 1) /* PIC2 data */

#define P1_I 0x20 /* start of PIC1's interrupts */
#define P2_I (P1_I + 0x08) /* start of PIC2's interrupts */

void pic_init();
void pic_ack(unsigned int i);
void irq_m_free(unsigned char line);

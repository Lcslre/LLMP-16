#ifndef LLMP16_PIC_H
#define LLMP16_PIC_H

#include <stdint.h>
#include <stdbool.h>
#include "llmp16.h"

/*
 * Programmable Interrupt Controller (PIC) pour LLMP16
 * ---------------------------------------------------
 * Gère jusqu'à 8 lignes d'interruptions matérielles.
 *
 * IO Port 8 (PIC_CMD) :
 *   Reg 0 (R0) : IMR      (bits 0..7, 1 = masquée, 0 = non masquée)
 *   Reg 1 (R1) : ISR      (bits 0..7, 1 = en attente, 0 = pas en attente)
 *   Reg 2 (R2) : IRR      (bits 0..7, 1 = en attente, 0 = pas en attente)
 *   Reg 3 (R3) : EOI      (acknowledge, bits 0..7)
 *   Reg 4 (R4) : INT_BASE (Adresse du tableau d'interruption en ROM)
 *
 */

#define LLMP_PIC_PORT       8  
#define LLMP_PIC_MAX_IRQ    16   /* Nombre de lignes gérées */
#define LLMP_PIC_IMR        0 
#define LLMP_PIC_ISR        1 
#define LLMP_PIC_IRR        2 
#define LLMP_PIC_EOI        3 
#define LLMP_PIC_BASE       4 

/* Structure interne du PIC */
typedef struct {
    uint16_t IMR;                /* Masque des IRQs (1 = masquée) */
    uint16_t ISR;               /* Registre d'état des IRQs (1 = en attente) */
    uint16_t IRR;               /* Registre d'attente des IRQs (1 = en attente) */
    uint16_t EOI;                /* Registre d'acknowledge des IRQs */
    uint16_t INT_BASE;
} llmp16_pic_t;


void llmp16_pic_init(llmp16_pic_t *pic);
void llmp16_pic_raise_irq(llmp16_pic_t *pic, uint8_t irq_line);
void llmp16_pic_end_of_interrupt(llmp16_pic_t *pic, uint16_t EOI);
uint16_t llmp16_pic_get_highest_pending(llmp16_pic_t *pic);
void llmp16_pic_readIO(llmp16_t *cpu, llmp16_pic_t *pic);
void llmp16_pic_writeIO(llmp16_t *cpu, llmp16_pic_t *pic);
void llmp16_pic_update(llmp16_t *cpu, llmp16_pic_t *pic);



#endif // LLMP16_PIC_H

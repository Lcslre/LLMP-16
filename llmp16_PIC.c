#include "llmp16_PIC.h"

void llmp16_pic_init(llmp16_pic_t *pic)
{
    pic->EOI = 0x0000;
    pic->IMR = 0xFFFF;
    pic->IRR = 0x0000;
    pic->ISR = 0x0000;
}

// irq_line : entre 0 et 15 pour dire quelle intéruption doit être levée
void llmp16_pic_raise_irq(llmp16_pic_t *pic, uint8_t irq_line)
{
    if(irq_line < 16 && !(pic->IMR & (1 << irq_line))) pic->IRR |= (1 << irq_line);
}

void llmp16_pic_end_of_interrupt(llmp16_pic_t *pic, uint16_t EOI)
{
    
    pic->ISR &= ~EOI;
}

uint16_t llmp16_pic_get_highest_pending(llmp16_pic_t *pic) {
    for (int i = 0; i < 16; i++) {
        if ((pic->IRR & (1 << i)) && !(pic->IMR & (1 << i)) && !(pic->ISR & (1 << i))) {
            return i;
        }
    }
    return 0xFFFF; // Aucun IRQ disponible
}

void llmp16_pic_readIO(llmp16_t *cpu, llmp16_pic_t *pic)
{
    pic->EOI = cpu->IO[LLMP_PIC_PORT][LLMP_PIC_EOI];
    pic->IMR = cpu->IO[LLMP_PIC_PORT][LLMP_PIC_IMR];
    pic->INT_BASE = cpu->IO[LLMP_PIC_PORT][LLMP_PIC_BASE];
}

void llmp16_pic_writeIO(llmp16_t *cpu, llmp16_pic_t *pic)
{
    cpu->IO[LLMP_PIC_PORT][LLMP_PIC_IRR] = pic->IRR;
    cpu->IO[LLMP_PIC_PORT][LLMP_PIC_ISR] = pic->ISR;
}

void llmp16_pic_update(llmp16_t *cpu, llmp16_pic_t *pic)
{
    llmp16_pic_readIO(cpu, pic);

    // Traiter l'EOI si défini (pas 0x0000)
    if (pic->EOI != 0x0000) {
        llmp16_pic_end_of_interrupt(pic, pic->EOI);
        pic->EOI = 0x0000; // Reset après traitement
    }

    // Gérer la prochaine interruption en attente
    uint16_t irq = llmp16_pic_get_highest_pending(pic);
    if (irq != 0xFFFF) {
        pic->IRR &= ~(1 << irq); // consomme l'IRQ
        pic->ISR |= (1 << irq);  // marque comme en service

        cpu->int_vector_pending = pic->INT_BASE + irq;
        cpu->int_pending = 1;
    }

    llmp16_pic_writeIO(cpu, pic);
}

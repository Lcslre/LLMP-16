#ifndef LLMP16_DMA_H
#define LLMP16_DMA_H

#include <stdint.h>
#include <stdbool.h>
#include "llmp16.h"

/*
 * Contrôleur DMA pour LLMP16
 * ---------------------------
 * Permet de transférer des blocs mémoire (RAM↔RAM, RAM↔VRAM, DISK↔RAM) sans CPU.
 *
 * IO Port 7 (DMA) :
 *   Reg 0 (R0) : SRC_ADDR          (bits 15..0)
 *   Reg 1 (R1) : DST_ADDR          (bits 15..0)
 *   Reg 2 (R2) : COUNT             (nombre d’octets à transférer, bits 15..0)
 *   Reg 3 (R3) : CTRL              (bit 0 = DMA_ENABLE, bit 1 = IRQ_ENABLE)
 *   Reg 4 (R4) : STAT              (bit 0 = BUSY, bit 1 = DONE, bit 2 = ERROR)
 */

#define LLMP_DMA_PORT       7
#define LLMP_DMA_REG_SRC    0
#define LLMP_DMA_REG_DST    1
#define LLMP_DMA_REG_CNT    2
#define LLMP_DMA_REG_CTRL   3
#define LLMP_DMA_REG_STAT   4
#define LLMP_DMA_REG_MAX    5

/* Contrôle */
#define DMA_CTRL_ENABLE     0x01  /* Démarrer le transfert */
#define DMA_CTRL_IRQ_EN     0x02  /* Générer une IRQ à la fin */

/* Statut */
#define DMA_STAT_BUSY       0x01
#define DMA_STAT_DONE       0x02
#define DMA_STAT_ERROR      0x04

typedef struct {
    uint16_t src_addr;   /* Adresse source 16-bits */
    uint16_t dst_addr;   /* Adresse destination 1--bits */
    uint16_t count;      /* Nombre d’octets à copier */
    uint8_t  ctrl;       /* Registre de contrôle */
    uint8_t  stat;       /* Registre de statut */
    bool     irq_line;   /* Ligne IRQ générée */
} llmp16_dma_t;


void llmp16_dma_init(llmp16_dma_t *dma);
void llmp16_dma_cpy(llmp16_t *cpu, llmp16_dma_t *dma);
void llmp16_dma_readIO(llmp16_t *cpu, llmp16_dma_t *dma);
void llmp16_dma_step(llmp16_t *cpu, llmp16_dma_t *dma);




#endif // LLMP16_DMA_H

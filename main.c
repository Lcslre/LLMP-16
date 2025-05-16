#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "llmp16.h"         // Structure de la VM


void llmp16_init(llmp16_t *vm)
{
    vm->vbank = 0;
    vm->PC = 0;
    vm->SP = 0xFFFF; // Initialisation de la pile
    vm->FLAGS = 0;
    vm->halted = false;

    vm->ROM = (uint8_t **)malloc(LLMP_ROM_BANKS * sizeof(uint8_t *));
    for (int i = 0; i < LLMP_ROM_BANKS; i++) {
        vm->ROM[i] = (uint8_t *)malloc(LLMP_ROM_BANK_SIZE);
    }
    vm->RAM = (uint8_t **)malloc(LLMP_RAM_BANKS * sizeof(uint8_t *));
    for (int i = 0; i < LLMP_RAM_BANKS; i++) {
        vm->RAM[i] = (uint8_t *)malloc(LLMP_RAM_BANK_SIZE);
    }
    vm->VRAM = (uint8_t **)malloc(LLMP_VRAM_BANKS * sizeof(uint8_t *));
    for (int i = 0; i < LLMP_VRAM_BANKS; i++) {
        vm->VRAM[i] = (uint8_t *)malloc(LLMP_VRAM_BANK_SIZE);
    }
    
    llmp16_keyb_init(&vm->keyboard);
    
    llmp16_mmu_init(&vm->mmu);

    llmp16_timer_init(&vm->timer1, 0, 0, 0);
    llmp16_timer_init(&vm->timer2, 0, 0, 0);
    llmp16_timer_init(&vm->timer3, 0, 0, 0);

    vm->clk = 0;
    
    for (int i = 0; i < LLMP_IO_PORTS; i++) {
        for (int j = 0; j < LLMP_IO_REGS; j++) {
            vm->IO[i][j] = 0;
        }
    }
    

}

int main() {
    llmp16_t cpu;

    llmp16_init(&cpu);

    

    
    

    return 0;
}

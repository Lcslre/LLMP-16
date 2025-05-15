#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "llmp16.h"         // Structure de la VM


void llmp16_init(llmp16_t *vm)
{
    vm->bank = 0;
    vm->vbank = 0;
    vm->PC = 0;
    vm->SP = 0xFFFF; // Initialisation de la pile
    vm->FLAGS = 0;
    vm->halted = false;
    vm->ROM = (uint8_t **)malloc(LLMP_ROM_BANKS * sizeof(uint8_t *));
    for (int i = 0; i < LLMP_ROM_BANKS; i++) {
        vm->ROM[i] = (uint8_t *)malloc(LLMP_ROM_BANK_SIZE);
    }
    vm->DISK = (uint8_t **)malloc(LLMP_DISK_BANKS * sizeof(uint8_t *));
    for (int i = 0; i < LLMP_DISK_BANKS; i++) {
        vm->DISK[i] = (uint8_t *)malloc(LLMP_DISK_BANK_SIZE);
    }
    vm->RAM = (uint8_t **)malloc(LLMP_RAM_BANKS * sizeof(uint8_t *));
    for (int i = 0; i < LLMP_RAM_BANKS; i++) {
        vm->RAM[i] = (uint8_t *)malloc(LLMP_RAM_BANK_SIZE);
    }
    vm->VRAM = (uint8_t **)malloc(LLMP_VRAM_BANKS * sizeof(uint8_t *));
    for (int i = 0; i < LLMP_VRAM_BANKS; i++) {
        vm->VRAM[i] = (uint8_t *)malloc(LLMP_VRAM_BANK_SIZE);
    }
    vm->keyboard.head = 0;
    vm->keyboard.tail = 0;
    vm->mmu.segments[0].type = MMU_MAP_RAM;
    vm->mmu.segments[0].bank = 0;
    vm->mmu.segments[1].type = MMU_MAP_RAM;
    vm->mmu.segments[1].bank = 0;
    vm->timer1.count = 0;
    vm->timer1.PSC = 0;
    vm->timer1.status = 0;
    vm->timer1.value = 0;
    vm->timer1.init_value = 0;
    vm->timer2.count = 0;
    vm->timer2.PSC = 0;
    vm->timer2.status = 0;
    vm->timer2.value = 0;
    vm->timer2.init_value = 0;
    vm->timer3.count = 0;
    vm->timer3.PSC = 0;
    vm->timer3.status = 0;
    vm->timer3.value = 0;
    vm->timer3.init_value = 0;
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

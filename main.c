#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <string.h>
#include "llmp16.h"         // Structure de la VM


void llmp16_init(llmp16_t *vm)
{
    vm->vbank = 0;
    vm->SPR[PC] = 0;
    vm->SPR[SP] = 0xFFFFFF;

    vm->FLAGS = 0;
    vm->halted = false;

    vm->memory = (uint8_t *)malloc(LLMP_MEM_SIZE * sizeof(uint8_t *));

    vm->VRAM = (uint8_t **)malloc(LLMP_VRAM_BANKS * sizeof(uint8_t *));
    for (int i = 0; i < LLMP_VRAM_BANKS; i++) {
        vm->VRAM[i] = (uint8_t *)malloc(LLMP_VRAM_BANK_SIZE);
    }
    
    llmp16_keyb_init(&vm->keyboard);
    
    llmp16_timer_init(&vm->timer1, 0, 0, 0);
    llmp16_timer_init(&vm->timer2, 0, 0, 0);
    llmp16_timer_init(&vm->timer3, 0, 0, 0);
    llmp16_screen_init(&vm->screen,vm->VRAM);

    vm->clk = 0;
    
    for (int i = 0; i < LLMP_IO_PORTS; i++) {
        for (int j = 0; j < LLMP_IO_REGS; j++) {
            vm->IO[i][j] = 0;
        }
    }
    

}

int main()
{
    return 0;
}

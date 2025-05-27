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
    llmp16_reg_set(vm, PC, 0);
    llmp16_reg_set(vm, SP, 0xFFFFF);

    vm->FLAGS = 0;
    vm->halted = false;

    vm->memory = (uint8_t *)malloc(LLMP_MEM_SIZE * sizeof(uint8_t));

    vm->VRAM = malloc(LLMP_VRAM_BANKS * sizeof(uint8_t *));
    for (int i = 0; i < LLMP_VRAM_BANKS; ++i)
        vm->VRAM[i] = malloc(LLMP_VRAM_BANK_SIZE);

    llmp16_keyb_init();
    
    llmp16_timer_init(&vm->timer1, 0, 0, 0);
    llmp16_timer_init(&vm->timer2, 0, 0, 0);
    llmp16_timer_init(&vm->timer3, 0, 0, 0);
    llmp16_screen_init(&vm->screen);
    llmp16_dma_init(&vm->dma);

    vm->clk = 0;
    
    for (int i = 0; i < LLMP_IO_PORTS; i++) {
        for (int j = 0; j < LLMP_IO_REGS; j++) {
            vm->IO[i][j] = 0;
        }
    }
    

}


void llmp16_run(llmp16_t *vm)
{
    while(!vm->halted)
    {
        llmp16_cpu_cycle(vm);
        llmp16_screen_render(vm->screen, vm->VRAM);
        llmp16_keyboard_scan(vm);
        llmp16_dma_step(vm, &vm->dma);
        llmp16_blitter_step(vm);
        //printf("%d\n", vm->R[0]);
        //if(vm->R[PC] >= 18) vm->halted = true;
    } 
}

void llmp16_off(llmp16_t *vm)
{
    free(vm->memory);
    free(vm->VRAM[0]);
    free(vm->VRAM[1]);
    free(vm->VRAM);
    llmp16_screen_off(&vm->screen);
    free(vm);
}

int main(int argc, char *argv[])
{
    /*==================== Initialisation de la machine virtuelle =====================*/
    llmp16_t* vm = (llmp16_t*)malloc(sizeof(llmp16_t));
    llmp16_init(vm);
    if(argc > 1)
    {
        llmp16_rom_load(vm, argv[1]);
    }

    dump_memory(vm->memory, 512);

    /*==================== Boucle de simulation =====================*/
    llmp16_run(vm);
    llmp16_off(vm);



    return 0;
}

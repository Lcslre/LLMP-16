#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <string.h>
#include "llmp16.h"         // Structure de la VM



void llmp16_init(llmp16_t *vm)
{
    llmp16_reg_set(vm, PC, 0);
    llmp16_reg_set(vm, SP, 0xFFFFF);

    vm->FLAGS = 0;
    vm->halted = false;

    vm->memory = (uint8_t *)malloc(LLMP_MEM_SIZE * sizeof(uint8_t));

    vm->VRAM = (uint8_t*)malloc(LLMP_VRAM_BANK_SIZE * sizeof(uint8_t ));
    
    llmp16_keyb_init();
    
    llmp16_timer_init(&vm->timer1, 0, 0, 0);
    llmp16_timer_init(&vm->timer2, 0, 0, 0);
    llmp16_timer_init(&vm->timer3, 0, 0, 0);
    llmp16_screen_init(&vm->screen);
    llmp16_dma_init(&vm->dma);

    
    for (int i = 0; i < LLMP_IO_PORTS; i++) {
        for (int j = 0; j < LLMP_IO_REGS; j++) {
            vm->IO[i][j] = 0;
        }
    }
    

}

void llmp16_debug_dump(llmp16_t *vm)
{
    const char* reg_names[16] = {
        "R0", "R1", "R2", "R3",
        "R4", "R5", "R6", "R7",
        "R8", "R9", "R10","RR11",
        "PC","SP","IDX","ACC"
    };

    // 1) Registres généraux et spéciaux
    printf("=== Registres ===\n");
    for (int i = 0; i < 16; i++) {
        uint32_t val = llmp16_reg_get(vm, (llmp16_register_t)i);
        // On affiche sur 16 bits pour R0-RR11, sur 32 bits pour PC/SP/IDX/ACC
        if (i < 12)
            printf("%4s = 0x%04X\n", reg_names[i], val);
        else
            printf("%4s = 0x%08X\n", reg_names[i], val);
    }

    // 2) Flags NZCV
    printf("\n=== Flags ===\n");
    printf("N=%u  Z=%u  C=%u  V=%u\n",
        flag_get(vm, FLAG_N),
        flag_get(vm, FLAG_Z),
        flag_get(vm, FLAG_C),
        flag_get(vm, FLAG_V));

    // 3) Autres états de la VM
    printf("\n=== VM State ===\n");
    printf("HALTED = %u\n", vm->halted);


    // 4) Tous les ports IO
    printf("\n=== IO Ports ===\n");
    for (int p = 0; p < LLMP_IO_PORTS; p++) {
        printf("Port %2d: ", p);
        for (int r = 0; r < LLMP_IO_REGS; r++) {
            printf("%04X ", vm->IO[p][r]);
        }
        printf("\n");
    }
}


#define CPU_FREQ     5000000   // 5 MHz
#define FRAME_RATE   60
#define CYCLES_PER_FRAME (CPU_FREQ / FRAME_RATE)

void llmp16_run(llmp16_t* vm) {
    const uint32_t frameDelay = 1000 / FRAME_RATE;  // en ms (~16 ms)
    uint32_t frameStart, frameTime;

    while (!vm->halted) {
        frameStart = SDL_GetTicks();

        llmp16_keyboard_scan(vm);

        // exécute CYCLES_PER_FRAME cycles avant chaque rendu
        for (uint32_t i = 0; i < CYCLES_PER_FRAME; i++) {
            llmp16_cpu_cycle(vm);
            llmp16_blitter_step(vm);
            llmp16_dma_step(vm, &vm->dma);
        }

        //llmp16_debug_dump(vm);

        //printf("%d\n", llmp16_reg_get(vm, 0));

        // un seul rendu par frame
        llmp16_screen_render(vm->screen, vm->VRAM);

        // throttle pour rester à ~60 Hz
        frameTime = SDL_GetTicks() - frameStart;
        if (frameDelay > frameTime)
            SDL_Delay(frameDelay - frameTime);
    }
}



void llmp16_off(llmp16_t *vm)
{
    free(vm->memory);
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

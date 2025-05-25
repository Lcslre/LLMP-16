#include "llmp16.h"
#include <SDL2/SDL.h>

void llmp16_keyb_init(llmp16_keyboard_t *kb)
{
    kb->head = 0;
    kb->tail = 0;
    for (int i = 0; i < LLMP_KEY_QUEUE_SIZE; i++) {
        kb->keys[i] = 0;
    }

    SDL_Init(SDL_INIT_VIDEO);
    SDL_StartTextInput();
}

// La fonction llmp16_keyboard_scan() lit les événements SDL et ajoute les touches pressées à la file d'attente du clavier.
// Elle est appelée à chaque itération de la boucle principale de la machine virtuelle.
void llmp16_keyboard_scan(llmp16_t *vm)
{
    SDL_Event event;
    if (SDL_PollEvent(&event)) {
        if (event.type == SDL_KEYDOWN) {
            // kb->keys[kb->head] = event.key.keysym.sym;
            // kb->head = (kb->head + 1) % LLMP_KEY_QUEUE_SIZE;
            vm->IO[1][0] = event.key.keysym.sym; // Envoie la touche pressée au registre 0
            vm->IO[1][1] = 0x01; // Indique qu'une touche est pressée
        }
        if(event.type == SDL_QUIT) vm->halted = true;
    }
}




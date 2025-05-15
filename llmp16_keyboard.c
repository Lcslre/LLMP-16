#include "llmp16.h"
#include <SDL2/SDL.h>

void llmp16_keyb_init()
{
    SDL_Init(SDL_INIT_VIDEO);
    SDL_StartTextInput();
}

// La fonction llmp16_keyboard_scan() lit les événements SDL et ajoute les touches pressées à la file d'attente du clavier.
// Elle est appelée à chaque itération de la boucle principale de la machine virtuelle.
void llmp16_keyboard_scan(llmp16_keyboard_t *kb)
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_KEYDOWN) {
            kb->keys[kb->head] = event.key.keysym.sym;
            kb->head = (kb->head + 1) % LLMP_KEY_QUEUE_SIZE;
        }
    }
}

// La fonction llmp16_keyboard_update() met à jour les registres du port 2 de la machine virtuelle.
// Elle est appelée à chaque itération de la boucle principale de la machine virtuelle.
void llmp16_keyboard_update(llmp16_t *cpu, llmp16_keyboard_t *kb)
{
    if (kb->head != kb->tail) {
        uint8_t key = kb->keys[kb->tail];
        kb->tail = (kb->tail + 1) % LLMP_KEY_QUEUE_SIZE;
        cpu->IO[2][0] = key; // Envoie la touche pressée au registre 0
        cpu->IO[2][1] = 0x01; // Indique qu'une touche est pressée
    } else {
        cpu->IO[2][0] = 0x00; // Aucune touche pressée
        cpu->IO[2][1] = 0x00; // Indique qu'aucune touche n'est pressée
    }
}




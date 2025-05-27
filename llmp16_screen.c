#include <SDL2/SDL.h>
#include <stdlib.h>
#include <stdint.h>
#include <strings.h>
#include "llmp16.h"


int llmp16_screen_init(llmp16_screen_t *screen)
{
    if(0 != SDL_Init(SDL_INIT_VIDEO))
    {
        fprintf(stderr, "Erreur SDL_Init : %s", SDL_GetError());
        SDL_Quit();
        return EXIT_FAILURE;
    }

    screen->window = SDL_CreateWindow("LLMP-16", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
        LLMP_WINDOW_WIDTH, LLMP_WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    
    if(NULL == screen->window)
    {
        fprintf(stderr, "Erreur SDL_CreateWindow : %s", SDL_GetError());
        SDL_Quit();
        return EXIT_FAILURE;
    }

    screen->renderer = SDL_CreateRenderer(screen->window, -1, SDL_RENDERER_ACCELERATED);

    if(screen->renderer == NULL)
    {
        fprintf(stderr, "Erreur SDL_CreateRenderer : %s", SDL_GetError());
        SDL_Quit();
        return EXIT_FAILURE;
    }

    screen->framebuffer = SDL_CreateTexture(screen->renderer, SDL_PIXELFORMAT_RGB332, SDL_TEXTUREACCESS_STREAMING, LLMP_SCREEN_WIDTH, LLMP_SCREEN_HEIGHT);

    if(screen->framebuffer == NULL)
    {
        fprintf(stderr, "Erreur SDL_CreateTexture : %s", SDL_GetError());
        llmp16_screen_off(screen);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}


void llmp16_screen_off(llmp16_screen_t *screen)
{
    if(screen->framebuffer != NULL) SDL_DestroyTexture(screen->framebuffer);
    if(screen->renderer != NULL) SDL_DestroyRenderer(screen->renderer);
    if(screen->window != NULL) SDL_DestroyWindow(screen->window);
    SDL_Quit();
}


void llmp16_screen_render(llmp16_screen_t screen,uint8_t* VRAM)
{
    SDL_UpdateTexture(screen.framebuffer, NULL,
                  VRAM, LLMP_SCREEN_WIDTH);
    SDL_RenderClear(screen.renderer);
    SDL_RenderCopy(screen.renderer, screen.framebuffer, NULL, NULL);
    SDL_RenderPresent(screen.renderer);


}


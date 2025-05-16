#include <SDL2/SDL.h>
#include <stdlib.h>
#include <stdint.h>
#include <strings.h>


#define LLMP_SCREEN_HEIGHT 200
#define LLMP_SCREEN_WIDTH  320
#define LLMP_SCREEN_SCALE 4
#define LLMP_WINDOW_WIDTH LLMP_SCREEN_WIDTH * LLMP_SCREEN_SCALE
#define LLMP_WINDOW_HEIGHT LLMP_SCREEN_HEIGHT * LLMP_SCREEN_SCALE

typedef struct
{
    SDL_Window *window;
    SDL_Renderer* renderer;
    SDL_Texture *framebuffer;
    uint8_t **VRAM;
}llmp16_screen_t;

int llmp16_screen_init(llmp16_screen_t *screen);
int llmp16_screen_off(llmp16_screen_t *screen);
void llmp16_screen_render(llmp16_screen_t screen);


int llmp16_screen_init(llmp16_screen_t *screen)
{

    screen->VRAM = (uint8_t**)malloc(sizeof(uint8_t*)*2);

    if(screen->VRAM == NULL)
    {
        return EXIT_FAILURE;
    }

    screen->VRAM[0] = (uint8_t*)malloc(LLMP_SCREEN_HEIGHT * LLMP_SCREEN_WIDTH*sizeof(uint8_t));

    if(screen->VRAM[0] == NULL)
    {
        return EXIT_FAILURE;
    }
    screen->VRAM[1] = (uint8_t*)malloc(LLMP_SCREEN_HEIGHT * LLMP_SCREEN_WIDTH*sizeof(uint8_t));

    if(screen->VRAM[1] == NULL)
    {
        return EXIT_FAILURE;
    }

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


int llmp16_screen_off(llmp16_screen_t *screen)
{
    if (screen->VRAM) {
        if (screen->VRAM[0]) free(screen->VRAM[0]);
        if (screen->VRAM[1]) free(screen->VRAM[1]);
        free(screen->VRAM);
    }

    if(screen->framebuffer != NULL) SDL_DestroyTexture(screen->framebuffer);
    if(screen->renderer != NULL) SDL_DestroyRenderer(screen->renderer);
    if(screen->window != NULL) SDL_DestroyWindow(screen->window);
    SDL_Quit();
}


void llmp16_screen_render(llmp16_screen_t screen)
{
    // Étape 1 : Copier la VRAM 1 dans la texture
    uint8_t *pixels;
    int pitch;
    if (SDL_LockTexture(screen.framebuffer, NULL, (void**)&pixels, &pitch) == 0)
    {
        for (int y = 0; y < LLMP_SCREEN_HEIGHT; ++y)
        {
            memcpy(pixels + y * pitch, screen.VRAM[1] + y * LLMP_SCREEN_WIDTH, LLMP_SCREEN_WIDTH);
        }
        SDL_UnlockTexture(screen.framebuffer);
    }

    // Étape 2 : Affichage de la texture
    SDL_RenderClear(screen.renderer);
    SDL_RenderCopy(screen.renderer, screen.framebuffer, NULL, NULL);
    SDL_RenderPresent(screen.renderer);

    // Etape 3 : On échange VRAM 1 et VRAM 0
    uint8_t* tmp = screen.VRAM[0];
    screen.VRAM[0] = screen.VRAM[1];
    screen.VRAM[1] = tmp;
}


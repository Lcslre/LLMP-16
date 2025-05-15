#include <SDL2/SDL.h>
#include <stdlib.h>

#define W_WIDTH    640
#define W_HEIGHT   400


#define BLACK   0, 0, 0, 255
#define WHITE   255, 255, 255, 255
uint8_t r,g ,b;
uint32_t couleur;

SDL_bool ecran_actif = SDL_TRUE;

SDL_Window *p_Window = NULL;
SDL_Renderer *p_Render = NULL;
SDL_Texture *texture = NULL;
SDL_Event event;



void ecran_init(void);
void ecran_inputs(void);
void ecran_clean(void);
void ecran_affiche_pixels(void);

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    ecran_init();

    while(ecran_actif)
    {
        ecran_inputs();

        // Exemple: remplir la VRAM 
       // Dans ta boucle de rendu :
for (int y = 0; y < W_HEIGHT; y++) {
    for (int x = 0; x < W_WIDTH; x++) {
        // Lire le pixel compressé
        uint8_t pixel8 = vram[0][y * W_WIDTH + x];

        // Extraire les composantes
        uint8_t r3 = (pixel8 >> 5) & 0x07; // bits 7-5
        uint8_t g3 = (pixel8 >> 2) & 0x07; // bits 4-2
        uint8_t b2 = pixel8 & 0x03;        // bits 1-0

        // Étendre sur 8 bits (approx.) :
        r = (r3 * 255) / 7;
        g = (g3 * 255) / 7;
        b = (b2 * 255) / 3;
        // Composer un pixel 32 bits ARGB
        couleur = (r << 24) | (g << 16) | (b << 8) | 255;

        // Stocker dans la texture d'affichage
        vram[1][y * W_WIDTH + x] = couleur;
    }
}

        SDL_SetRenderDrawColor(p_Render,r,g,b,255);
        SDL_RenderClear(p_Render);

        ecran_affiche_pixels();

        SDL_RenderPresent(p_Render);
        SDL_Delay(16);
    }

    ecran_clean();
    return 0;
}

void ecran_init(void)
{
    if(SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        SDL_Log("Erreur SDL Init: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    p_Window = SDL_CreateWindow("Affichage VRAM",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        W_WIDTH, W_HEIGHT,
        SDL_WINDOW_SHOWN);

    p_Render = SDL_CreateRenderer(p_Window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if(!p_Window || !p_Render) {
        SDL_Log("Erreur fenêtre ou rendu: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    texture = SDL_CreateTexture(
        p_Render,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        W_WIDTH, W_HEIGHT
    );

    if (!texture) {
        SDL_Log("Erreur création texture: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
}

void ecran_inputs(void)
{
    while(SDL_PollEvent(&event)) {
        if(SDL_QUIT == event.type) ecran_actif = SDL_FALSE;
        if(SDL_KEYDOWN == event.type) {
            switch(event.key.keysym.sym) {
                case SDLK_ESCAPE: ecran_actif = SDL_FALSE;
                break;
            }
        }
    }
}

void ecran_affiche_pixels(void)
{
    SDL_UpdateTexture(texture, NULL, vram, W_WIDTH * sizeof(Uint32));
    SDL_RenderCopy(p_Render, texture, NULL, NULL);
}

void ecran_clean(void)
{
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(p_Render);
    SDL_DestroyWindow(p_Window);
    SDL_Quit();
}

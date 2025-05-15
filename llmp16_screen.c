#include <SDL2/SDL.h>
#include <stdlib.h>

#define W_WIDTH    640
#define W_HEIGHT   400


#define BLACK   0, 0, 0, 255
#define WHITE   255, 255, 255, 255


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
        for (int y = 0; y < W_HEIGHT; y++) {
           for (int x = 0; x < W_WIDTH; x++) {
            // Base RGB en fonction de bande
             Uint8 r = 0, g = 0, b = 0;
              if (x < W_WIDTH / 5) {
                 r = 0; g = 0; b = 0;        // NOIR
              } else if (x < 2 * W_WIDTH / 5) {
                r = 255; g = 0; b = 0;      // ROUGE
              } else if (x < 3 * W_WIDTH / 5) {
                r = 0; g = 255; b = 0;      // VERT
              } else if (x < 4 * W_WIDTH / 5) {
                r = 0; g = 0; b = 255;      // BLEU
              } else {
                r = 255; g = 255; b = 255;  // BLANC
        }

        // Encode couleur complète dans VRAM (ARGB)
        Uint32 pixel = (255 << 24) | (r << 16) | (g << 8) | b;
        vram[y * W_WIDTH + x] = pixel;

        // Lire à nouveau le pixel de la VRAM
        Uint32 px = vram[y * W_WIDTH + x];

        // Appliquer MASQUES pour extraire les 3 bits les plus forts
        Uint8 r3 = (px >> 16) & 0xE0; // 11100000
        Uint8 g3 = (px >> 8)  & 0xE0;
        Uint8 b3 = px & 0xE0;

        // Reconvertir 3 bits en 8 bits (ex: r3 >> 5 donne 0–7, on scale vers 255)
        Uint8 rf = (r3 >> 5) * 36;  // (255 / 7) ≈ 36
        Uint8 gf = (g3 >> 5) * 36;
        Uint8 bf = (b3 >> 5) * 36;

        // Écrire à nouveau dans la VRAM avec les composantes limitées
        vram[y * W_WIDTH + x] = (255 << 24) | (rf << 16) | (gf << 8) | bf;
    }
}

        SDL_SetRenderDrawColor(p_Render, BLACK);
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

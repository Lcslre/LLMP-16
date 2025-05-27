#include "llmp16.h"

// largeur de l'écran en pixels
#define LLMP_SCREEN_WIDTH   320

void llmp16_blitter_step(llmp16_t *vm) {
    uint16_t src   = vm->IO[LLMP_BLT_PORT][LLMP_BLT_REG_SRC];
    uint16_t x     = vm->IO[LLMP_BLT_PORT][LLMP_BLT_REG_X];
    uint16_t y     = vm->IO[LLMP_BLT_PORT][LLMP_BLT_REG_Y];
    uint16_t w     = vm->IO[LLMP_BLT_PORT][LLMP_BLT_REG_W];
    uint16_t h     = vm->IO[LLMP_BLT_PORT][LLMP_BLT_REG_H];
    uint16_t ctrl  = vm->IO[LLMP_BLT_PORT][LLMP_BLT_REG_CTRL];

    if (!(ctrl & BLT_CTRL_START))
        return;

    bool bitmode = (ctrl & BLT_CTRL_BITMODE) != 0;

    for (uint16_t row = 0; row < h; row++) {
        uint32_t vram_base = (uint32_t)(y + row) * LLMP_SCREEN_WIDTH + x;

        if (!bitmode) {
            // copie simple octet→pixel (pour les sprites déjà unpackés)
            // copie simple octet→pixel (mode non-bit)
            uint8_t* srcptr = vm->memory + src + row*w;
            uint8_t* dstptr = vm->VRAM + vram_base;
            memcpy(dstptr, srcptr, w);

        } else {
            static uint8_t expand[256][8];
            // à l’initialisation du module :
            for(int b=0; b<256; b++){
            for(int i=0; i<8; i++){
                expand[b][i] = (b & (0x80>>i)) ? 0xFF : 0x00;
            }
            }

            // puis, dans la boucle :
            uint16_t bytesPerRow = w/8;
            uint8_t* dst  = vm->VRAM + vram_base;
            uint8_t* srcb = vm->memory + src + row*bytesPerRow;
            for(uint16_t k=0; k<bytesPerRow; k++){
            memcpy(dst + k*8, expand[srcb[k]], 8);
            }

        }
    }

    // on nettoie START pour ne pas relancer
    vm->IO[LLMP_BLT_PORT][LLMP_BLT_REG_CTRL] &= ~(BLT_CTRL_START);
}

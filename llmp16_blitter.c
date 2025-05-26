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
            for (uint16_t col = 0; col < w; col++) {
                uint8_t px = mem_read8(vm, src + row * w + col);
                vram_write(vm, (uint16_t)(vram_base + col), px);
            }
        } else {
            // décompression bit-à-bit : chaque octet donne 8 pixels
            for (uint16_t col = 0; col < w; col++) {
                // on parcourt les pixels d'une ligne, w doit être multiple de 8
                uint16_t byte_idx = src + (row * (w/8)) + (col / 8);
                uint8_t mask = 0x80 >> (col % 8);
                uint8_t bit = (mem_read8(vm, byte_idx) & mask) ? 0xFF : 0x00;
                vram_write(vm, (uint16_t)(vram_base + col), bit);
            }
        }
    }

    // on nettoie START pour ne pas relancer
    vm->IO[LLMP_BLT_PORT][LLMP_BLT_REG_CTRL] &= ~(BLT_CTRL_START);
}

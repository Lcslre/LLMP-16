#include "llmp16.h"

void llmp16_mmu_update(llmp16_t *cpu) {
    llmp16_mmu_mapping_t *s0 = &(cpu->mmu.segments[0]);
    llmp16_mmu_mapping_t *s1 = &(cpu->mmu.segments[1]);

    s0->type = cpu->IO[0][0] & 0xFF;
    s0->bank = cpu->IO[0][1] & 0xFF;
    s1->type = cpu->IO[0][2] & 0xFF;
    s1->bank = cpu->IO[0][3] & 0xFF;
}
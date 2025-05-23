#include "llmp16.h"


void llmp16_mmu_init(llmp16_mmu_t *mmu) {
    mmu->code_segment = 0;
    mmu->data_segment = 0;
    mmu->stack_segment = 0;
}

void llmp16_mmu_update(llmp16_t *cpu, llmp16_mmu_t *mmu) {
    mmu->code_segment = cpu->IO[0][0] & 0xFF;
    mmu->data_segment = cpu->IO[0][1] & 0xFF;
    mmu->stack_segment = cpu->IO[0][2] & 0xFF;
}

uint32_t llmp16_mmu_addr(llmp16_mmu_t mmu, llmp16_mmu_type_t type, uint16_t offset)
{
    uint32_t addr;

    switch (type)
    {

    case MMU_SEGMENT_CODE:
        addr = (mmu.code_segment << 16) + offset;
        break;
    case MMU_SEGMENT_DATA:
        addr = (mmu.data_segment << 16) + offset;
        break;
    case MMU_SEGMENT_STACK:
        addr = (mmu.stack_segment << 16) + offset;
        break;
    
    default:
        addr = 0;
        break;
    }

    return addr;
}
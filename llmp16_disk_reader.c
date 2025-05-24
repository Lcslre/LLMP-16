#include "llmp16.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>


uint16_t CHS_to_offset(uint16_t CHS) {
    uint8_t H = (CHS >> 15) & 0x01;
    uint8_t C = (CHS >> 8) & 0x7F;
    uint8_t S = CHS & 0xFF;

    if (S < 1 || S > 40 || C >= 8 || H > 1) {
        fprintf(stderr, "CHS invalide : C=%d H=%d S=%d\n", C, H, S);
        return -1;
    }

    uint16_t segment = ((C * 2 + H) * 40) + (S-1);
    return segment * 512;
}

void llmp16_disk_read_segment(FILE *disk, uint16_t CHS, uint8_t *mem) {
    fseek(disk, CHS_to_offset(CHS), SEEK_SET);
    if (fread(mem, 512, 1 , disk) != 1) perror("Disk read error");
}   

void llmp16_disk_write_segment(FILE *disk, uint16_t CHS, uint8_t *mem) {
    fseek(disk, CHS_to_offset(CHS), SEEK_SET);
    fwrite(mem, 512, 1 , disk);
}

// Fonction pour afficher un bloc mémoire
void dump_memory(const uint8_t *mem, size_t size) {
    for (size_t i = 0; i < size; i += 16) {
        printf("%04zx : ", i);
        for (size_t j = 0; j < 16 && i + j < size; j++) {
            printf("%02x ", mem[i + j]);
        }
        printf(" | ");
        for (size_t j = 0; j < 16 && i + j < size; j++) {
            char c = mem[i + j];
            printf("%c", (c >= 32 && c <= 126) ? c : '.');
        }
        printf("\n");
    }
}

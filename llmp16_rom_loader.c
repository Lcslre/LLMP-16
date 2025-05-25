#include "llmp16.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


/*
	La LLMP16 a une puce de ROM de 64Ko, pour ecrire dans la ROM il faut la flacher lors de l'éxécution du programme
	La ROM est chargée en mémoire lors de l'initialisation de la VM, elle est chargée à partir de l'adresse 0.
	En générale la ROM contient le BIOS et les vecteurs d'intéruptions, c'est lui qui charge l'OS à partir de l'adresse 0x08000. 
*/

void llmp16_rom_load(llmp16_t *vm, char* file){
	FILE* rom_file = fopen(file, "rb");
	if (fread(vm->memory, LLMP_ROM_SIZE, 1 , rom_file) != 1) perror("ROM read error");
	fclose(rom_file);
}

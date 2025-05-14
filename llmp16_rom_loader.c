#include "llmp16.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void llmp16_rom_load(llmp16_t *cpu, char* file){
	llmp16_header_rom_file_t head;
	FILE* rom_file = fopen(file, "rb");
	fread(&head, sizeof(llmp16_header_rom_file_t), 1, rom_file);
	if(head.code != FILE_CODE)
		return;
	//page 0 réservé au bios et interuptions
	int page_act = 1;
	while(page_act < head.nb_pages){
  		char* page_content = (char*)malloc(sizeof(head.taille_page[head.nb_pages]));
  		fread(page_content, sizeof(head.taille_page[head.nb_pages]), 1, rom_file);
		memcpy( cpu->ROM[page_act-1], page_content, sizeof(head.taille_page[head.nb_pages]));
		free(page_content);
  		page_act++;
	}
	fclose(rom_file);
}

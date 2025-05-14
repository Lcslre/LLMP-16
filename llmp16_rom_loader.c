#include "llmp16.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void llmp16_rom_load(llmp16_t *cpu, char* file){
	header_file_t *head = (header_file_t*)malloc(sizeof(header_file_t));
	char* FILE = fopen(file, "rb");
	fread(head, sizeof(header_file_t), 1, FILE);
	if(head->code != FILE_CODE)
		return;
	int page_act = 0;
	while(page_act < head->nb_pages){
  		char* page_content = (char*)malloc(sizeof(head->taille_page[head->nb_pages]));
  		fread(page_content, sizeof(head->taille_page[head->nb_pages]), 1, FILE);
		memcpy( cpu->ROM[page_act][], page_content, sizeof(head->taille_page[head->nb_pages]));
  		page_act++;
	}
  
}

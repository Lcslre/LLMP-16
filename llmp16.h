#ifndef LLMP_VM_H
#define LLMP_VM_H


#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

/*
*                       Spécifications de la machine virtuelle
*| Paramètre                | Valeur                                            |
*|--------------------------|---------------------------------------------------|
*| Horloge CPU              | 5 MHz                                             |
*| Largeur bus données      | 16 bits                                           |
*| Largeur bus adresses     | 16 bits adressés par octet (0–65 535)             |
*| Registres généraux       | R0–R15 (R15 = ACC), PC, SP, FLAGS                 |
*| ROM bancaire             | 128 banques × 32 Ko (total = 4 Mio)               |
*| RAM                      | 32 Ko (map \$8000–\$FFFF)                         |
*| VRAM                     | 128 Ko (320 * 200 * 8bpp) x 2                     |
*| Ports E/S                | 16 ports * 16 registres                           |
*| Endian                   | little‑endian (LSB à l’adresse la plus basse)     |
*/
 
#define LLMP_ROM_BANKS   128
#define LLMP_ROM_BANK_SIZE 0x8000  /* 32 KiB */
#define LLMP_DISK_BANKS   128
#define LLMP_DISK_BANK_SIZE 0x8000  /* 32 KiB */
#define LLMP_RAM_BANKS   16
#define LLMP_RAM_BANK_SIZE      0x8000  /* 32 KiB */
#define LLMP_VRAM_BANKS   2
#define LLMP_VRAM_BANK_SIZE 0x10000  /* 64 KiB */
#define LLMP_IO_PORTS    16
#define LLMP_IO_REGS     16

enum{
   FLAG_N = 0x08, /* Negative (bit 3) */
   FLAG_Z = 0x04, /* Zero     (bit 2) */
   FLAG_C = 0x02, /* Carry    (bit 1) */
   FLAG_V = 0x01  /* oVerflow (bit 0) */
};

typedef struct llmp16_s llmp16_t;

/*=========================== KeyBoard =====================================*/

/*
Le clavier est géré par SDL2. Il faut donc initialiser la bibliothèque SDL2 avant d'utiliser ces fonctions.
On définit le clavier comme une file d'attente de touches. On peut ajouter des touches à la file d'attente avec la fonction llmp16_key_push() 
et les récupérer avec la fonction llmp16_key_pop().

Le clavier correspond au port 2 de la machine virtuelle.
Il y a 2 registres de 16 bits chacun. Le registre 0 contient le code de la touche pressée (0x00 si aucune touche n'est pressée).
Le registre 1 contient le status du clavier (touche pressée ou pas).
*/

#define LLMP_KEY_QUEUE_SIZE 8

typedef struct {
   uint8_t keys[LLMP_KEY_QUEUE_SIZE];
   uint8_t head;
   uint8_t tail;
} llmp16_keyboard_t;

void llmp16_keyb_init();

/* La fonction llmp16_keyboard_scan() est appelée à chaque itération de la boucle principale de la machine virtuelle.
   Elle permet de scanner l'état du clavier et d'ajouter les touches pressées à la file d'attente. */
void llmp16_keyboard_scan(llmp16_keyboard_t *kb);


void llmp16_keyboard_update(llmp16_t *cpu, llmp16_keyboard_t *kb);


/*====================================== TIMER ==========================================*/

/* Les timers fonctionnent tous en mode comparateur uniquement */

typedef struct
{
   uint16_t count; // valeur courante du timer
   uint8_t PSC; // préscaler
   uint16_t value; // valeur max/min
   uint16_t init_value;
   uint16_t status;
}llmp16_timer_t;


void llmp16_timer_init(llmp16_timer_t *timer, uint8_t PSC, uint16_t value, uint16_t init_value);
void llmp16_timer_count(llmp16_timer_t *timer, uint8_t clk_counter);

/*====================================== MMU ==========================================*/

typedef enum {
  MMU_MAP_ROM,
  MMU_MAP_DISK,
  MMU_MAP_RAM
} llmp16_mmu_type_t;

typedef struct {
  llmp16_mmu_type_t type;
  uint8_t bank; // bank ID
} llmp16_mmu_mapping_t;

typedef struct {
  llmp16_mmu_mapping_t segments[2]; // segment 0: 0x0000-7FFF, segment 1: 0x8000-FFFF
} llmp16_mmu_t;

/*============================== Machine Virtuelle ==============================*/

typedef struct llmp16_s {
   uint16_t R[16];                      /* R0‑R15 (R15 = ACC) */
   uint16_t PC;                         /* Program Counter    */
   uint16_t SP;                         /* Stack Pointer      */
   uint8_t  FLAGS;                      /* NZCV, bits 3..0    */
   uint8_t  bank;                       /* Current ROM bank   */
   uint8_t  vbank;                       /* Current VRAM bank   */

   bool halted;
 
   // uint8_t  ROM[LLMP_ROM_BANKS][LLMP_ROM_BANK_SIZE];
   // uint8_t  DISK[LLMP_DISK_BANKS][LLMP_DISK_BANK_SIZE];
   // uint8_t  RAM[LLMP_RAM_BANKS][LLMP_RAM_SIZE];
   // uint8_t  VRAM[LLMP_VRAM_BANKS][LLMP_VRAM_BANK_SIZE];

   uint8_t  **ROM;
   uint8_t  **DISK;
   uint8_t  **RAM;
   uint8_t  **VRAM;

   llmp16_keyboard_t keyboard;

   llmp16_mmu_t mmu; // memory management unit

   llmp16_timer_t timer1;                  /* Timer 1 (16 bits) */
   llmp16_timer_t timer2;                  /* Timer 2 (16 bits) */
   llmp16_timer_t timer3;                  /* Timer 3 (16 bits) */

 
   uint16_t IO[LLMP_IO_PORTS][LLMP_IO_REGS];  /* 16‑bit regs  */

} llmp16_t;



void llmp16_run(llmp16_t *cpu);
void llmp16_init(llmp16_t *vm);
 
/*============== Routines de mises à jour des flags ===============*/
 
static inline void flag_set(llmp16_t *cpu, uint8_t mask, bool cond)
{
   if (cond) cpu->FLAGS |=  mask;
   else cpu->FLAGS &= ~mask;
}
 
static inline bool flag_get(const llmp16_t *cpu, uint8_t mask)
{
   return (cpu->FLAGS & mask) != 0;
}
 
/* Mise à jour des flags N et Z */
static inline void flag_nz(llmp16_t *cpu, uint16_t res)
{
   flag_set(cpu, FLAG_Z, res == 0); // Si le résultat est nul alors on met le flag Z à 1 sinon on le met à 0
   flag_set(cpu, FLAG_N, (res & 0x8000) != 0); // Si le bit de poids fort est à 1 alors on met le flag N à 1 sinon on le met à 0
}
 
static inline void flag_add_cv(llmp16_t *cpu, uint16_t a, uint16_t b, uint32_t result32)
{
   flag_set(cpu, FLAG_C, result32 > 0xFFFF);
   uint16_t res = (uint16_t)result32;
   /* Overflow si a et b ont le même signe mais le résulat à un singe différent */
   bool ov = (~(a ^ b) & (a ^ res) & 0x8000) != 0;
   flag_set(cpu, FLAG_V, ov);
}
 
static inline void flag_sub_cv(llmp16_t *cpu, uint16_t a, uint16_t b, uint32_t result32)
{
   flag_set(cpu, FLAG_C, result32 & 0x10000);        /* borrow -> carry clear; we invert later */
   uint16_t res = (uint16_t)result32;
   bool ov = ((a ^ b) & (a ^ res) & 0x8000) != 0;
   flag_set(cpu, FLAG_V, ov);
}
 
/*============== Routines de manipulation de la mémoire ==============*/

void llmp16_mmu_update(llmp16_t *cpu);

static inline uint8_t mem_read8(llmp16_t *cpu, uint16_t addr) {
  llmp16_mmu_mapping_t map = (addr < 0x8000) ? cpu->mmu.segments[0] : cpu->mmu.segments[1];
  uint16_t offset = addr & 0x7FFF;

  switch (map.type) {
      case MMU_MAP_ROM:  return cpu->ROM[map.bank][offset];
      case MMU_MAP_DISK: return cpu->DISK[map.bank][offset];
      case MMU_MAP_RAM:  return cpu->RAM[map.bank][offset];
      default: return 0xFF;
  }
}

static inline void mem_write8(llmp16_t *cpu, uint16_t addr, uint8_t v) {
  llmp16_mmu_mapping_t map = (addr < 0x8000) ? cpu->mmu.segments[0] : cpu->mmu.segments[1];
  uint16_t offset = addr & 0x7FFF;

  switch (map.type) {
      case MMU_MAP_ROM:  return; /* On ne peut pas écrire dans la ROM */
      case MMU_MAP_DISK: cpu->DISK[map.bank][offset] = v; break;
      case MMU_MAP_RAM:  cpu->RAM[map.bank][addr] = v; break;
      default: return;
  }
}
 
static inline uint16_t mem_read16(llmp16_t *cpu, uint16_t addr)
{
   /* little‑endian: LSB first */
   uint8_t lo = mem_read8(cpu, addr);
   uint8_t hi = mem_read8(cpu, addr + 1);
   return (uint16_t)(lo | (hi << 8));
}
 
static inline void mem_write16(llmp16_t *cpu, uint16_t addr, uint16_t v)
{
   mem_write8(cpu, addr,     (uint8_t)(v & 0xFF));
   mem_write8(cpu, addr + 1, (uint8_t)(v >> 8));
}


static inline uint8_t  vram_read(llmp16_t *cpu, uint16_t addr)
{
   return cpu->VRAM[cpu->vbank][addr];
}
 
static inline void vram_write(llmp16_t *cpu, uint16_t addr, uint8_t v)
{
   cpu->VRAM[cpu->vbank][addr] = v;
}
 
static inline void llmp16_reset(llmp16_t *cpu)
{
   memset(cpu, 0, sizeof *cpu);
   cpu->SP   = 0xFFFF;   /* on initialise la pile (descendante)   */
   cpu->PC   = 0x0000;   
   cpu->bank = 0;
   cpu->vbank = 0;
}

 /*============== Routines de fetch/decode/execute ==============*/
 
typedef struct {
   uint8_t  op_class;   
   uint8_t  X;          
   uint8_t  Y;          
   uint8_t  t;          
   bool     has_imm;    
   uint16_t imm;        
   uint16_t raw;        
} instr_t;
 
static inline uint16_t fetch(llmp16_t *cpu)
{
   uint16_t w = mem_read16(cpu, cpu->PC);
   cpu->PC += 2;
   return w;
}

instr_t decode(llmp16_t *cpu, uint16_t instr);
void execute(llmp16_t *cpu, instr_t in);
void llmp16_cpu_cycle(llmp16_t *cpu);


/*=========================== header fichier binaire ROM ===========================*/
#define FILE_CODE 0xFAE1

typedef struct {
  uint16_t code;
  uint8_t nb_pages;
  uint16_t taille_page[LLMP_ROM_BANKS];
}llmp16_header_rom_file_t;

/* chaque fichier binaire entré dans la ROM devra avoir le code FILE_CODE pour etre traité, 
le nombre de pages utilisés dans le fichier, et la taille de chaque page.*/

void llmp16_rom_load(llmp16_t *cpu, char* file);

/*=========================== Lecteur de  disquettes ===========================*/

// Les disquettes sont des périphériques de stockage accéssibles via le lecteur de disquette (port $6 des IOs)
// Elles sont composées de 256 segments de 512 octets
// Le lecteur a 2 modes de fonctionnement : lecture/écriture
// Le lecteur de disques manipule uniquement des blocs (segments) de 512 octects

typedef struct
{
   uint8_t NC;
   uint8_t NH;
   uint8_t NS;
} llmp16_disk_header_t;


typedef struct
{
   uint8_t buffer[512];
   uint8_t nb_segments;
   uint8_t start_segment; //CHS
   FILE *disk;
   bool mode; // 0 : read   1 : write
} llmp16_disk_reader_t;

uint16_t CHS_to_offset(uint16_t CHS);
void llmp16_disk_read_segment(FILE *disk, uint16_t CHS, uint8_t *mem);
void llmp16_disk_write_segment(FILE *disk, uint16_t CHS, uint8_t *mem);
void dump_memory(const uint8_t *mem, size_t size);


#endif // LLMP_VM_H

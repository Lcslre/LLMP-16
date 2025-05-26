#ifndef LLMP_VM_H
#define LLMP_VM_H


#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <SDL2/SDL.h>
/*
*                       Spécifications de la machine virtuelle
*| Paramètre                | Valeur                                            |
*|--------------------------|---------------------------------------------------|
*| Horloge CPU              | 5 MHz                                             |
*| Largeur bus données      | 16 bits                                           |
*| Largeur bus adresses     | 20 bits adressés par octet                        |
*| Registres généraux       | R0–R9  (R15 = ACC), PC, SP, FLAGS                 |
*| RAM                      | 1 Mo                                              |
*| VRAM                     | 128 Ko (320 * 200 * 8bpp) x 2                     |
*| Ports E/S                | 16 ports * 16 registres                           |
*| Endian                   | little‑endian (LSB à l’adresse la plus basse)     |
*/

#define LLMP_MEM_SIZE 0xFFFFF  /* 1 Mo */
#define LLMP_ROM_SIZE 0x8000 /*64 Ko*/
#define LLMP_VRAM_BANKS   2
#define LLMP_VRAM_BANK_SIZE 0x10000  /* 64 Ko */
#define LLMP_IO_PORTS    16
#define LLMP_IO_REGS     16


enum{
   FLAG_N = 0x08, /* Negative (bit 3) */
   FLAG_Z = 0x04, /* Zero     (bit 2) */
   FLAG_C = 0x02, /* Carry    (bit 1) */
   FLAG_V = 0x01  /* oVerflow (bit 0) */
};

typedef struct llmp16_s llmp16_t;


/*
 * Contrôleur DMA pour LLMP16
 * ---------------------------
 * Permet de transférer des blocs mémoire (RAM↔RAM, RAM↔VRAM, DISK↔RAM) sans vm.
 *
 * IO Port 7 (DMA) :
 *   Reg 0 (R0) : SRC_ADDR          (bits 15..0)
 *   Reg 1 (R1) : DST_ADDR          (bits 15..0)
 *   Reg 2 (R2) : COUNT             (nombre d’octets à transférer, bits 15..0)
 *   Reg 3 (R3) : CTRL              (bit 0 = DMA_ENABLE, bit 1 = IRQ_ENABLE)
 *   Reg 4 (R4) : STAT              (bit 0 = BUSY, bit 1 = DONE, bit 2 = ERROR)
 */

#define LLMP_DMA_PORT       7
#define LLMP_DMA_REG_SRC    0
#define LLMP_DMA_REG_DST    1
#define LLMP_DMA_REG_CNT    2
#define LLMP_DMA_REG_CTRL   3
#define LLMP_DMA_REG_STAT   4
#define LLMP_DMA_REG_MAX    5

/* Contrôle */
#define DMA_CTRL_ENABLE     0x01  /* Démarrer le transfert */
#define DMA_CTRL_IRQ_EN     0x02  /* Générer une IRQ à la fin */

/* Statut */
#define DMA_STAT_BUSY       0x01
#define DMA_STAT_DONE       0x02
#define DMA_STAT_ERROR      0x04

typedef struct {
    uint16_t src_addr;   /* Adresse source 16-bits */
    uint16_t dst_addr;   /* Adresse destination 1--bits */
    uint16_t count;      /* Nombre d’octets à copier */
    uint8_t  ctrl;       /* Registre de contrôle */
    uint8_t  stat;       /* Registre de statut */
    bool     irq_line;   /* Ligne IRQ générée */
} llmp16_dma_t;


void llmp16_dma_init(llmp16_dma_t *dma);
void llmp16_dma_cpy(llmp16_t *vm, llmp16_dma_t *dma);
void llmp16_dma_readIO(llmp16_t *vm, llmp16_dma_t *dma);
void llmp16_dma_step(llmp16_t *vm, llmp16_dma_t *dma);

// Numéro de port IO pour le blitter
#define LLMP_BLT_PORT       8

// Registres du blitter (tous 16 bits)
#define LLMP_BLT_REG_SRC    0  // adresse source en mémoire (RAM ou ROM)
#define LLMP_BLT_REG_X      1  // coordonnée X de destination (en pixels, 0..319)
#define LLMP_BLT_REG_Y      2  // coordonnée Y de destination (en pixels, 0..199)
#define LLMP_BLT_REG_W      3  // largeur du bloc (en pixels)
#define LLMP_BLT_REG_H      4  // hauteur du bloc (en pixels)
#define LLMP_BLT_REG_CTRL   5  // contrôle : bit 0 = démarrer

// bit 0 = START, bit 1 = BITMODE (0=copie simple, 1=décompression bit-à-bit)
#define BLT_CTRL_START      0x01
#define BLT_CTRL_BITMODE    0x02


void llmp16_blitter_step(llmp16_t *vm);



// =========================== screen =====================================

#define LLMP_SCREEN_HEIGHT 200
#define LLMP_SCREEN_WIDTH  320
#define LLMP_SCREEN_SCALE 2
#define LLMP_WINDOW_WIDTH LLMP_SCREEN_WIDTH * LLMP_SCREEN_SCALE
#define LLMP_WINDOW_HEIGHT LLMP_SCREEN_HEIGHT * LLMP_SCREEN_SCALE

typedef struct
{
    SDL_Window *window;
    SDL_Renderer* renderer;
    SDL_Texture *framebuffer;
 
}llmp16_screen_t;

int llmp16_screen_init(llmp16_screen_t *screen);
void llmp16_screen_off(llmp16_screen_t *screen);
void llmp16_screen_render(llmp16_screen_t screen, uint8_t** VRAM);


/*=========================== KeyBoard =====================================*/

/*
Le clavier est géré par SDL2. Il faut donc initialiser la bibliothèque SDL2 avant d'utiliser ces fonctions.
On définit le clavier comme une file d'attente de touches. On peut ajouter des touches à la file d'attente avec la fonction llmp16_key_push() 
et les récupérer avec la fonction llmp16_key_pop().

Le clavier correspond au port 2 de la machine virtuelle.
Il y a 2 registres de 16 bits chacun. Le registre 0 contient le code de la touche pressée (0x00 si aucune touche n'est pressée).
Le registre 1 contient le status du clavier (touche pressée ou pas).
*/


void llmp16_keyb_init();

/* La fonction llmp16_keyboard_scan() est appelée à chaque itération de la boucle principale de la machine virtuelle.
   Elle permet de scanner l'état du clavier et d'ajouter les touches pressées à la file d'attente. */
void llmp16_keyboard_scan(llmp16_t *vm);




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

/*============================== Machine Virtuelle ==============================*/

typedef enum {
   R0,
   R1,
   R2,
   R3,
   R4,
   R5,
   R6,
   R7,
   R8,
   R9,
   PC,
   SP,
   IDXH,
   IDXL,
   IDX,
   ACC
} llmp16_register_t;


typedef struct llmp16_s {                      /* R0‑R8 sont des registres généraux */
   uint32_t R[16];                      /*R9-R15 sont des registres spéciaux (Acc, Index_high, Index_low, Index, SP, PC)*/

   uint8_t  FLAGS;                      /* NZCV, bits 3..0    */
   uint8_t  vbank;                       /* Current VRAM bank   */

   bool halted;

   uint8_t  *memory;
   uint8_t  **VRAM; // uint8_t  VRAM[LLMP_VRAM_BANKS][LLMP_VRAM_BANK_SIZE];

   uint64_t clk;

   llmp16_screen_t screen;

   llmp16_dma_t dma;


   llmp16_timer_t timer1;                  /* Timer 1 (16 bits) */
   llmp16_timer_t timer2;                  /* Timer 2 (16 bits) */
   llmp16_timer_t timer3;                  /* Timer 3 (16 bits) */

 
   uint16_t IO[LLMP_IO_PORTS][LLMP_IO_REGS];  /* 16‑bit regs  */

   // llmp16.h, dans votre struct llmp16_t
uint16_t key_buffer;  // dernier code reçu
uint8_t  key_flag;    // =1 si key_buffer contient une donnée non lue


   // Interruptions
   uint16_t int_vector_pending;
   bool int_pending;

} llmp16_t;



void llmp16_run(llmp16_t *vm);
void llmp16_off(llmp16_t *vm);
void llmp16_init(llmp16_t *vm);
 
/*============== Routines de mises à jour des flags ===============*/
 
static inline void flag_set(llmp16_t *vm, uint8_t mask, bool cond)
{
   if (cond) vm->FLAGS |=  mask;
   else vm->FLAGS &= ~mask;
}
 
static inline bool flag_get(const llmp16_t *vm, uint8_t mask)
{
   return (vm->FLAGS & mask) != 0;
}
 
/* Mise à jour des flags N et Z */
static inline void flag_nz(llmp16_t *vm, uint16_t res)
{
   flag_set(vm, FLAG_Z, res == 0); // Si le résultat est nul alors on met le flag Z à 1 sinon on le met à 0
   flag_set(vm, FLAG_N, (res & 0x8000) != 0); // Si le bit de poids fort est à 1 alors on met le flag N à 1 sinon on le met à 0
}
 
static inline void flag_add_cv(llmp16_t *vm, uint16_t a, uint16_t b, uint32_t result32)
{
   flag_set(vm, FLAG_C, result32 > 0xFFFF);
   uint16_t res = (uint16_t)result32;
   /* Overflow si a et b ont le même signe mais le résulat à un singe différent */
   bool ov = (~(a ^ b) & (a ^ res) & 0x8000) != 0;
   flag_set(vm, FLAG_V, ov);
}
 
static inline void flag_sub_cv(llmp16_t *vm, uint16_t a, uint16_t b, uint32_t result32)
{
   flag_set(vm, FLAG_C, result32 & 0x10000);        /* borrow -> carry clear; we invert later */
   uint16_t res = (uint16_t)result32;
   bool ov = ((a ^ b) & (a ^ res) & 0x8000) != 0;
   flag_set(vm, FLAG_V, ov);
}
 
/*============== Routines de manipulation de la mémoire ==============*/

void dump_memory(const uint8_t *mem, size_t size);


// segment 0: 0x0000-0x3FFF, segment 1: 0x4000-0x7FFF, segment 2: 0x8000-0xBFFF, segment 3: 0xC000-0xFFFF
static inline uint8_t mem_read8(llmp16_t *vm, uint32_t addr) {
  return vm->memory[addr];
}

static inline void mem_write8(llmp16_t *vm, uint32_t addr, uint8_t v) {
 
   vm->memory[addr] = v;
}
 
static inline uint16_t mem_read16(llmp16_t *vm, uint32_t addr)
{
   /* little‑endian: LSB first */
   uint8_t lo = mem_read8(vm, addr);
   uint8_t hi = mem_read8(vm, addr + 1);
   return (uint16_t)(lo | (hi << 8));
}
 
static inline void mem_write16(llmp16_t *vm, uint32_t addr, uint16_t v)
{
   mem_write8(vm, addr,     (uint8_t)(v & 0xFF));
   mem_write8(vm, addr + 1, (uint8_t)(v >> 8));
}


static inline uint8_t  vram_read(llmp16_t *vm, uint16_t addr)
{
   return vm->VRAM[vm->vbank][addr];
}
 
static inline void vram_write(llmp16_t *vm, uint16_t addr, uint8_t v)
{
   vm->VRAM[vm->vbank][addr] = v;
}
 
static inline void llmp16_reset(llmp16_t *vm)
{

   vm->FLAGS = 0;
   vm->vbank = 0;
   vm->halted = false;
   memset(vm->R, 0, sizeof(vm->R));
   vm->R[PC] = 0;
   vm->R[SP] = 0xFFFFFF; // Initialisation de la pile
   memset(vm->IO, 0, sizeof(vm->IO));
   memset(vm->memory, 0, LLMP_MEM_SIZE);
   for (int i = 0; i < LLMP_VRAM_BANKS; ++i)
    memset(vm->VRAM[i], 0, LLMP_VRAM_BANK_SIZE);
}

 /*============== Routines de fetch/decode/execute ==============*/
 
typedef struct {
   uint8_t  op_class;   
   uint8_t  X;          
   uint8_t  Y;          
   uint8_t  t;          
   bool     has_imm;
   bool has_addr;    
   uint16_t imm; 
   uint32_t addr;       
   uint16_t raw;        
} instr_t;
 
static inline uint16_t fetch(llmp16_t *vm)
{
   uint16_t w = mem_read16(vm, vm->R[PC]);
   vm->R[PC] += 2;
   return w;
}

instr_t decode(llmp16_t *vm, uint16_t instr);
void execute(llmp16_t *vm, instr_t in);
void llmp16_cpu_cycle(llmp16_t *vm);


/*=========================== header fichier binaire ROM ===========================*/
#define FILE_CODE 0xFAE1


void llmp16_rom_load(llmp16_t *vm, char* file);

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
#include "llmp16.h"
#include <stdio.h>

instr_t decode(llmp16_t *cpu, uint16_t instr)
{
    instr_t d;
    d.raw      = instr;
    d.op_class = (d.raw >> 12) & 0x0F;
    d.X        = (d.raw >>  8) & 0x0F;
    d.Y        = (d.raw >>  4) & 0x0F;
    d.t        =  d.raw        & 0x0F;
    d.has_imm  = false;
    d.imm      = 0;
 
    /* 0x2 (arith imm),
    * 0x4 (logic imm), 0xA (memory imm),
    * 0x7 (jumps imm16)                                           */
    if (d.op_class == 0x2 || d.op_class == 0x4 ||
        d.op_class == 0x7 || d.op_class == 0xA)
    {
        d.has_imm = true;
        d.imm     = fetch(cpu);
    }
 
    return d;
}
 
void execute(llmp16_t *cpu, instr_t in)
{
     switch (in.op_class)
     {
     /* ========= 0x0 – Specials ======== */
     case 0x0:
        switch (in.raw)
        {
        case 0x0000:  /* NOP */
            break;
        case 0x0001:  /* HALT */
            cpu->PC -= 2;
            break;
        default: 
            break;
        }
        break;
 
     /* ========= 0x1 – Arithmetique registre‑registre =============== */
     case 0x1:
        switch (in.t)
        {
        case 0x0: { /* ADD  R15 <- RX + RY  */
            uint16_t a = cpu->R[in.X];
            uint16_t b = cpu->R[in.Y];
            uint32_t r32 = (uint32_t)a + b;
            uint16_t r16 = (uint16_t)r32;
            cpu->R[15] = r16;
            flag_nz(cpu, r16);
            flag_add_cv(cpu, a, b, r32);
            break;
        }
        case 0x1: { /* SUB  R15 <- RX - RY */
            uint16_t a = cpu->R[in.X];
            uint16_t b = cpu->R[in.Y];
            uint32_t r32 = (uint32_t)a - b + 0x10000;
            uint16_t r16 = (uint16_t)r32;
            cpu->R[15] = r16;
            flag_nz(cpu, r16);
            flag_sub_cv(cpu, a, b, r32);
            break;
        }
        case 0x2: { /* MUL */
            uint32_t r32 = (uint32_t)cpu->R[in.X] * cpu->R[in.Y];
            uint16_t r16 = (uint16_t)r32;
            cpu->R[15] = r16;
            flag_nz(cpu, r16);
            flag_set(cpu, FLAG_C | FLAG_V, false);
            break;
        }
        case 0x3: { /* DIV (unsigned) */
            uint16_t denom = cpu->R[in.Y];
            if (denom == 0) {
                break;
            }
            uint16_t res = cpu->R[in.X] / denom;
            cpu->R[15] = res;
            flag_nz(cpu, res);
            break;
        }
        case 0x5: { /* DEC */
            uint16_t res = cpu->R[in.X] - 1;
            cpu->R[in.X] = res;
            flag_nz(cpu, res);
            break;
        }
        case 0x4: { /* SDIV – signed division (optional) */
            break; // TODO
        }
        case 0x6: { /* CMP */
            uint16_t a = cpu->R[in.X];
            uint16_t b = cpu->R[in.Y];
            uint32_t r32 = (uint32_t)a - b + 0x10000;
            uint16_t r16 = (uint16_t)r32;
            flag_nz(cpu, r16);
            flag_sub_cv(cpu, a, b, r32);
            break;
        }
        case 0x8: { /* LSR X,Y  */
            uint8_t shift = cpu->R[in.Y] & 0xF;
            uint16_t val = cpu->R[in.X];
            uint16_t res = val >> shift;
            flag_set(cpu, FLAG_C, (val >> (shift - 1)) & 0x1);
            cpu->R[in.X] = res;
            flag_nz(cpu, res);
            break;
        }
        case 0x9: { /* ASR X,Y  */
            uint8_t shift = cpu->R[in.Y] & 0xF;
            int16_t val = (int16_t)cpu->R[in.X];
            int16_t res = val >> shift;
            flag_set(cpu, FLAG_C, (uint16_t)val >> (shift - 1) & 0x1);
            cpu->R[in.X] = (uint16_t)res;
            flag_nz(cpu, (uint16_t)res);
            break;
        }
        case 0xA: { /* LSL X,Y – logical shift left */
            uint8_t shift = cpu->R[in.Y] & 0xF;
            uint16_t val = cpu->R[in.X];
            uint16_t res = val << shift;
            flag_set(cpu, FLAG_C, (val >> (16 - shift)) & 0x1);
            cpu->R[in.X] = res;
            flag_nz(cpu, res);
            break;
        }
        default:
            break;
        }
        break;
 
     /* ========= 0x2 – Arithmetique immediate ================ */
     case 0x2:
        switch (in.t)
        {
        case 0x0: { /* ADDI */
            uint16_t a = cpu->R[in.X];
            uint16_t b = in.imm;
            uint32_t r32 = (uint32_t)a + b;
            uint16_t r16 = (uint16_t)r32;
            cpu->R[15] = r16;
            flag_nz(cpu, r16);
            flag_add_cv(cpu, a, b, r32);
            break;
        }
        case 0x1: { /* SUBI */
            uint16_t a = cpu->R[in.X];
            uint16_t b = in.imm;
            uint32_t r32 = (uint32_t)a - b + 0x10000;
            uint16_t r16 = (uint16_t)r32;
            cpu->R[15] = r16;
            flag_nz(cpu, r16);
            flag_sub_cv(cpu, a, b, r32);
            break;
        }
        case 0x2: { /* MULI */
            uint32_t r32 = (uint32_t)cpu->R[in.X] * in.imm;
            uint16_t r16 = (uint16_t)r32;
            cpu->R[15] = r16;
            flag_nz(cpu, r16);
            break;
        }
        case 0x3: { /* DIVI */
            if (in.imm == 0) break;
            uint16_t res = cpu->R[in.X] / in.imm;
            cpu->R[15] = res;
            flag_nz(cpu, res);
            break;
        }
        case 0x5: { /* CMPI */
            uint16_t a = cpu->R[in.X];
            uint16_t b = in.imm;
            uint32_t r32 = (uint32_t)a - b + 0x10000;
            flag_nz(cpu, (uint16_t)r32);
            flag_sub_cv(cpu, a, b, r32);
            break;
        }
        default:
            break;
        }
        break;
 
     /* ========= 0x3 – Logical reg‑reg ===================== */
     case 0x3:
        switch (in.t)
        {
        case 0x0: { /* AND */
            uint16_t res = cpu->R[in.X] & cpu->R[in.Y];
            cpu->R[15] = res;
            flag_nz(cpu, res);
            break;
        }
        case 0x1: { /* OR */
            uint16_t res = cpu->R[in.X] | cpu->R[in.Y];
            cpu->R[15] = res;
            flag_nz(cpu, res);
            break;
        }
        case 0x2: { /* XOR */
            uint16_t res = cpu->R[in.X] ^ cpu->R[in.Y];
            cpu->R[15] = res;
            flag_nz(cpu, res);
            break;
        }
        case 0x3: { /* NOT X */
            uint16_t res = ~cpu->R[in.X];
            cpu->R[in.X] = res;
            flag_nz(cpu, res);
            break;
        }
        case 0x4: { /* TST X Y -> flags on RX & RY */
            uint16_t res = cpu->R[in.X] & cpu->R[in.Y];
            flag_nz(cpu, res);
            break;
        }
         default:
            break;
        }
        break;
 
     /* ========= 0x4 – Logical immediate ================== */
     case 0x4:
         switch (in.t)
         {
         case 0x0: { /* ANDI */
            uint16_t res = cpu->R[in.X] & in.imm;
            cpu->R[15] = res;
            flag_nz(cpu, res);
            break;
         }
         case 0x1: { /* ORI */
            uint16_t res = cpu->R[in.X] | in.imm;
            cpu->R[15] = res;
            flag_nz(cpu, res);
            break;
         }
         case 0x2: { /* XORI */
            uint16_t res = cpu->R[in.X] ^ in.imm;
            cpu->R[15] = res;
            flag_nz(cpu, res);
            break;
         }
         case 0x3: { /* TSTI */
            uint16_t res = cpu->R[in.X] & in.imm;
            flag_nz(cpu, res);
            break;
        }
        default:
            break;
        }
        break;
 
     /* ========= 0x5 – Memory controls reg‑reg ============= */
     case 0x5:
        switch (in.t)
        {
        case 0x0: /* MOV */
            cpu->R[in.X] = cpu->R[in.Y];
            break;
        case 0x1: /* LD RX <- MEM[RY] */
            cpu->R[in.X] = mem_read16(cpu, cpu->R[in.Y]);
            break;
        case 0x2: /* STR MEM[RX] <- RY */
            mem_write16(cpu, cpu->R[in.X], cpu->R[in.Y]);
            break;
        case 0x3: /* PUSH RX */
            cpu->SP -= 2;
            mem_write16(cpu, cpu->SP, cpu->R[in.X]);
            break;
        case 0x4: /* POP RX */
            cpu->R[in.X] = mem_read16(cpu, cpu->SP);
            cpu->SP += 2;
            break;
        case 0x5: /*VLOAD*/
            cpu->R[in.X] = vram_read(cpu, cpu->R[in.Y]);
            break;
        case 0x6: /*VSTORE*/
            vram_write(cpu, cpu->R[in.X], cpu->R[in.Y]);
            break;
        default:
            break;
        }
        break;
 
     /* ========= 0x6 – jumps === */
     case 0x6: {
        bool take = false;
        switch (in.t)
        {
            case 0x0: take = true; break;                     /* JUMP  */
            case 0x1: take = flag_get(cpu, FLAG_Z); break;     /* JEQ   */
            case 0x2: take = !flag_get(cpu, FLAG_Z); break;    /* JNE   */
            case 0x3: take = flag_get(cpu, FLAG_C); break;     /* JCS   */
            case 0x4: take = !flag_get(cpu, FLAG_C); break;    /* JCC   */
            case 0x5: take = flag_get(cpu, FLAG_N); break;     /* JNS   */
            case 0x6: take = !flag_get(cpu, FLAG_N); break;    /* JNC   */
            case 0x7: take = flag_get(cpu, FLAG_V); break;     /* JVS   */
            case 0x8: take = !flag_get(cpu, FLAG_V); break;    /* JVC   */
            default:
                break;
        }
        if (take) cpu->PC = cpu->R[in.X];
        break;
     }
 
     /* ========= 0x7 – Jump / call with immediate ========== */
     case 0x7: {
         bool take = false;
         switch (in.t)
         {
         case 0x0: take = true; break;                     /* JUMP imm */
         case 0x1: take = flag_get(cpu, FLAG_Z); break;    /* JEQ      */
         case 0x2: take = !flag_get(cpu, FLAG_Z); break;   /* JNE      */
         case 0x3: take = flag_get(cpu, FLAG_C); break;    /* JCS      */
         case 0x4: take = !flag_get(cpu, FLAG_C); break;   /* JCC      */
         case 0x5: take = flag_get(cpu, FLAG_N); break;    /* JNS      */
         case 0x6: take = !flag_get(cpu, FLAG_N); break;   /* JNC      */
         case 0x7: take = flag_get(cpu, FLAG_V); break;    /* JVS      */
         case 0x8: take = !flag_get(cpu, FLAG_V); break;   /* JVC      */
         case 0x9: /* CALL */
            cpu->SP -= 2;
            mem_write16(cpu, cpu->SP, cpu->PC);
            cpu->PC = in.imm;
            break;
         default:
            break;
        }
        if (take) cpu->PC = in.imm;
        break;
     }
 
     /* ========= 0x8 – IN RX, port(Y, t) ================== */
     case 0x8: {
        uint8_t port = in.Y;
        uint8_t reg  = in.t;
        cpu->R[in.X] = cpu->IO[port][reg];
        printf("Port : %d\n Reg : %d\n", port, reg);
        printf("R : %d\n", cpu->R[4]);
        break;
     }
 
     /* ========= 0x9 – OUT RX -> port(Y,t) ================ */
     case 0x9: {
        uint8_t port = in.Y;
        uint8_t reg  = in.t;
        cpu->IO[port][reg] = cpu->R[in.X];
 
         /* sélection d'une banque : OUT Rx, $0000 */
        // if (port == 0x00 && reg == 0x00) {
        //     //cpu->bank = cpu->R[in.X] & 0xFF;
        // }

        /* sélection d'une banque de VRAM: OUT Rx, $0001 */
        if (port == 0x00 && reg == 0x01) {
            cpu->vbank = cpu->R[in.X] & 0x01;
        }
        break;
    }

     case 0xA:
        switch (in.t)
         {
        case 0x0: /* MOVI */
            cpu->R[in.X] = in.imm;
            break;
        case 0x1: /* LDI */
            cpu->R[in.X] = mem_read16(cpu, in.imm);
            break;
        case 0x2: /* STRI MEM[imm] <- RX */
            mem_write16(cpu, in.imm, cpu->R[in.X]);
            break;
        case 0x3: /* VLOAD */
            cpu->R[in.X] = vram_read(cpu, in.imm);
            break;
        case 0x4: /* VSTORE */
            vram_write(cpu, in.imm, cpu->R[in.X] & 0xFF);
            break;
        default:
            break;
        } 
        break;
 
     default:
        break;
     }
}

void llmp16_cpu_cycle(llmp16_t *cpu)
{
    uint16_t instr = fetch(cpu);
    instr_t in = decode(cpu, instr);
    execute(cpu, in);
}

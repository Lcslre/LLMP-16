#include "llmp16.h"
#include <stdio.h>

instr_t decode(llmp16_t *vm, uint16_t instr)
{
    instr_t d;
    d.raw      = instr;
    d.op_class = (d.raw >> 12) & 0x0F;
    d.X        = (d.raw >>  8) & 0x0F;
    d.Y        = (d.raw >>  4) & 0x0F;
    d.t        =  d.raw        & 0x0F;
    d.has_imm  = false;
    d.has_addr = false;
    d.imm      = 0;
    d.addr = 0;
 
    /* 0x2 (arith imm), 0x4 (logic imm)*/                                           
    if (d.op_class == 0x2 || d.op_class == 0x4 || (d.op_class == 0x6 && (d.t == 0 || d.t == 3 || d.t == 5 || d.t == 6)))
    {
        d.has_imm = true;
        d.imm     = fetch(vm);
    }

    /* 0x6 (memory imm), 0x8 (jumps imm16) */
    if(d.op_class == 0x8 || (d.op_class == 0x6 && (d.t == 1 || d.t == 2)))
    {
        d.has_addr = true;
        
        d.addr = ((d.raw & 0x00F0) << 12) + fetch(vm); 
    }

    return d;
}
 
void execute(llmp16_t *vm, instr_t in)
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
            llmp16_reg_set(vm, PC, llmp16_reg_get(vm, PC)-2);
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
            uint16_t a = llmp16_reg_get(vm, in.X);
            uint16_t b = llmp16_reg_get(vm, in.Y);
            uint32_t r32 = (uint32_t)a + b;
            uint16_t r16 = (uint16_t)r32;
            llmp16_reg_set(vm, ACC, r16);
            flag_nz(vm, r16);
            flag_add_cv(vm, a, b, r32);
            break;
        }
        case 0x1: { /* SUB  R15 <- RX - RY */
            uint16_t a = llmp16_reg_get(vm, in.X);
            uint16_t b = llmp16_reg_get(vm, in.Y);
            uint32_t r32 = (uint32_t)a - b + 0x10000;
            uint16_t r16 = (uint16_t)r32;
            llmp16_reg_set(vm, ACC, r16);
            flag_nz(vm, r16);
            flag_sub_cv(vm, a, b, r32);
            break;
        }
        case 0x2: { /* MUL */
            uint32_t r32 = (uint32_t)llmp16_reg_get(vm, in.X) * llmp16_reg_get(vm, in.Y);
            uint16_t r16 = (uint16_t)r32;
            llmp16_reg_set(vm, ACC, r16);
            flag_nz(vm, r16);
            flag_set(vm, FLAG_C | FLAG_V, false);
            break;
        }
        case 0x3: { /* DIV (unsigned) */
            uint16_t denom = llmp16_reg_get(vm, in.Y);
            if (denom == 0) {
                break;
            }
            uint16_t res = llmp16_reg_get(vm, in.X) / denom;
            llmp16_reg_set(vm, ACC, res);
            flag_nz(vm, res);
            break;
        }
        case 0x4: { /* SDIV – signed division (optional) */
            break; // TODO
        }
        case 0x5: { /* INC */
            uint16_t res = llmp16_reg_get(vm, in.X) + 1;
            llmp16_reg_set(vm, in.X, res);
            flag_nz(vm, res);
            break;
        }
        case 0x6: { /* DEC */
            uint16_t res = llmp16_reg_get(vm, in.X) - 1;
            llmp16_reg_set(vm, in.X, res);
            flag_nz(vm, res);
            break;
        }
        case 0x7: { /* CMP */
            uint16_t a = llmp16_reg_get(vm, in.X);
            uint16_t b = llmp16_reg_get(vm, in.Y);
            uint32_t r32 = (uint32_t)a - b + 0x10000;
            uint16_t r16 = (uint16_t)r32;
            flag_nz(vm, r16);
            flag_sub_cv(vm, a, b, r32);
            break;
        }
        case 0x8: { /* LSR X,Y  */
            uint8_t shift = llmp16_reg_get(vm, in.Y) & 0xF;
            uint16_t val = llmp16_reg_get(vm, in.X);
            uint16_t res = val >> shift;
            flag_set(vm, FLAG_C, (val >> (shift - 1)) & 0x1);
            llmp16_reg_set(vm, in.X, res);
            flag_nz(vm, res);
            break;
        }
        case 0x9: { /* ASR X,Y  */
            uint8_t shift = llmp16_reg_get(vm, in.Y) & 0xF;
            int16_t val = (int16_t)llmp16_reg_get(vm, in.X);
            int16_t res = val >> shift;
            flag_set(vm, FLAG_C, (uint16_t)val >> (shift - 1) & 0x1);
            llmp16_reg_set(vm, in.X, (uint16_t)res);
            flag_nz(vm, (uint16_t)res);
            break;
        }
        case 0xA: { /* LSL X,Y – logical shift left */
            uint8_t shift = llmp16_reg_get(vm, in.Y) & 0xF;
            uint16_t val = llmp16_reg_get(vm, in.X);
            uint16_t res = val << shift;
            flag_set(vm, FLAG_C, (val >> (16 - shift)) & 0x1);
            llmp16_reg_set(vm, in.X, res);
            flag_nz(vm, res);
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
            uint16_t a = llmp16_reg_get(vm, in.X);
            uint16_t b = in.imm;
            uint32_t r32 = (uint32_t)a + b;
            uint16_t r16 = (uint16_t)r32;
            llmp16_reg_set(vm, ACC, r16);
            flag_nz(vm, r16);
            flag_add_cv(vm, a, b, r32);
            break;
        }
        case 0x1: { /* SUBI */
            uint16_t a = llmp16_reg_get(vm, in.X);
            uint16_t b = in.imm;
            uint32_t r32 = (uint32_t)a - b + 0x10000;
            uint16_t r16 = (uint16_t)r32;
            llmp16_reg_set(vm, ACC, r16);
            flag_nz(vm, r16);
            flag_sub_cv(vm, a, b, r32);
            break;
        }
        case 0x2: { /* MULI */
            uint32_t r32 = (uint32_t)llmp16_reg_get(vm, in.X) * in.imm;
            uint16_t r16 = (uint16_t)r32;
            llmp16_reg_set(vm, ACC, r16);
            flag_nz(vm, r16);
            break;
        }
        case 0x3: { /* DIVI */
            if (in.imm == 0) break;
            uint16_t res = llmp16_reg_get(vm, in.X) / in.imm;
            llmp16_reg_set(vm, ACC, res);
            flag_nz(vm, res);
            break;
        }
        case 0x7: { /* CMPI */
            uint16_t a = llmp16_reg_get(vm, in.X);
            uint16_t b = in.imm;
            uint32_t r32 = (uint32_t)a - b + 0x10000;
            uint16_t r16 = (uint16_t)r32;
            flag_nz(vm, r16);
            flag_sub_cv(vm, a, b, r32);
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
            uint16_t res = llmp16_reg_get(vm, in.X) & llmp16_reg_get(vm, in.Y);
            llmp16_reg_set(vm, ACC, res);
            flag_nz(vm, res);
            break;
        }
        case 0x1: { /* OR */
            uint16_t res = llmp16_reg_get(vm, in.X) | llmp16_reg_get(vm, in.Y);
            llmp16_reg_set(vm, ACC, res);
            flag_nz(vm, res);
            break;
        }
        case 0x2: { /* XOR */
            uint16_t res = llmp16_reg_get(vm, in.X) ^ llmp16_reg_get(vm, in.Y);
            llmp16_reg_set(vm, ACC, res);
            flag_nz(vm, res);
            break;
        }
        case 0x3: { /* NOT X */
            uint16_t res = ~llmp16_reg_get(vm, in.X);
            llmp16_reg_set(vm, ACC, res);
            flag_nz(vm, res);
            break;
        }
        case 0x4: { /* TST X Y -> flags on RX & RY */
            uint16_t res = llmp16_reg_get(vm, in.X) & llmp16_reg_get(vm, in.Y);
            flag_nz(vm, res);
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
            uint16_t res = llmp16_reg_get(vm, in.X) & in.imm;
            llmp16_reg_set(vm, ACC, res);
            flag_nz(vm, res);
            break;
         }
         case 0x1: { /* ORI */
            uint16_t res = llmp16_reg_get(vm, in.X) | in.imm;
            llmp16_reg_set(vm, ACC, res);
            flag_nz(vm, res);
            break;
         }
         case 0x2: { /* XORI */
            uint16_t res = llmp16_reg_get(vm, in.X) ^ in.imm;
            llmp16_reg_set(vm, ACC, res);
            flag_nz(vm, res);
            break;
         }
         case 0x3: { /* TSTI */
            uint16_t res = llmp16_reg_get(vm, in.X) & in.imm;
            flag_nz(vm, res);
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
            llmp16_reg_set(vm, in.X, llmp16_reg_get(vm, in.Y));
            break;
        case 0x1: /* LD RX <- MEM[RY] */
            llmp16_reg_set(vm, in.X, mem_read16(vm, llmp16_reg_get(vm, in.Y)));
            break;
        case 0x2: /* STR MEM[RX] <- RY */
            mem_write16(vm, llmp16_reg_get(vm, in.X), llmp16_reg_get(vm, in.Y));
            break;
        case 0x3: /* PUSH RX */
            llmp16_reg_set(vm, SP, llmp16_reg_get(vm, SP)-2);
            mem_write16(vm, llmp16_reg_get(vm, SP), llmp16_reg_get(vm, in.X));
            break;
        case 0x4: /* POP RX */
            llmp16_reg_set(vm, in.X, mem_read16(vm, llmp16_reg_get(vm, SP)));
            llmp16_reg_set(vm, SP, llmp16_reg_get(vm, SP)+2);
            break;
        case 0x5: /*VLOAD*/
            llmp16_reg_set(vm, in.X, vram_read(vm, llmp16_reg_get(vm, in.Y)));
            break;
        case 0x6: /*VSTORE*/
            vram_write(vm, llmp16_reg_get(vm, in.X), llmp16_reg_get(vm, in.Y));
            break;
        default:
            break;
        }
        break;
    
    case 0x6:
        switch (in.t)
         {
        case 0x0: /* MOVI */
            llmp16_reg_set(vm, in.X, in.imm);
            break;
        case 0x1: /* LDI */
            llmp16_reg_set(vm, in.X, mem_read16(vm, in.addr));
            break;
        case 0x2: /* STRI MEM[imm] <- RX */
            mem_write16(vm, in.addr, llmp16_reg_get(vm, in.X));
            break;
        case 0x3: /* PUSHI Imm */
            llmp16_reg_set(vm, SP, llmp16_reg_get(vm, SP)-2);
            mem_write16(vm, llmp16_reg_get(vm, SP), in.imm);
            break;
        case 0x5: /* VLOADI */
            llmp16_reg_set(vm, in.X, vram_read(vm, in.imm));
            break;
        case 0x6: /* VSTOREI */
            vram_write(vm, in.imm, llmp16_reg_get(vm, in.X) & 0xFF);
            break;
        default:
            break;
        } 
        break;
 
     /* ========= 0x7 – jumps === */
     case 0x7: {
        bool take = false;
        switch (in.t)
        {
            case 0x0: take = true; break;                     /* JUMP  */
            case 0x1: take = flag_get(vm, FLAG_Z); break;     /* JEQ   */
            case 0x2: take = !flag_get(vm, FLAG_Z); break;    /* JNE   */
            case 0x3: take = flag_get(vm, FLAG_C); break;     /* JCS   */
            case 0x4: take = !flag_get(vm, FLAG_C); break;    /* JCC   */
            case 0x5: take = flag_get(vm, FLAG_V); break;     /* JVS   */
            case 0x6: take = !flag_get(vm, FLAG_V); break;    /* JVC   */
            case 0x7: take = ((flag_get(vm, FLAG_N) == flag_get(vm, FLAG_V)) && !flag_get(vm, FLAG_Z)); break; /*JGT*/
            case 0x8: take = (flag_get(vm, FLAG_N) != flag_get(vm, FLAG_V)); break; /*JLT*/
            case 0x9: take = (flag_get(vm, FLAG_N) == flag_get(vm, FLAG_V)); break; /*JGE*/
            case 0xA: take = ((flag_get(vm, FLAG_N) != flag_get(vm, FLAG_V)) && flag_get(vm, FLAG_Z)); break; /*JLE*/
            case 0xB: take = (!flag_get(vm, FLAG_C) && !flag_get(vm, FLAG_Z)); break; /*JHI*/
            case 0xC: take = (flag_get(vm, FLAG_C) && flag_get(vm, FLAG_Z)); break; /*JLS*/
            default:
                break;
        }
        if (take) llmp16_reg_set(vm, PC, llmp16_reg_get(vm, in.X));
        break;
     }
 
     /* ========= 0x8 – Jump / call with immediate ========== */
     case 0x8: {
         bool take = false;
         switch (in.t)
         {
            case 0x0: take = true; break;                     /* JUMP  */
            case 0x1: take = flag_get(vm, FLAG_Z); ; break;     /* JEQ   */
            case 0x2: take = !flag_get(vm, FLAG_Z); break;    /* JNE   */
            case 0x3: take = flag_get(vm, FLAG_C); break;     /* JCS   */
            case 0x4: take = !flag_get(vm, FLAG_C); break;    /* JCC   */
            case 0x5: take = flag_get(vm, FLAG_V); break;     /* JVS   */
            case 0x6: take = !flag_get(vm, FLAG_V); break;    /* JVC   */
            case 0x7: take = ((flag_get(vm, FLAG_N) == flag_get(vm, FLAG_V)) && !flag_get(vm, FLAG_Z)); break; /*JGT*/
            case 0x8: take = (flag_get(vm, FLAG_N) != flag_get(vm, FLAG_V)); break; /*JLT*/
            case 0x9: take = (flag_get(vm, FLAG_N) == flag_get(vm, FLAG_V)); break; /*JGE*/
            case 0xA: take = ((flag_get(vm, FLAG_N) != flag_get(vm, FLAG_V)) && flag_get(vm, FLAG_Z)); break; /*JLE*/
            case 0xB: take = (!flag_get(vm, FLAG_C) && !flag_get(vm, FLAG_Z)); break; /*JHI*/
            case 0xC: take = (flag_get(vm, FLAG_C) && flag_get(vm, FLAG_Z)); break; /*JLS*/
            case 0xD: /* CALL */
                llmp16_reg_set(vm, SP, llmp16_reg_get(vm, SP)-2);
                mem_write16(vm, llmp16_reg_get(vm, SP), llmp16_reg_get(vm, PC));
                llmp16_reg_set(vm, PC, in.addr);
                break;
            default:
                break;
        }
        if (take){
        
         llmp16_reg_set(vm, PC, in.addr);
        }
        break;
     }
 
     /* ========= 0x8 – IN RX, port(Y, t) ================== */
     case 0x9: {
        uint8_t port = in.Y;
        uint8_t reg  = in.t;
        llmp16_reg_set(vm, in.X, vm->IO[port][reg]);
        if (port == 1) {
        // on consomme la donnée clavier
        vm->IO[1][0]   = 0;
        }
        break;
     }
 
     /* ========= 0x9 – OUT RX -> port(Y,t) ================ */
     case 0xA: {
        uint8_t port = in.Y;
        uint8_t reg  = in.t;
        vm->IO[port][reg] = llmp16_reg_get(vm, in.X);
 

        /* sélection d'une banque de VRAM: OUT Rx, $0001 */
        if (port == 0x00 && reg == 0x01) {
            vm->vbank = llmp16_reg_get(vm, in.X) & 0x01;
        }
        break;
    }
     default:
        break;
     }
}

void llmp16_cpu_cycle(llmp16_t *vm)
{
    uint16_t instr = fetch(vm);
    instr_t in = decode(vm, instr);
    execute(vm, in);
}

# LLMP-16
Machine virtuel


## Les ports d'entrées/sorties

Il y a 16 ports($0 - $F) et chaque port a 16 registres de configurations au maximum ($0 - $F)
Pour accéder aux registres de configurations il faut utiliser les instructions IN/OUT

| Ports | Périphériques | registre 0 | registre 1 | registre 2 | registre 3 |
| :---: | :---: | :---: | :---: | :---: | :---: |
| $0 | MMU | Type de mémoire du segment 1 ($0000 - $7FFF) | choix de la banque | type de mémoire du segment 2 ($8000 - $FFFF) | choix de la banque |
| $1 |     Ecran     | choix de ROM | choix de la VRAM | - |
| $2 |    clavier    | code de la touche pressée | registre de status | - | - |
| $3 |    Timer 1    | PSC |           INIT VALUE           | status | - |
| $4 |    Timer 2    | TODO |           TODO           | - | - |
| $5 |    Timer 3    | TODO |           TODO           | - | - |


## Le jeu d’instructions

   1. ## **Les instructions Arithmétiques** {#les-instructions-arithmétiques}

| opcode | description | taille (mots) | format | flags |
| :---: | :---: | :---: | :---: | :---: |
| ADD X Y | R15 \<- RX \+ RY | 1 | 0x1XY0 | N Z C V |
| SUB X Y | R15 \<- RX \- RY | 1 | 0x1XY1 | N Z C V |
| MUL X Y | R15 \<- RX \* RY | 1 | 0x1XY2 | N Z |
| DIV X Y | R15 \<- RX / RY | 1 | 0x1XY3 | N Z |
| SDIV X Y |  | 1 | 0x1XY4 | N Z |
| INC X | RX \<- RX \+ 1 | 1 | 0x1X05 | N Z |
| DEC X | RX \<- RX \+ 1 | 1 | 0x1X06 | N Z |
| CMP X Y | RX \- RY et met à jour NZCV | 1 | 0x1XY7 | N Z C V |
| LSR X Y | RX \>\> RY  | 1 | 0x1XY8 | N Z C |
| ASR X Y | RX \>\> RY | 1 | 0x1XY9 | N Z C |
| LSL X Y | RX \<\< RY | 1 | 0x1XYA | N Z C |
| ADDI X imm16 | R15 \<- RX \+ imm | 2 | 0x2X00 0xnnnn | N Z C V |
| SUBI X imm16 | R15 \<- RX \- imm | 2 | 00x2X01 0xnnnn | N Z C V |
| MULI X imm16 | R15 \<- RX \* imm | 2 | 00x2X02 0xnnnn | N Z |
| DIVI X imm16 | R15 \<- RX / imm | 2 | 0x2X03 0xnnnn | N Z |
| SDIVI X imm16 |  | 2 | 0x2X04 0xnnnn |  |
| CMPI X imm16 | RX \- imm et met à jour NZCV | 2 | 0x2X05 0xnnnn | N Z C V |
| LSRI X imm16 | RX \>\> imm | 2 | 0x2X06 0xnnnn | N Z |
| ASRI X imm16 | RX \>\> imm | 2 | 0x2X07 0xnnnn | N Z |
| LSLI | RX \<\< imm | 2 | 0x2X08 0xnnnn | N Z |

      

   2. ## **Les instructions Logiques** {#les-instructions-logiques}

| opcode | description | taille (mots) | format | flags |
| :---: | :---: | :---: | :---: | :---: |
| AND X Y | R15 ← RX & RY | 1 | 0x3XY0 | N Z |
| OR X Y | R15 ← RX | RY | 1 | 0x3XY1 | N Z |
| XOR X Y | R15 ←RX ^ RY | 1 | 0x3XY2 | N Z |
| NOT X | RX ← \~RX | 1 | 0x30Y3 | N Z |
| TST X Y | met à jour NZCV en fonction de RX & RY | 1 | 0x3XY4 | N Z |
| ANDI X imm16 | R15 ← RX & imm | 2 | 0x4X00 0xnnnn | N Z |
| ORI X imm16 | R15 ← RX | imm | 2 | 0x4X01 0xnnnn  | N Z |
| XORI X imm16 | R15 ← RX ^ imm | 2 | 0x4X02 0xnnnn  | N Z |
| TSTI X imm16 | met à jour NZCV en fonction de RX & imm | 2 | 0x4003 0xnnnn  | N Z |

      

   3. ## **Les instructions de contrôles mémoire** {#les-instructions-de-contrôles-mémoire}

| opcode | description | taille (mots) | format | flags |
| :---: | :---: | :---: | :---: | :---: |
| MOV X Y | RX \<- RY | 1 | 0x5XY0 | \- |
| LD X Y | RX \<- MEM\[RY\] | 1 | 0x5XY1 | \- |
| STR X Y | MEM\[RX\] \<- RY | 1 | 0x5XY2 | \- |
| VLD X Y | RX \<- MEM\[RY\] | 1 | 0x5XY5 | \- |
| VSTR X Y | MEM\[RX\] \<- RY | 1 | 0x5XY6 | \- |
| MOVI X imm16 | RX \<- imm16 | 2 | 0XAX00 0xnnnn | \- |
| LDI X imm16 | RX \<- MEM\[imm16\] | 2 | 0XAX01 0xnnnn | \- |
| STRI X imm16 | MEM\[imm16\] \<- RX | 2 | 0XAX02 0xnnnn | \- |
| VLDI X imm16 |  RX \<- VRAM\[imm16\] | 2 | 0XAX03 0xnnnn | \- |
| VSTRI X imm16 | VRAM\[imm16\] \<- RX | 2 | 0XAX04 0xnnnn | \- |
| PUSH X | MEM\[--SP\] \<- RX  | 1 | 0x5X03 | \- |
| POP Y | RX \<- MEM\[SP++\] | 1 | 0x5X04 | \- |

   4. ## **Les instructions de sauts** {#les-instructions-de-sauts}

| opcode | description | taille (mots) | format | flags |
| :---: | :---: | :---: | :---: | :---: |
| JUMP X | PC ←RX | 1 | 0x6X00 | \- |
| JEQ X | Si Z \= 1 PC ←RX | 1 | 0x6X01 | \- |
| JNE X | Si Z \= 0 PC ←RX | 1 | 0x6X02 | \- |
| JCS X | Si C \= 1 PC ←RX | 1 | 0x6X03 | \- |
| JCC X | Si C \= 0 PC ←RX | 1 | 0x6X04 | \- |
| JNS X | Si N \= 1 PC ←RX | 1 | 0x6X05 | \- |
| JNC X | Si N \= 0 PC ←RX | 1 | 0x6X06 | \- |
| JVS X | Si V \= 1 PC ←RX | 1 | 0x6X07 | \- |
| JVC X | Si V \= 0 PC ←RX | 1 | 0x6X08 | \- |
| JUMP imm16 | PC ←imm | 2 | 0x7000 0xnnnn | \- |
| JEQ imm16 | Si Z \= 1 PC ←imm | 2 | 0x7001 0xnnnn | \- |
| JNE imm16 | Si Z \= 0 PC ←imm | 2 | 0x7002 0xnnnn | \- |
| JCS imm16 | Si C \= 1 PC ←imm | 2 | 0x7003 0xnnnn | \- |
| JCC imm16 | Si C \= 0 PC ←imm | 2 | 0x7004 0xnnnn | \- |
| JNS imm16 | Si N \= 1 PC ←imm | 2 | 0x7005 0xnnnn | \- |
| JNC imm16 | Si N \= 0 PC ←imm | 2 | 0x7006 0xnnnn | \- |
| JVS imm16 | Si V \= 1 PC ←imm | 2 | 0x7007 0xnnnn | \- |
| JVC imm16 | Si V \= 0 PC ←imm | 2 | 0x7008 0xnnnn | \- |
| CALL imm16 | PC ←imm PUSH PC | 2 | 0x7009 0xnnnn | \- |
| RET | POP PC | 1 | 0x6X09 | \- |

      

      

   5. ## **Les instructions spéciales** {#les-instructions-spéciales}

| opcode | description | taille (mots) | format | flags |
| :---: | :---: | :---: | :---: | :---: |
| NOP | ne fais rien | 1 | 0x0000 | \- |
| HALT | arrête le CPU | 1 | 0x0001 | \- |
| WFI |  | 1 |  | \- |
| INT |  | 1 |  | \- |
| IRET |  | 1 |  | \- |
| IN X Y offset |  | 1 | 0x8XYO | \- |
| OUT X Y offset |  | 1 | 0x9XYO | \- |




   6. ## Code d'interruption


| Numéro |	Déclenchement	| Description |
| :---: | :---: | :---: |
| 00	| Division par 0.	| Traitement de l'erreur quand une division par 0 survient. |
| 01	| Avant chaque instruction quand TF=1 |	Trap : Exécution pas à pas. |
| 02	| INT 3	| Breakpoint : Point d'arrêt dans le code. |
| 03	| INTO	| Débordement numérique (Overflow). |
| 04  | - | - |
| 05	| IRQ0	| Timer |
| 06	| IRQ1	| Clavier	|
| 07	| IRQ2	| Disquette	|
| 08	| IRQ3 | Mauvaise instruction |
| 09	| - | - |
| 0A	| - | - |
| 0B	| - | - |
| 0C	| - | - |
| 0D	| - | - |
| 0E  | - | - |
| 0F	| - | - |
## 

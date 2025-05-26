#!/usr/bin/env python3
import struct

# -----------------------------------------------------------------------------
# build_rom.py : Génère rom.bin avec le code "bit-blit" pour afficher "Hello World!"
# et place la police bitmap après le code grâce à FONT_OFFSET.
# -----------------------------------------------------------------------------

# Configuration du texte et de la position de départ
TEXT       = "Hello World!"
START_X    = 10  # Position X initiale en pixels
START_Y    = 20  # Position Y initiale en pixels

# Offset où la police est stockée dans la ROM (en octets)
FONT_OFFSET = 0x0200  # 512 décimal: espace pour le code avant la police

# --- 1) Construction dynamique des instructions machine -----------------
# Définition des opcodes MOVI et OUT (dans votre ISA)
OP_MOVI = 0x6000  # base MOVI opcode, + registre index dans les 4 bits supérieurs
OP_OUT  = 0xA000  # base OUT opcode, + paramètres (voir table)

code_words = []
for i, ch in enumerate(TEXT):
    c = ord(ch)
    src = FONT_OFFSET + c * 8
    x   = START_X + i * 8
    y   = START_Y
    # MOVI R1, src  => opcode 0x6100 + immediate
    code_words += [0x6100, src]
    # MOVI R2, x
    code_words += [0x6200, x]
    # MOVI R3, y
    code_words += [0x6300, y]
    # MOVI R4, 8
    code_words += [0x6400, 8]
    # MOVI R5, 8
    code_words += [0x6500, 8]
    # MOVI R6, 3 (START|BITMODE)
    code_words += [0x6600, 3]
    # OUT R1,8,0
    code_words.append(0xA180)
    # OUT R2,8,1
    code_words.append(0xA281)
    # OUT R3,8,2
    code_words.append(0xA382)
    # OUT R4,8,3
    code_words.append(0xA483)
    # OUT R5,8,4
    code_words.append(0xA584)
    # OUT R6,8,5
    code_words.append(0xA685)
# HALT
code_words.append(0x0001)

# --- 2) Extraction de la police BIOS_FONT.h ---------------------------------
font_bytes = bytearray()
with open("BIOS_FONT.h", "r") as f:
    for line in f:
        line = line.strip()
        if line.startswith("{") and "//" in line:
            hex_vals = line.split("}")[0].strip("{} ")
            for byte_str in hex_vals.split(","):
                byte_str = byte_str.strip()
                if byte_str.startswith("0x"):
                    font_bytes.append(int(byte_str, 16))
assert len(font_bytes) == 256 * 8, f"Police incorrectement lue: trouvé {len(font_bytes)} octets"

# --- 3) Création de rom.bin (64 Ko) ------------------------------------------
ROM_SIZE = 0x8000  # 64 Ko

def write_words_at_offset(words, rom_file, offset):
    """Écrit les mots <words> (16-bit) à partir de l'offset <offset>."""
    rom_file.seek(offset)
    for w in words:
        rom_file.write(struct.pack("<H", w))

with open("rom.bin", "wb+") as rom:
    # a) Créer un espace vierge jusqu'à FONT_OFFSET
    rom.write(b"\x00" * FONT_OFFSET)
    # b) Insérer le code avant FONT_OFFSET (à 0x0000)
    #    On écrira par dessus le début des zéros
    write_words_at_offset(code_words, rom, 0)
    # c) Insérer la police à FONT_OFFSET
    rom.seek(FONT_OFFSET)
    rom.write(font_bytes)
    # d) Pad final jusqu’à ROM_SIZE
    final_pad = ROM_SIZE - rom.tell()
    if final_pad < 0:
        raise RuntimeError("rom.bin dépasse la taille allouée !")
    rom.write(b"\x00" * final_pad)

print(f"rom.bin généré ({ROM_SIZE} octets) avec police à 0x{FONT_OFFSET:04X} et texte '{TEXT}'")

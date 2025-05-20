import struct
import binascii
from abc import ABC, abstractmethod
from tokens import IMM, REGISTER


class INSTR(ABC):
	""" Instruction base type """
	name = "Instr"
	x = None
	y = None

	def __init__(self, x: IMM | REGISTER, y: IMM | REGISTER) -> None:
		self.x = x
		self.y = y

	@abstractmethod
	def compile(self) -> bytes:
		pass

	def __repr__(self) -> str:
		return self.name \
			+ (f" ({self.x})" if self.x is not None else "") \
			+ (f", ({self.y})" if self.y is not None else "") \
			+ f" -> [{binascii.hexlify(self.compile())}]"


class ARITH(INSTR):
	defs = {
		# Format: name, argc, code
		"add": ("Add", 2, 0),
		"sub": ("Sub", 2, 1),
		"mul": ("Mul", 2, 2),
		"div": ("Div", 2, 3),
		"sdiv": ("SDiv", 2, 4),
		"inc": ("Inc", 1, 5),
		"dec": ("Dec", 1, 6),
		"cmp": ("Cmp", 2, 7),
		"lsr": ("Lsr", 2, 8),
		"asr": ("Asr", 2, 9),
		"lsl": ("Lsl", 2, 10)
	}

	op = ""

	def __init__(self, op: str, x: IMM | REGISTER, y: IMM | REGISTER) -> None:
		super().__init__(x, y)
		self.op = op
		self.name = self.defs[op][0]

	def compile(self) -> bytes:
		return bytes([
			(1 << 4) + self.x.i, 
			(self.y.i << 4) + self.defs[self.op][2]
		])

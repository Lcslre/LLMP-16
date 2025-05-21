import struct
import binascii
from abc import ABC, abstractmethod

from tokens import *


class INSTR(ABC):
	""" Instruction base type """
	name = "Instr"
	x = None
	y = None

	# Format: name, argc, code
	defs: dict[str, tuple[str, int, int]] = {}
	op = ""

	def __init__(self, op: str, 
				 x: IMM | REGISTER | None, 
				 y: IMM | REGISTER | None) -> None:
		self.x = x
		self.y = y
		self.op = op
		self.name = self.defs[op][0]

	@abstractmethod
	def compile(self) -> bytes:
		pass

	def __repr__(self) -> str:
		return self.name \
			+ (f" ({self.x})" if self.x is not None else "") \
			+ (f", ({self.y})" if self.y is not None else "") \
			+ f" -> [{binascii.hexlify(self.compile(), "-", 2)}]"


class ARITH(INSTR):
	defs = {
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

	def __init__(self, op: str, x: REGISTER, y: IMM | REGISTER | None) -> None:
		super().__init__(op, x, y)


class ARITH_R(ARITH):
	def __init__(self, op: str, x: REGISTER, y: REGISTER | None) -> None:
		super().__init__(op, x, y)

	def compile(self) -> bytes:
		op2 = self.y.i << 4 if self.y is not None else 0

		return bytes([
			(1 << 4) + self.x.i, 
			op2 + self.defs[self.op][2]
		])


class ARITH_I(ARITH):
	def __init__(self, op: str, x: REGISTER, y: IMM) -> None:
		super().__init__(op, x, y)

	def compile(self) -> bytes:
		return bytes([
			(2 << 4) + self.x.i,
			self.defs[self.op][2],
			self.y.i >> 4,
			self.y.i & 0xFF
		])


class LOGIC(INSTR):
	defs = {
		"and": ("And", 2, 0),
		"or": ("Or", 2, 1),
		"xor": ("Xor", 2, 2),
		"not": ("Not", 1, 3),
		"tst": ("Test", 2, 4)
	}

	def __init__(self, op: str, x: REGISTER, y: IMM | REGISTER | None) -> None:
		super().__init__(op, x, y)


class LOGIC_R(LOGIC):
	def __init__(self, op: str, x: REGISTER, y: REGISTER | None) -> None:
		super().__init__(op, x, y)

	def compile(self) -> bytes:
		op2 = self.y.i << 4 if self.y is not None else 0

		return bytes([
			(3 << 4) + self.x.i, 
			op2 + self.defs[self.op][2]
		])


class LOGIC_I(LOGIC):
	def __init__(self, op: str, x: REGISTER, y: IMM) -> None:
		super().__init__(op, x, y)

	def compile(self) -> bytes:
		return bytes([
			(4 << 4) + self.x.i,
			self.defs[self.op][2],
			self.y.i >> 4,
			self.y.i & 0xFF
		])

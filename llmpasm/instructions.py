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
				 x: IMM | ADDRESS | REGISTER | None, 
				 y: IMM | ADDRESS | REGISTER | None):
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

	def __init__(self, op: str, x: REGISTER, y: IMM | REGISTER | None):
		super().__init__(op, x, y)


class ARITH_R(ARITH):
	def __init__(self, op: str, x: REGISTER, y: REGISTER | None):
		super().__init__(op, x, y)

	def compile(self) -> bytes:
		op2 = self.y.i << 4 if self.y is not None else 0

		return bytes([
			(1 << 4) + self.x.i, 
			op2 + self.defs[self.op][2]
		])


class ARITH_I(ARITH):
	def __init__(self, op: str, x: REGISTER, y: IMM):
		super().__init__(op, x, y)

	def compile(self) -> bytes:
		return bytes([
			(2 << 4) + self.x.i,
			self.defs[self.op][2],
			self.y.i >> 8,
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

	def __init__(self, op: str, x: REGISTER, y: IMM | REGISTER | None):
		super().__init__(op, x, y)


class LOGIC_R(LOGIC):
	def __init__(self, op: str, x: REGISTER, y: REGISTER | None):
		super().__init__(op, x, y)

	def compile(self) -> bytes:
		op2 = self.y.i << 4 if self.y is not None else 0

		return bytes([
			(3 << 4) + self.x.i, 
			op2 + self.defs[self.op][2]
		])


class LOGIC_I(LOGIC):
	def __init__(self, op: str, x: REGISTER, y: IMM):
		super().__init__(op, x, y)

	def compile(self) -> bytes:
		return bytes([
			(4 << 4) + self.x.i,
			self.defs[self.op][2],
			self.y.i >> 8,
			self.y.i & 0xFF
		])


class MEMCONTROL(INSTR):
	defs = {
		"mov": ("Mov", 2, 0),
		"ld": ("Load", 2, 1),
		"str": ("Store", 2, 2),
		"push": ("Push", 1, 3),
		"pop": ("Pop", 1, 4),
		"vld": ("VLoad", 2, 5),
		"vstr": ("VStore", 2, 6)
	}

	def __init__(self, op: str, x: REGISTER | IMM, y: IMM | ADDRESS | REGISTER | None):
		super().__init__(op, x, y)


class MEMCONTROL_R(MEMCONTROL):
	def __init__(self, op: str, x: REGISTER, y: REGISTER | None):
		super().__init__(op, x, y)

	def compile(self) -> bytes:
		op2 = self.y.i << 4 if self.y is not None else 0
		return bytes([
			(5 << 4) + self.x.i,
			op2 + self.defs[self.op][2]
		])


class MEMCONTROL_I(MEMCONTROL):
	def __init__(self, op: str, x: REGISTER | IMM, y: IMM | None):
		super().__init__(op, x, y)

	def compile(self) -> bytes:
		op1 = self.x.i if self.y is not None else 0
		op2 = self.x.i if self.y is None else self.y.i

		return bytes([
			(6 << 4) + op1,
			self.defs[self.op][2],
			op2 >> 8,
			op2 & 0xFF
		])


class MEMCONTROL_A(MEMCONTROL):
	def __init__(self, op: str, x: REGISTER, y: ADDRESS):
		super().__init__(op, x, y)

	def compile(self) -> bytes:
		return bytes([
			(6 << 4) + self.x.i,
			((self.y.i & 0xF0000) >> 12) + self.defs[self.op][2],
			(self.y.i >> 8) & 0xFF,
			self.y.i & 0xFF
		])


class JUMP(INSTR):
	defs = {
		"jmp": ("Jump", 1, 0),
		"jeq": ("Jump EQ", 1, 1),
		"jne": ("Jump NE", 1, 2),
		"jcs": ("Jump CS", 1, 3),
		"jcc": ("Jump CC", 1, 4),
		"jvs": ("Jump VS", 1, 5),
		"jvc": ("Jump VC", 1, 6),
		"jgt": ("Jump GT", 1, 7),
		"jlt": ("Jump LT", 1, 8),
		"jge": ("Jump GE", 1, 9),
		"jle": ("Jump LE", 1, 10),
		"jhi": ("Jump HI", 1, 11),
		"jls": ("Jump LS", 1, 12),
		"call": ("Call", 1, 13),
		"ret": ("Return", 0, 13),
	}

	def __init__(self, op: str, x: REGISTER | ADDRESS | None):
		super().__init__(op, x, None)


class JUMP_R(JUMP):
	def __init__(self, op: str, x: REGISTER | None):
		super().__init__(op, x)

	def compile(self) -> bytes:
		op1 = self.x.i if self.x is not None else 0
		return bytes([
			(7 << 4) + op1,
			self.defs[self.op][2]
		])


class JUMP_A(JUMP):
	def __init__(self, op: str, x: ADDRESS):
		super().__init__(op, x)

	def compile(self) -> bytes:
		return bytes([
			8 << 4,
			((self.x.i & 0xF0000) >> 12) + self.defs[self.op][2],
			(self.x.i >> 8) & 0xFF,
			self.x.i & 0xFF
		])

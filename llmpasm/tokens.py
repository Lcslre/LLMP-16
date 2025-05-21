from errors import *


class TOKEN:
	""" Token base type """
	name = "Token"
	i = None
	line = 0

	__match_args__ = ("i")

	def __init__(self, line: int):
		self.line = line

	def __repr__(self) -> str:
		return self.name + (f" {self.i}" if self.i is not None else "")


class OPERATOR(TOKEN):
	name = "Operator"
	def __init__(self, line: int, op: str):
		super().__init__(line)
		self.i = op


class BANK(TOKEN):
	name = "Bank"


class COMMENT_OPEN(TOKEN):
	name = "/*"


class COMMENT_CLOSE(TOKEN):
	name = "*/"


class IMM(TOKEN):
	name = "Imm"
	
	def __init__(self, line: int, imm: int):
		super().__init__(line)
		if imm > 0xFFFF:
			raise TokenError(line, f"Immediate {imm} (0x{imm:x}) is larger than 16 bits")
		self.i = imm


class REGISTER(TOKEN):
	name = "Reg"

	def __init__(self, line: int, reg: int):
		super().__init__(line)
		self.i = reg


class LABEL(TOKEN):
	name = "Label"

	def __init__(self, line: int, label: str):
		super().__init__(line)
		self.i = label[:-1]

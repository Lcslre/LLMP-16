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


class ADDRESS(TOKEN):
	name = "Addr"

	def __init__(self, line: int, addr: int):
		super().__init__(line)
		if addr > 0xFFFFF:
			raise TokenError(line, f"Address {addr} (0x{addr:06x}) is larger than 20 bits")
		self.i = addr

	def __repr__(self) -> str:
		return self.name + f" 0x{self.i:05x}"


class LABELED_ADDRESS(ADDRESS):
	name = "LAddr"

	def __init__(self, line: int, label: str, ctx: dict[str, int]):
		super().__init__(line, 0)
		self.label = label
		self.ctx = ctx

	def __repr__(self) -> str:
		return super().__repr__() + f" [{self.label}]"

	@property
	def i(self):
		return self.ctx[self.label]

	@i.setter
	def i(self, value):
		pass


class REGISTER(TOKEN):
	name = "Reg"

	def __init__(self, line: int, reg: int):
		super().__init__(line)
		self.i = reg


class LABELDEF(TOKEN):
	name = "Labeldef"

	def __init__(self, line: int, label: str):
		super().__init__(line)
		self.i = label


class LABEL(TOKEN):
	name = "Label"

	def __init__(self, line: int, label: str):
		super().__init__(line)
		self.i = label


class BYTES(TOKEN):
	name = "Bytes"


class END(TOKEN):
	name = "End"

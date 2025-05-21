class TOKEN:
	""" Token base type """
	name = "Token"
	i = None

	__match_args__ = ("i")

	def __repr__(self) -> str:
		return self.name + (f" {self.i}" if self.i is not None else "")


class OPERATOR(TOKEN):
	name = "Operator"
	def __init__(self, op: str) -> None:
		self.i = op


class BANK(TOKEN):
	name = "Bank"


class COMMENT_OPEN(TOKEN):
	name = "/*"


class COMMENT_CLOSE(TOKEN):
	name = "*/"


class IMM(TOKEN):
	name = "Imm"
	
	def __init__(self, imm: int) -> None:
		self.i = imm


class REGISTER(TOKEN):
	name = "Reg"

	def __init__(self, reg: int) -> None:
		self.i = reg


class LABEL(TOKEN):
	name = "Label"

	def __init__(self, label: str) -> None:
		self.i = label[:-1]

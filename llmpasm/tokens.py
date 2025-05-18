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


class DATA(TOKEN): 
	name = "Data"


class IMM(TOKEN):
	name = "Imm"
	
	def __init__(self, imm: str) -> None:
		self.i = int(imm)


class REGISTER(TOKEN):
	name = "Reg"

	def __init__(self, reg: str) -> None:
		self.i = int(reg[1:])


class LABEL(TOKEN):
	name = "Label"

	def __init__(self, label: str) -> None:
		self.i = label[:-1]

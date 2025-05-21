from tokens import IMM
from instructions import INSTR
from errors import SectionError


class SECTION:
	name = "Section"
	i = 0

	def __init__(self, n: IMM) -> None:
		self.instructions = []
		
		match n:
			case IMM():
				self.n = n.i
			case _:
				raise SectionError("Bank index is not an integer")

	def push(self, instr: INSTR) -> None:
		self.instructions.append(instr)

	def __repr__(self) -> str:
		representation = f"Section {self.name} {self.n}:\n"
		for instr in self.instructions:
			representation += "\t" + str(instr) + "\n"
		return representation


class SBANK(SECTION):
	name = "Bank"


class SDATA(SECTION):
	name = "Data"

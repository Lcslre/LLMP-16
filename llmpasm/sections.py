from tokens import IMM
from instructions import INSTR


class SECTION:
	name = "Section"
	i = 0

	def __init__(self, n: IMM) -> None:
		self.instructions = []
		self.n = n.i

	def push(self, instr: INSTR) -> None:
		self.instructions.append(instr)

	def __repr__(self) -> str:
		representation = f"Section {self.name} {self.n}:\n"
		for instr in self.instructions:
			representation += "\t" + str(instr) + "\n"
		return representation


class SBANK(SECTION):
	name = "Bank"

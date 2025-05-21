#!python3

import sys
import re
from typing import Type, TypeVar

from tokens import *
from instructions import *
from sections import *
from errors import *


class Lexer:
	number = re.compile(r"^-?\d+$")
	hexnumber = re.compile(r"^0x\d+$")

	register = re.compile(r"^r\d$")
	label = re.compile(r"^\w+:$")

	def __init__(self, code: str):
		self.symbols = []
		self.str = [s.casefold() for s in code.split()]

		self.lex()

	def lex(self) -> None:
		for s in self.str:
			token = s
			if re.match(self.number, s):
				token = IMM(int(s))
			elif re.match(self.hexnumber, s):
				token = IMM(int(s, 16))
			elif re.match(self.register, s):
				token = REGISTER(int(s[1:]))
			elif re.match(self.label, s):
				token = LABEL(s)
			elif s == ".bank":
				token = BANK()
			else:
				token = OPERATOR(token)

			self.symbols.append(token)

	T = TypeVar("T")
	def pop(self, t : Type[T]) -> T:
		res = self.symbols.pop(0) if len(self.symbols) else None

		if res is not None:
			if not isinstance(res, t):
				raise LexerError(f"Token '{res}' type is incorrect ({type(res)} is not {t})")
		else:
			raise LexerError("No more token available")

		return res


class Parser:
	def __init__(self, lexer: Lexer):
		self.lexer = lexer

	def parse(self) -> list[SECTION]:
		page = []
		context = None

		while True:
			try:
				token = self.lexer.pop(TOKEN)
			except LexerError:
				page.append(context)
				break

			if context is None:
				match token:
					case BANK():
						context = SBANK(self.lexer.pop(IMM))
					case _:
						raise ParsingError("Not in a bank or data section")
			else:
				match token:
					case BANK():
						page.append(context)
						context = SBANK(self.lexer.pop(IMM))
					case OPERATOR(i=s):
						match s:
							case s if s in ARITH.defs:
								operation = ARITH_R
								reg = self.lexer.pop(REGISTER)
								op2 = None
								if ARITH.defs[s][1] > 1:
									op2 = self.lexer.pop(IMM | REGISTER)
									match op2:
										case IMM():
											operation = ARITH_I
										case REGISTER():
											operation = ARITH_R
								context.push(operation(s, reg, op2))
							case s if s in LOGIC.defs:
								operation = LOGIC_R
								reg = self.lexer.pop(REGISTER)
								op2 = None
								if LOGIC.defs[s][1] > 1:
									op2 = self.lexer.pop(IMM | REGISTER)
									match op2:
										case IMM():
											operation = LOGIC_I
										case REGISTER():
											operation = LOGIC_R
								context.push(operation(s, reg, op2))

		return page


if __name__ == "__main__":
	#sys.tracebacklimit = 0

	with open(sys.argv[1]) as file:
		lexer = Lexer(file.read())
		for s in Parser(lexer).parse():
			print(s)

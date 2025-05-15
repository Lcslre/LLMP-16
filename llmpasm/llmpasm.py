#!python3

import sys
import re

from tokens import *
from instructions import *
from sections import *


class Lexer:
	number = re.compile("-?\\d")
	register = re.compile("r\\d")
	label = re.compile("\\w+:")

	symbols = []

	def __init__(self, code: str) -> None:
		self.str = [s.casefold() for s in code.split()]

		self.lex()
		print(self.symbols)

	def lex(self) -> None:
		for s in self.str:
			token = s
			if re.match(self.number, s):
				token = IMM(s)
			elif re.match(self.register, s):
				token = REGISTER(s)
			elif re.match(self.label, s):
				token = LABEL(s)
			elif s == ".bank":
				token = BANK()
			elif s == ".data":
				token = DATA()

			self.symbols.append(token)

	def pop(self) -> TOKEN | None:
		return self.symbols.pop(0) if len(self.symbols) else None


class ParsingError(Exception):
	pass


class Parser:
	def __init__(self, lexer: Lexer):
		self.lexer = lexer

	def parse(self) -> list[SECTION]:
		page = []
		context = None

		while True:
			token = self.lexer.pop()
			if token is None:
				page.append(context)
				break

			if context is None:
				match token:
					case BANK():
						context = SBANK(self.lexer.pop())
					case DATA():
						context = SDATA(self.lexer.pop())
					case _:
						raise ParsingError("Not in a bank or data section")
			else:
				match token:
					case BANK():
						page.append(context)
						context = SBANK(self.lexer.pop())
					case SDATA():
						page.append(context)
						context = SDATA(self.lexer.pop())
					case token if token in ARITH.defs:
						context.push(ARITH(token, 
							self.lexer.pop(), 
							self.lexer.pop() if ARITH.defs[token][1] > 1 else None))

		return page


if __name__ == "__main__":
	with open(sys.argv[1]) as file:
		lexer = Lexer(file.read())
		for s in Parser(lexer).parse():
			print(s)

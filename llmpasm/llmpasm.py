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

	current_line = 1

	def __init__(self, code: str):
		self.symbols = []
		lines = [s.casefold() for s in code.split("\n")]
		self.str = [line.split() for line in lines]

		self.lex()

	def lex(self) -> None:
		for idx, line in enumerate(self.str):
			for s in line:
				token = s
				if re.match(self.number, s):
					token = IMM(idx+1, int(s))
				elif re.match(self.hexnumber, s):
					token = IMM(idx+1, int(s, 16))
				elif re.match(self.register, s):
					token = REGISTER(idx+1, int(s[1:]))
				elif re.match(self.label, s):
					token = LABEL(idx+1, s)
				elif s == ".bank":
					token = BANK(idx+1)
				elif s == "/*":
					token = COMMENT_OPEN(idx+1)
				elif s == "*/":
					token = COMMENT_CLOSE(idx+1)
				else:
					token = OPERATOR(idx+1, token)

				self.symbols.append(token)

	T = TypeVar("T")
	def pop(self, t : Type[T]) -> T:
		res = self.symbols.pop(0) if len(self.symbols) else None

		if res is not None:
			self.current_line = res.line
			if not isinstance(res, t):
				raise LexerError(res.line, f"Token '{res}' type is incorrect ({type(res)} is not {t})")
		else:
			raise LexerError(self.current_line, "No more token available")

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

			if isinstance(token, COMMENT_OPEN):
				while not isinstance(self.lexer.pop(TOKEN), COMMENT_CLOSE):
					continue
				continue

			if context is None:
				match token:
					case BANK():
						context = SBANK(self.lexer.pop(IMM))
					case _:
						raise ParsingError(token.line, "Not in a bank or data section")
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
							case _:
								raise ParsingError(token.line, f"Unknown operator '{s}'")
					case _:
						raise ParsingError(token.line, f"Wrong token '{token}'")

		return page


if __name__ == "__main__":
	#sys.tracebacklimit = 0

	with open(sys.argv[1]) as file:
		lexer = Lexer(file.read())
		for s in Parser(lexer).parse():
			print(s)

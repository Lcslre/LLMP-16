#!python3

import sys
import re
import os
import types
from typing import Type, TypeVar, get_args

from tokens import *
from instructions import *
from errors import *

from dataclasses import dataclass
import argparse


class Lexer:
	number = re.compile(r"^-?\d+$")
	hexnumber = re.compile(r"^0x[\w\d]+$")

	address = re.compile(r"^\[\d+\]$")
	hexaddress = re.compile(r"^\[0x[\w\d]+\]$")

	character = re.compile(r"^'\w'$")

	register = re.compile(r"^r\d+$")
	labeldef = re.compile(r"^\w+:$")
	label = re.compile(r"^\[\w+\]$")

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
				elif re.match(self.character, s):
					token = IMM(idx+1, ord(s[1]))
				elif re.match(self.address, s):
					token = ADDRESS(idx+1, int(s[1:-1]))
				elif re.match(self.hexaddress, s):
					token = ADDRESS(idx+1, int(s[1:-1], 16))
				elif re.match(self.register, s):
					token = REGISTER(idx+1, int(s[1:]))
				elif re.match(self.labeldef, s):
					token = LABELDEF(idx+1, s[:-1])
				elif re.match(self.label, s):
					token = LABEL(idx+1, s[1:-1])
				elif s == "/*":
					token = COMMENT_OPEN(idx+1)
				elif s == "*/":
					token = COMMENT_CLOSE(idx+1)
				elif s == ".bytes":
					token = BYTES(idx+1)
				elif s == "(end)":
					token = END(idx+1)
				else:
					token = OPERATOR(idx+1, token)

				self.symbols.append(token)

	T = TypeVar("T")
	def pop(self, t : Type[T]) -> T:
		res = self.symbols.pop(0) if len(self.symbols) else None

		if res is not None:
			self.current_line = res.line
			if not isinstance(res, t):
				name2 = t.__name__ if not isinstance(t, types.UnionType) else " or ".join([c.__name__ for c in get_args(t)])
				raise LexerError(res.line, f"Token '{res}' type is incorrect ({type(res).__name__} is not {name2})")
		else:
			raise LexerError(self.current_line, "No more token available")

		return res


class Parser:
	def __init__(self, lexer: Lexer):
		self.lexer = lexer

	def parse(self) -> list[INSTR]:
		page = []
		pc = 0
		context = {}

		while True:
			try:
				token = self.lexer.pop(TOKEN)
			except LexerError:
				break

			if isinstance(token, COMMENT_OPEN):
				while not isinstance(self.lexer.pop(TOKEN), COMMENT_CLOSE):
					continue
				continue

			match token:
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
										pc += 2
									case REGISTER():
										operation = ARITH_R
										pc += 1
							page.append(operation(s, reg, op2))
						case s if s in LOGIC.defs:
							operation = LOGIC_R
							reg = self.lexer.pop(REGISTER)
							op2 = None
							if LOGIC.defs[s][1] > 1:
								op2 = self.lexer.pop(IMM | REGISTER)
								match op2:
									case IMM():
										operation = LOGIC_I
										pc += 2
									case REGISTER():
										operation = LOGIC_R
										pc += 1
							page.append(operation(s, reg, op2))
						case s if s in MEMCONTROL.defs:
							operation = MEMCONTROL_R
							op1 = self.lexer.pop(REGISTER | IMM)
							op2 = None
							match op1:
								case IMM():
									operation = MEMCONTROL_I
									pc += 2
								case REGISTER():
									op2 = None
									if MEMCONTROL.defs[s][1] > 1:
										op2 = self.lexer.pop(IMM | ADDRESS | REGISTER | LABEL)
										match op2:
											case IMM():
												operation = MEMCONTROL_I
												pc += 2
											case ADDRESS():
												operation = MEMCONTROL_A
												pc += 2
											case LABEL(i=label):
												operation = MEMCONTROL_A
												op2 = LABELED_ADDRESS(op2.line, label, context)
												pc += 2
											case REGISTER():
												operation = MEMCONTROL_R
												pc += 1
							page.append(operation(s, op1, op2))
						case s if s in JUMP.defs:
							operation = JUMP_R
							op1 = None
							if s != "ret":
								op1 = self.lexer.pop(REGISTER | ADDRESS | LABEL)
								match op1:
									case REGISTER():
										operation = JUMP_R
										pc += 1
									case ADDRESS():
										operation = JUMP_A
										pc += 2
									case LABEL(i=label):
										operation = JUMP_A
										op1 = LABELED_ADDRESS(op1.line, label, context)
										pc += 2
							page.append(operation(s, op1))
						case s if s in SPECIAL.defs:
							page.append(SPECIAL(s))
						case s if s in INOUT.defs:
							page.append(
								INOUT(s, 
									self.lexer.pop(REGISTER),
									self.lexer.pop(REGISTER),
									self.lexer.pop(IMM)))
						case _:
							raise ParsingError(token.line, f"Unknown operator '{s}'")
				case LABELDEF(i=label):
					context[label] = pc
				case BYTES():
					pbytearray = []
					while not isinstance((b := self.lexer.pop(IMM | END)), END):
						pbytearray.append(b)
					page.append(BYTEARRAY(pbytearray))
				case _:
					raise ParsingError(token.line, f"Wrong token '{token}'")

		return page


if __name__ == "__main__":
	parser = argparse.ArgumentParser(
		prog="LLMP16asm",
		description="Assembly compiler for the LLMP16 microprocessor")

	parser.add_argument("filename", help="Input filename")
	parser.add_argument("-o", "--output", type=str, default=f"{os.getcwd()}/a.out", help="Output filename")
	parser.add_argument("-d", "--debug", action="store_true", help="Print the error stacktrace")
	parser.add_argument("-v", "--verbose", action="store_true", help="Print the parsed instructions")

	args = parser.parse_args()

	if not args.debug:
		sys.tracebacklimit = 0

	with open(args.filename) as inputfile:
		lexer = Lexer(inputfile.read())
		parsed = Parser(lexer).parse()

		if args.verbose:
			print(f"Compiled content from {args.filename}:")
			for s in parsed:
				print(f"\t{s}")

		with open(args.output, "bw") as outputfile:
			for s in parsed:
				outputfile.write(s.compile())

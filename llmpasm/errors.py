class LlmpAsmError(Exception):
	def __init__(self, line: int, msg: str):
		super().__init__(f"At line {line}: {msg}")


class LexerError(LlmpAsmError):
	pass


class ParsingError(LlmpAsmError):
	pass


class TokenError(LlmpAsmError):
	pass

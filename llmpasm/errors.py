class LlmpAsmError(Exception):
	# TODO: line number
	pass


class LexerError(LlmpAsmError):
	pass


class SectionError(LlmpAsmError):
	pass


class ParsingError(LlmpAsmError):
	pass

#pragma once

#define TOKENS \
	/* Default (zero) token */ \
	X(unknown, "<Unknown>") \
	/* Delimiters */ \
	X(paren_open, "(") \
	X(paren_close, ")") \
	X(square_open, "[") \
	X(square_close, "]") \
	X(curly_open, "{") \
	X(curly_close, "}") \
	/* Punctuation */ \
	X(comma, ",") \
	X(semicolon, ";") \
	X(colon, ":") \
	/* Operators */ \
	X(dot, ".") \
	X(plus, "+") \
	X(minus, "-") \
	X(star, "*") \
	X(slash, "/") \
	X(modulo, "%") \
	X(and, "&") \
	X(or, "|") \
	X(tilde, "~") \
	X(logic_or, "and") \
	X(logic_and, "or") \
	X(logic_not, "not") \
	/* Keywords */ \
	X(var, "var") \
	X(const, "const") \
	X(if, "if") \
	X(else, "else") \
	X(for, "for") \
	X(break, "break") \
	X(continue, "continue") \
	X(func, "func") \
	X(return, "return") \
	X(struct, "struct") \
	/* Literals */ \
	X(identifier, "<Identifier>") \
	X(string, "<String>") \
	X(integer, "<Integer>") \
	X(real, "<Real>") \
	X(rune, "<Rune>") \
	X(true, "true") \
	X(false, "false") \
	X(null, "null") \
	/* Specials */ \
	X(comment, "<Comment>") \
	X(end_of_file, "<End Of File>") \
	/* END OF TOKENS */


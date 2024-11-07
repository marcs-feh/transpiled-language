#pragma once
#include "prelude.h"

typedef u8 Compiler_Error_Type;
typedef struct Compiler_Error Compiler_Error;

enum Compiler_Error_Type {
	none = 0,
	lex_end_of_file,
	lex_unexpected_token,
	lex_unrecognized_token,
};

struct Compiler_Error {
	string message;
	i32 position;
	Compiler_Error_Type type;
};


#include "prelude.h"

typedef struct Lexer Lexer;
typedef struct Token Token;
typedef u8 Token_Kind;

#include "tokens.c"

enum Token_Kind {
	#define X(TokName, _) tk_##TokName,
	TOKENS
	#undef X
};

static const string token_names[] = {
	#define X(TokName, TokSymbol) [tk_##TokName] = str_lit(TokSymbol),
	TOKENS
	#undef X
};

struct Token {};

struct Lexer {
	string source;
	i32 current;
	i32 previous;

	Mem_Arena arena;
};

int main(){
	return 0;
}


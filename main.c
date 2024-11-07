#include "prelude.h"
#include "common.h"
#include <stdio.h>

Mem_Arena* get_temp_arena(){
	static byte TEMP_ARENA_MEM[MiB(4)];
	static Mem_Arena arena = {
		.cap = sizeof(TEMP_ARENA_MEM),
		.data = TEMP_ARENA_MEM,
		.last_allocation = null,
		.offset = 0,
	};

	return &arena;
}

static
string tprintf(char const * restrict fmt, ...){
	Mem_Arena* arena = get_temp_arena();
	const isize bufsize = 512;
	byte* buffer = arena_push_array(arena, byte, bufsize);
	int n = 0;

	va_list args;
	va_start(args, fmt);
	{
		n = vsnprintf((char*) buffer, bufsize, fmt, args);
	}
	va_end(args);

	string result = {.data = buffer, .len = n};
	return result;
}

typedef struct Lexer Lexer;
typedef struct Token Token;
typedef struct Token_List Token_List;
typedef i8 Token_Kind;

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

struct Token {
	string lexeme;
	Token_Kind kind;
	union {
		i64 integer;
		f64 real;
		rune codepoint;
		string text;
	};
};

struct Token_List {
	Token* data;
	u32 len;
	u32 cap;
	Mem_Arena* arena;
};

struct Lexer {
	string source;
	i32 current;
	i32 previous;
	Token_List tokens;
};

static
void tk_list_add(Token_List* list, Token token){
	if(list->len >= list->cap){
		isize new_cap = min(16, list->cap * 2) * sizeof(*list->data);
		void* new_data = arena_resize(list->arena, list->data, new_cap);
		if(new_data == null){
			panic("Could not add token to slice");
		}
		list->cap = new_cap;
		list->data = new_data;
	}

	list->data[list->len] = token;
	list->len += 1;
}

#define MAX_LEXER_ERRORS 64

Compiler_Error lex_next(Lexer* lex){
	panic("Unimplemented.");
}

Token_List tokenize(string source, Mem_Arena* arena){
	Lexer lex = {
		.source = source,
		.previous = 0,
		.current = 0,
		.tokens = {
			.arena = arena,
			.len = 0,
			.cap = 0,
			.data = null,
		},
	};

	Compiler_Error errors[MAX_LEXER_ERRORS] = {0};
	i32 error_count = 0;

	while(true){
		Compiler_Error err = lex_next(&lex);
		if(err.type == lex_end_of_file){
			break;
		}
		if(err.type != none){
			errors[min(MAX_LEXER_ERRORS - 1, error_count)] = err;
			error_count += 1;
		}
	}

	if(error_count > 0){
		panic("Lexer error");
	}

	return lex.tokens;
}

int main(){
	string msg = tprintf("[%d] Hello, %s\n", 69, "BIXA");
	puts((char*)msg.data);
	return 0;
}


/* A Prelude for C
 * This header contains very essential definitions. */
#pragma once

/* ---------------- Definitions ---------------- */
#include <stddef.h>
#include <stdint.h>
#include <stdalign.h>
#include <stdatomic.h>
#include <stdnoreturn.h>
#include <stdbool.h>
#include <stdarg.h>
#include <limits.h>

#define func static inline

#define null NULL

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef unsigned int uint;
typedef uint8_t byte;

typedef ptrdiff_t isize;
typedef size_t    usize;

typedef uintptr_t uintptr;

typedef float f32;
typedef double f64;

typedef char const * cstring;

typedef i32 rune;
typedef struct string string;

func
void swap_bytes_raw(byte* data, isize len){
	for(isize i = 0; i < (len / 2); i += 1){
		byte temp = data[i];
		data[i] = data[len - (i + 1)];
		data[len - (i + 1)] = temp;
	}
}

#define swap_bytes(Ptr) swap_bytes_raw((byte*)(Ptr), sizeof(*(Ptr)))

#define static_assert(Expr, Msg) _Static_assert(Expr, Msg)

static_assert(sizeof(f32) == 4 && sizeof(f64) == 8, "Bad float size");
static_assert(sizeof(isize) == sizeof(usize), "Mismatched (i/u)size");
static_assert(CHAR_BIT == 8, "Invalid char size");

#define min(A, B) ((A) < (B) ? (A) : (B))
#define max(A, B) ((A) > (B) ? (A) : (B))
#define clamp(Lo, X, Hi) min(max(Lo, X), Hi)

#define container_of(Ptr, Type, Member) \
	((Type *)(((void *)(Ptr)) - offsetof(Type, Member)))

#ifndef __cplusplus
#undef bool
typedef _Bool bool;
#endif

/* ---------------- Assert & Panic ---------------- */
extern int snprintf (char* s, size_t len, char const* fmt, ...);
extern int puts(char const* s);
extern noreturn void abort();

#define MAX_PANIC_MSG_LEN 2048

func
noreturn void panic_ex(cstring msg, cstring file, i32 line){
	char buf[MAX_PANIC_MSG_LEN];
	int n = snprintf(buf, MAX_PANIC_MSG_LEN - 1, "%s:%d Panic: %s", file, line, msg);
	buf[n] = 0;
	puts(buf);
	abort();
}

func
void assert_ex(bool predicate, cstring msg, cstring file, i32 line){
	if(!predicate){
		char buf[MAX_PANIC_MSG_LEN];
		int n = snprintf(buf, MAX_PANIC_MSG_LEN - 1, "%s:%d Assertion failed: %s\n", file, line, msg);
		buf[n] = 0;
		puts(buf);
		abort();
	}
}

#ifdef assert
#undef assert
#endif

#define assert(Pred, Msg) assert_ex((Pred), (Msg), __FILE__, __LINE__)

#define panic(Msg) panic_ex((Msg), __FILE__, __LINE__)

#undef MAX_PANIC_MSG_LEN

/* ---------------- UTF-8 Support ---------------- */
#define UTF8_RANGE1 ((i32)0x7f)
#define UTF8_RANGE2 ((i32)0x7ff)
#define UTF8_RANGE3 ((i32)0xffff)
#define UTF8_RANGE4 ((i32)0x10ffff)

typedef struct Utf8_Encode_Result Utf8_Encode_Result;
typedef struct Utf8_Decode_Result Utf8_Decode_Result;
typedef struct Utf8_Iterator Utf8_Iterator;

struct Utf8_Encode_Result {
	byte bytes[4];
	i8 len;
};

struct Utf8_Decode_Result {
	rune codepoint;
	i8 len;
};

static const rune UTF8_ERROR = 0xfffd;

static const Utf8_Encode_Result UTF8_ERROR_ENCODED = {
	.bytes = {0xef, 0xbf, 0xbd},
	.len = 3,
};

struct Utf8_Iterator {
	byte const* data;
	isize data_length;
	isize current;
};

#define str_lit(cstrLit) ((string const){.data = (byte const*)(cstrLit), .len = sizeof(cstrLit)})

// Meant to be used with `%.*s`
#define str_fmt(str) ((int)str.len), str.data

#define SURROGATE1 ((i32)0xd800)
#define SURROGATE2 ((i32)0xdfff)

#define MASK2 (0x1f) /* 0001_1111 */
#define MASK3 (0x0f) /* 0000_1111 */
#define MASK4 (0x07) /* 0000_0111 */

#define MASKX (0x3f) /* 0011_1111 */

#define SIZE2 (0xc0) /* 110x_xxxx */
#define SIZE3 (0xe0) /* 1110_xxxx */
#define SIZE4 (0xf0) /* 1111_0xxx */

#define CONT  (0x80) /* 10xx_xxxx */

#define CONTINUATION1 (0x80)
#define CONTINUATION2 (0xbf)

func
bool is_continuation_byte(rune c){
	return (c >= CONTINUATION1) && (c <= CONTINUATION2);
}

func
Utf8_Encode_Result utf8_encode(rune c){
	Utf8_Encode_Result res = {0};

	if((c >= CONTINUATION1 && c <= CONTINUATION2)
		|| (c >= SURROGATE1 && c <= SURROGATE2)
		|| (c > UTF8_RANGE4))
	{
		return UTF8_ERROR_ENCODED;
	}

	if(c <= UTF8_RANGE1){
		res.len = 1;
		res.bytes[0] = c;
	}
	else if(c <= UTF8_RANGE2){
		res.len = 2;
		res.bytes[0] = SIZE2 | ((c >> 6) & MASK2);
		res.bytes[1] = CONT  | ((c >> 0) & MASKX);
	}
	else if(c <= UTF8_RANGE3){
		res.len = 3;
		res.bytes[0] = SIZE3 | ((c >> 12) & MASK3);
		res.bytes[1] = CONT  | ((c >> 6) & MASKX);
		res.bytes[2] = CONT  | ((c >> 0) & MASKX);
	}
	else if(c <= UTF8_RANGE4){
		res.len = 4;
		res.bytes[0] = SIZE4 | ((c >> 18) & MASK4);
		res.bytes[1] = CONT  | ((c >> 12) & MASKX);
		res.bytes[2] = CONT  | ((c >> 6)  & MASKX);
		res.bytes[3] = CONT  | ((c >> 0)  & MASKX);
	}
	return res;
}

#define DECODE_ERROR ((Utf8_Decode_Result){ .codepoint = UTF8_ERROR, .len = 0 })

func
Utf8_Decode_Result utf8_decode(byte const* buf, isize len){
	Utf8_Decode_Result res = {0};
	if(buf == NULL || len <= 0){ return DECODE_ERROR; }

	u8 first = buf[0];

	if((first & CONT) == 0){
		res.len = 1;
		res.codepoint |= first;
	}
	else if ((first & ~MASK2) == SIZE2 && len >= 2){
		res.len = 2;
		res.codepoint |= (buf[0] & MASK2) << 6;
		res.codepoint |= (buf[1] & MASKX) << 0;
	}
	else if ((first & ~MASK3) == SIZE3 && len >= 3){
		res.len = 3;
		res.codepoint |= (buf[0] & MASK3) << 12;
		res.codepoint |= (buf[1] & MASKX) << 6;
		res.codepoint |= (buf[2] & MASKX) << 0;
	}
	else if ((first & ~MASK4) == SIZE4 && len >= 4){
		res.len = 4;
		res.codepoint |= (buf[0] & MASK4) << 18;
		res.codepoint |= (buf[1] & MASKX) << 12;
		res.codepoint |= (buf[2] & MASKX) << 6;
		res.codepoint |= (buf[3] & MASKX) << 0;
	}
	else {
		return DECODE_ERROR;
	}

	// Validation
	if(res.codepoint >= SURROGATE1 && res.codepoint <= SURROGATE2){
		return DECODE_ERROR;
	}
	if(res.len > 1 && (buf[1] < CONTINUATION1 || buf[1] > CONTINUATION2)){
		return DECODE_ERROR;
	}
	if(res.len > 2 && (buf[2] < CONTINUATION1 || buf[2] > CONTINUATION2)){
		return DECODE_ERROR;
	}
	if(res.len > 3 && (buf[3] < CONTINUATION1 || buf[3] > CONTINUATION2)){
		return DECODE_ERROR;
	}

	return res;
}

func
bool utf8_iter_next(Utf8_Iterator* iter, rune* r, i8* len){
	if(iter->current >= iter->data_length){ return 0; }

	Utf8_Decode_Result res = utf8_decode(&iter->data[iter->current], iter->data_length);
	*r = res.codepoint;
	*len = res.len;

	if(res.codepoint == DECODE_ERROR.codepoint){
		*len = res.len + 1;
	}

	iter->current += res.len;

	return 1;
}

func
bool utf8_iter_prev(Utf8_Iterator* iter, rune* r, i8* len){
	if(iter->current <= 0){ return false; }

	iter->current -= 1;
	while(is_continuation_byte(iter->data[iter->current])){
		iter->current -= 1;
	}

	Utf8_Decode_Result res = utf8_decode(&iter->data[iter->current], iter->data_length - iter->current);
	*r = res.codepoint;
	*len = res.len;
	return true;
}

#undef SURROGATE2
#undef SURROGATE1
#undef MASK2
#undef MASK3
#undef MASK4
#undef MASKX
#undef SIZE2
#undef SIZE3
#undef SIZE4
#undef CONT
#undef CONTINUATION1
#undef CONTINUATION2
#undef DECODE_ERROR

/* ---------------- Strings ---------------- */
struct string {
	byte const * data;
	isize len;
};

func
isize cstring_len(cstring cstr){
	static const isize CSTR_MAX_LENGTH = (~(u32)0) >> 1;
	isize size = 0;
	for(isize i = 0; i < CSTR_MAX_LENGTH && cstr[i] != 0; i += 1){
		size += 1;
	}
	return size;
}

func
Utf8_Iterator str_iterator(string s){
	return (Utf8_Iterator){
		.current = 0,
		.data_length = s.len,
		.data = s.data,
	};
}

func
Utf8_Iterator str_iterator_reversed(string s){
	return (Utf8_Iterator){
		.current = s.len,
			.data_length = s.len,
			.data = s.data,
	};
}

func
bool str_empty(string s){
	return s.len == 0 || s.data == NULL;
}

func
string str_from(cstring data){
	string s = {
		.data = (byte const *)data,
		.len = cstring_len(data),
	};
	return s;
}

func
string str_from_bytes(byte const* data, isize length){
	string s = {
		.data = (byte const *)data,
		.len = length,
	};
	return s;
}

func
string str_from_range(cstring data, isize start, isize length){
	string s = {
		.data = (byte const *)&data[start],
		.len = length,
	};
	return s;
}

func
isize str_codepoint_count(string s){
	Utf8_Iterator it = str_iterator(s);

	isize count = 0;
	rune c; i8 len;
	while(utf8_iter_next(&it, &c, &len)){
		count += 1;
	}
	return count;
}

func
isize str_codepoint_offset(string s, isize n){
	Utf8_Iterator it = str_iterator(s);

	isize acc = 0;

	rune c; i8 len;
	do {
		if(acc == n){ break; }
		acc += 1;
	} while(utf8_iter_next(&it, &c, &len));

	return it.current;
}

func
string str_sub(string s, isize start, isize byte_count){
	static const string EMPTY = {0};
	if(start < 0 || byte_count < 0 || (start + byte_count) > s.len){ return EMPTY; }

	string sub = {
		.data = &s.data[start],
		.len = byte_count,
	};

	return sub;
}

func
bool str_eq(string a, string b){
	if(a.len != b.len){ return false; }

	for(isize i = 0; i < a.len; i += 1){
		if(a.data[i] != b.data[i]){ return false; }
	}

	return true;
}

#define MAX_CUTSET_LEN 64

func
string str_trim_leading(string s, string cutset){
	rune set[MAX_CUTSET_LEN] = {0};
	isize set_len = 0;
	isize cut_after = 0;

	/* Decode Cutset */ {
		rune c; i8 n;
		Utf8_Iterator iter = str_iterator(cutset);

		isize i = 0;
		while(utf8_iter_next(&iter, &c, &n) && i < MAX_CUTSET_LEN){
			set[i] = c;
			i += 1;
		}
		set_len = i;
	}

	/* Strip Cutset */ {
		rune c; i8 n;
		Utf8_Iterator iter = str_iterator(s);

		while(utf8_iter_next(&iter, &c, &n)){
			bool to_be_cut = false;
			for(isize i = 0; i < set_len; i += 1){
				if(set[i] == c){
					to_be_cut = true;
					break;
				}
			}

			if(to_be_cut){
				cut_after += n;
			}
			else {
				break; /* Reached first rune that isn't in cutset */
			}

		}
	}

	return str_sub(s, cut_after, s.len - cut_after);
}

func
string str_trim_trailing(string s, string cutset){
	rune set[MAX_CUTSET_LEN] = {0};
	isize set_len = 0;
	isize cut_until = s.len;

	/* Decode Cutset */ {
		rune c; i8 n;
		Utf8_Iterator iter = str_iterator(cutset);

		isize i = 0;
		while(utf8_iter_next(&iter, &c, &n) && i < MAX_CUTSET_LEN){
			set[i] = c;
			i += 1;
		}
		set_len = i;
	}

	/* Strip Cutset */ {
		rune c; i8 n;
		Utf8_Iterator iter = str_iterator_reversed(s);

		while(utf8_iter_prev(&iter, &c, &n)){
			bool to_be_cut = false;
			for(isize i = 0; i < set_len; i += 1){
				if(set[i] == c){
					to_be_cut = true;
					break;
				}
			}

			if(to_be_cut){
				cut_until -= n;
			}
			else {
				break; /* Reached first rune that isn't in cutset */
			}

		}
	}

	return str_sub(s, 0, cut_until);
}

func
string str_trim(string s, string cutset){
	string st = str_trim_leading(str_trim_trailing(s, cutset), cutset);
	return st;
}

#undef MAX_CUTSET_LEN


/* ---------------- Spinlock ---------------- */
#define SPINLOCK_LOCKED 1
#define SPINLOCK_UNLOCKED 0

typedef struct Spinlock Spinlock;

struct Spinlock {
	atomic_int _state;
};

func
void spinlock_acquire(Spinlock* l){
	for(;;){
		if(!atomic_exchange_explicit(&l->_state, SPINLOCK_LOCKED, memory_order_acquire)){
			break;
		}
		/* Busy wait while locked */
		while(atomic_load_explicit(&l->_state, memory_order_relaxed));
	}
}

func
bool spinlock_try_acquire(Spinlock* l){
    return !atomic_exchange_explicit(&l->_state, SPINLOCK_LOCKED, memory_order_acquire);
}

func
void spinlock_release(Spinlock* l){
	atomic_store(&l->_state, SPINLOCK_UNLOCKED);
}

#define spinlock_guard(Lock, Code) do {\
	spinlock_acquire(Lock); \
	do { Code ; } while(0); \
	spinlock_release(Lock); \
} while(0)

/* ---------------- Memory ---------------- */
#if defined(__clang__) || defined(__GNUC__)
#define _memset_impl __builtin_memset
#define _memcpy_impl __builtin_memcpy
#define _memmove_impl __builtin_memmove
#else
extern void* memset(void *dst, int c, size_t n);
extern void* memmove(void *dst, void const * src, size_t n);
extern void* memcpy(void *dst, void const * src, size_t n);

#define _memset_impl memset
#define _memcpy_impl memcpy
#define _memmove_impl memmove
#endif

func
void mem_copy(void* dest, void const* source, isize nbytes){
	assert(nbytes >= 0, "Cannot copy < 0 bytes");
	_memmove_impl(dest, source, nbytes);
}

func
void mem_copy_no_overlap(void* dest, void const* source, isize nbytes){
	assert(nbytes >= 0, "Cannot copy < 0 bytes");
	_memcpy_impl(dest, source, nbytes);
}

func
void mem_set(void* dest, byte val, isize nbytes){
	assert(nbytes >= 0, "Cannot copy < 0 bytes");
	_memset_impl(dest, val, nbytes);
}

#undef _memset_impl
#undef _memcpy_impl
#undef _memmove_impl

#define KiB(x) ((isize)(x) * 1024ll)
#define MiB(x) ((isize)(x) * 1024ll * 1024ll)
#define GiB(x) ((isize)(x) * 1024ll * 1024ll * 1024ll)

func
bool mem_valid_alignment(isize align){
	return (align & (align - 1)) == 0 && (align != 0);
}

func
uintptr mem_align_forward(uintptr p, uintptr a){
	assert(mem_valid_alignment(a), "Invalid memory alignment");
	uintptr mod = p & (a - 1);
	if(mod > 0){
		p += (a - mod);
	}
	return p;
}

/* ---------------- Arena ---------------- */
typedef struct Mem_Arena Mem_Arena;

struct Mem_Arena {
	byte* data;
	isize offset;
	isize cap;

	void* last_allocation;
};

func
Mem_Arena arena_make(byte* data, isize data_len){
	assert(data_len > 0, "Data length cannot be <= 0");
	Mem_Arena a = {
		.data = data,
		.offset = 0,
		.cap = data_len,
		.last_allocation = NULL,
	};
	return a;
}

func
uintptr arena_required_mem(uintptr cur, isize nbytes, isize align){
	assert(mem_valid_alignment(align), "Invalid memory alignment");
	uintptr aligned  = mem_align_forward(cur, align);
	uintptr padding  = (uintptr)(aligned - cur);
	uintptr required = padding + nbytes;
	return required;
}

func
void* arena_alloc_non_zero(Mem_Arena* a, isize size, isize align){
	uintptr base = (uintptr)a->data;
	uintptr current = (uintptr)base + (uintptr)a->offset;

	uintptr available = (uintptr)a->cap - (current - base);
	uintptr required = arena_required_mem(current, size, align);

	if(required > available){
		return NULL;
	}

	a->offset += required;
	void* allocation = &a->data[a->offset - size];
	a->last_allocation = allocation;
	return allocation;
}

func
void* arena_alloc(Mem_Arena* a, isize size, isize align){
	void* p = arena_alloc_non_zero(a, size, align);
	if(p){
		mem_set(p, 0, size);
	}
	return p;
}

#define arena_push(ArenaPtr, Type) \
	arena_alloc((ArenaPtr), sizeof(Type), alignof(Type))

#define arena_push_array(ArenaPtr, Type, Count) \
	arena_alloc((ArenaPtr), sizeof(Type) * (Count), alignof(Type))

func
void arena_free(Mem_Arena* a){
	a->offset = 0;
	a->last_allocation = NULL;
}

func
void* arena_resize(Mem_Arena* a, void* p, isize size){
	if(p != a->last_allocation || size < 0 || p == NULL){
		return NULL;
	}

	uintptr base = (uintptr)a->data;
	uintptr last_offset = (uintptr)p - base;

	isize old_size = a->offset - last_offset;
	isize delta = size - old_size;

	if((a->offset + delta) < a->cap){
		a->offset += delta;
	}
	else {
		return NULL;
	}

	return p;
}

#undef func


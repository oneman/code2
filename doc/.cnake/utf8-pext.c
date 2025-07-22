#include <stddef.h>
#include <stdint.h>

#define ASSERT(X) ((X) ? (void)0 : __builtin_unreachable())
#define MIN(A, B) ((A) < (B) ? (A) : (B))

typedef uint8_t      u8;
typedef uint32_t     u32;
typedef uint64_t     u64;
typedef ptrdiff_t    Size;

#if __has_include(<immintrin.h>) && __BMI2__
	#include <immintrin.h>
	static u32 pext32(u32 v, u32 mask) { return _pext_u32(v, mask); }
#else
	#warning "using slower portable C fallback for pext32()"
	static u32
	pext32(u32 v, u32 mask)
	{
		u32 ret = 0;
		u32 test = 0x1;
		while (mask != 0) {
			u32 ontarget = mask & test;
			ret |= v & ontarget;
			test <<= !!ontarget;
			v    >>= !ontarget;
			mask >>= !ontarget;
		}
		return ret;
	}
#endif

//////////
// caller should initialize `err` to 0
// str is expected to be at least 1 byte long
// the decoded code-point is returned in `.cp`
// `.len` has the length of the code-point in (utf8) bytes
// the content of `U8Result` is garbage if `*err` gets set
typedef struct {
	u32 cp;
	int len;
} U8Result;

static U8Result
utf8_dec(u8 *str, Size slen, int *err)
{
	static const u8 lens[] = {
		/* 0XXX */ 1, 1, 1, 1, 1, 1, 1, 1,
		/* 10XX */ 1, 1, 1, 1,  /* invalid */
		/* 110X */ 2, 2,
		/* 1110 */ 3,
		/* 1111 */ 4, /* maybe */
	};
	static const u32 pextmask[] = {
		0x7F000000, 0x1F3F0000, 0x0F3F3F00, 0x073F3F3F,
	};
	static const u32 emask[] = {
		0x80000000, 0xE0C00000, 0xF0C0C000, 0xF8C0C0C0,
	};
	static const u32 eexpect[] = {
		0x00000000, 0xC0800000, 0xE0808000, 0xF0808080,
	};

	u32 v = 0;
	switch (MIN(slen, 4)) { // branchy
	case 4: v |= ((u32)str[3] <<  0); // fallthrough
	case 3: v |= ((u32)str[2] <<  8); // fallthrough
	case 2: v |= ((u32)str[1] << 16); // fallthrough
	case 1: v |= ((u32)str[0] << 24); break;
	default: ASSERT(!"unreachable"); break; // we don't accept empty strings
	}

	U8Result ret = {0};
	ret.len = lens[str[0] >> 4];
	u32 idx = ret.len - 1;
	ret.cp  = pext32(v, pextmask[idx]);

	*err |= (v & emask[idx]) ^ (eexpect[idx]);
	*err |= ret.cp > 0x10FFFF; // out of range
	*err |= (ret.cp >> 11) == 0x1B; // surrogate
	#if 1
	static const u32 overlong[] = { 0x0, 0x80, 0x0800, 0x10000 };
	*err |= ret.cp < overlong[idx];
	#else // less memory, but more instruction
	static const u8 overlong[] = { 32, 7, 11, 16 };
	*err |= ret.cp < (u32)((u64)1 << overlong[idx]);
	#endif

	return ret;
}


//////////////
//  gcc -o out utf8-pext.c -march=x86-64-v3 -DTEST -g3 -fsanitize=address,undefined -fsanitize-trap
#if TEST

#include <stdio.h>

#define S(X) { (X), sizeof(X) - 1 }
#undef  TEST
#define TEST(Expect, Advanced, ...) do { \
	u8 s[] = { __VA_ARGS__ }; \
	int err = 0; \
	U8Result r = utf8_dec(s, sizeof s, &err); \
	u32 cp = r.cp; \
	ASSERT(Expect); \
	ASSERT((Advanced) == -1 || r.len == (Advanced)); \
} while (0)

// similar to libgrapheme's utf8 test vector
extern int
main(void)
{
	TEST(cp == 'A',     1, 'A');
	TEST(cp == '\0',    1, '\0', '\0');
	TEST(cp == 0xFF,    2, 0xC3, 0xBF);
	TEST(cp == 0xFFF,   3, 0xE0, 0xBF, 0xBF);
	TEST(cp == 0xFFFFF, 4, 0xF3, 0xBF, 0xBF, 0xBF);

	TEST(err,  -1, 0xFD); // invalid header
	TEST(err,  -1, 0xFB); // "

	TEST(err,  -1, 0xC3); // missing byte
	TEST(err,  -1, 0xC3, 0xFF); // invalid 2nd byte
	TEST(err,  -1, 0xC1, 0xBF); // overlong

	TEST(err,  -1, 0xE0); // missing bytes
	TEST(err,  -1, 0xE0, 0xBF); // missing byte
	TEST(err,  -1, 0xE0, 0x7F, 0xBF); // invalid 2nd byte
	TEST(err,  -1, 0xE0, 0xBF, 0x7F); // invalid 3rd byte
	TEST(err,  -1, 0xE0, 0x9F, 0xBF); // overlong
	TEST(err,  -1, 0xED, 0xA0, 0x80); // surrogate

	TEST(err,  -1, 0xF3);             // missing bytes
	TEST(err,  -1, 0xF3, 0xBF);       // "
	TEST(err,  -1, 0xF3, 0xBF, 0xBF); // "
	TEST(err,  -1, 0xF3, 0x7F, 0xBF, 0xBF); // invalid 2nd byte
	TEST(err,  -1, 0xF3, 0xBF, 0x7F, 0xBF); // "       3rd
	TEST(err,  -1, 0xF3, 0xBF, 0xBF, 0x7F); // "       4th
	TEST(err,  -1, 0xF0, 0x80, 0x81, 0xBF); // overlong
	TEST(err,  -1, 0xF4, 0x90, 0x80, 0x80); // out of range
	TEST(err,  -1, 0xF7, 0xBF, 0xBF, 0xBF); // "

	printf("[PASS]\n");
}
#endif

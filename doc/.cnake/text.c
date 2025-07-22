#ifndef C_TEXT
#define C_TEXT

/* If the return value == the sz passed in, this buffer is valid utf8 */
/* to be specific the sz passed in and the value returned are the number
 * of bytes not the number of utf8 encoded unicode characters */
u64 text_len(u8 *buf, u64 sz);
u64 term_len(u8 *buf, u64 sz); /* A sequence of 78 or fewer non-blank chars */
u64 line_len(u8 *buf, u64 sz);
u64 word_len(u8 *buf, u64 sz); /* A sequence of 26 or fewer letters */

/* ? inquire
 * ! instruct
 * . inform
 */

/* 3 to 26 or fewer words followed by a ? . ! */
u64 sentence_len(u8 *buf, u64 sz);
u64 non_sentence_len(u8 *buf, u64 sz);

int a_line(u8 c);
int a_space(u8 c);
int a_letter(u8 c);
int a_digit(u8 c);
int is_uadod(u8 c);
int a_dodad(u8 c);
int u_dohead(u8 c);

int a_vowel(u8 c);
int a_glide(u8 c);
int a_consonant(u8 c);

typedef enum {
  NUL = 0,
  SOH,STX,
  ETX,EOT,
  ENQ,ACK,
  BEL,BS,
  HT,LF,VT,FF,CR,
  SO,SI,
  DLE,
  DC1,DC2,DC3,DC4,
  NAK,SYN,
  ETB,CAN,
  EM,
  SUB,ESC,
  FS,GS,RS,US
} CC;

#define SP (US + 1)
#define DEL 127

char ascii_cc_str[32][4] = {
  "NUL",
  "SOH","STX",
  "ETX","EOT",
  "ENQ","ACK",
  "BEL","BS",
  "HT","LF","VT","FF","CR",
  "SO","SI",
  "DLE",
  "DC1","DC2","DC3","DC4",
  "NAK","SYN",
  "ETB","CAN",
  "EM",
  "SUB","ESC",
  "FS","GS","RS","US"};

#define UNICODES 1112064
/*
13312 - 19893 CJK Ideographs Extension A
19968 - 40869 CJK Ideographs
44032 - 55203 Hangul Syllables
55296 - 56191 Non-Private Use High Surrogates
56192 - 56319 Private Use High Surrogates
56320 - 57343 Low Surrogates
57344 - 63743 The Private Use Area
983040 - 1048573 Private Use
1048576 - 1114109 Private Use*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

u8 a_blank(u8 byte) {
  if (byte == SP) return 1;
  if (byte == LF) return 1;
  if (byte == CR) return 1;
  if (byte == VT) return 1;
  if (byte == HT) return 1;
  if (byte == FF) return 1;
  return 0;
}

u8 a_achar(u8 byte) {
  if ((byte > SP) && (byte < DEL)) {
    return 1;
  }
  return 0;
}

int u_dohead(u8 byte) {
  if (byte <= 191) return 0;
  if (byte >= 248) return 0;
  if (byte <= 223) return 2;
  if (byte <= 239) return 3;
  if (byte <= 247) return 4;
  return 0;
}

int is_unicode_tail(u8 byte) {
  if ((byte >= 128) && (byte <= 191)) return 1;
  return 0;
}

int is_unicode_neckbeard(u8 head, u8 neck) {
  if (head == 224) {
    if ((neck >= 160) && (neck <= 191)) return 1;
    printk("unicode neckbeard error!");
    return 0;
  }
  if (head == 237) {
    if ((neck >= 128) && (neck <= 159)) return 1;
    printk("unicode neckbeard error!");
    return 0;
  }
  if (head == 240) {
    if ((neck >= 144) && (neck <= 191)) return 1;
    printk("unicode neckbeard error!");
    return 0;
  }
  if (head == 244) {
    if ((neck >= 128) && (neck <= 143)) return 1;
    printk("unicode neckbeard error!");
    return 0;
  }
  return is_unicode_tail(neck);
}

u8 a_ctlcode(u8 byte) {
  if (byte == DEL) return 1;
  if ((byte > NUL) && (byte < SP)) return 1;
  return 0;
}

int a_ascii(u8 byte) {
  if ((byte > 0) && (byte < 128)) return 1;
  return 0;
}

u64 blank_len(u8 *buf, u64 sz) {
  u64 i;
  u8 byte;
  for (i = 0; i < sz; i++) {
    byte = buf[i];
    if (a_blank(byte)) continue;
    break;
  }
  return i;
}

#define TERM_MAX 78

u64 term_len(u8 *buf, u64 sz) {
  u64 n;
  char c;
  for (n = 0; n < sz; n++) {
    c = buf[n];
    if (a_blank(c)) return n;
    if (n == TERM_MAX) break;
  }
  return n;
}

u8 endpunch(u8 c) {
  if ((c == '?') || (c == '!') || (c == '.')) return c;
  return 0;
}

u64 sentence_len(u8 *buf, u64 sz) {
  u64 n = blank_len(buf, sz);
  for (; n < sz;) {
    u64 len;
    if (n > 0) {
      if (endpunch(buf[n - 1])) return n;
    }
    len = term_len(buf + n, sz - n);
    if (len == 0) exit(1);
    if (len == TERM_MAX) exit(1);
    printf("%.*s ", len, buf + n);
    n += len;
    len = blank_len(buf + n, sz - n);
    if (len == 0) return n;
    n += len;
  }
  return n;
}

static char *ascii_dodads = "!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~";
#define ASCII_NDODADS 32

int a_dodad(u8 c) {
  int n;
  char dod;
  for (n = 0; n < ASCII_NDODADS; n++) {
    dod = ascii_dodads[n];
    if (c == dod) return 1;
  }
  return 0;
}

int is_uadod(u8 c) {
  if (a_dodad(c)) return 1;
  return u_dohead(c);
}

int is_hexdigit(u8 c) {
  if (c == 'A') return 1;
  if (c == 'B') return 1;
  if (c == 'C') return 1;
  if (c == 'D') return 1;
  if (c == 'E') return 1;
  if (c == 'F') return 1;
  if (c == 'a') return 1;
  if (c == 'b') return 1;
  if (c == 'c') return 1;
  if (c == 'd') return 1;
  if (c == 'e') return 1;
  if (c == 'f') return 1;
  if (c == '0') return 1;
  if (c == '1') return 1;
  if (c == '2') return 1;
  if (c == '3') return 1;
  if (c == '4') return 1;
  if (c == '5') return 1;
  if (c == '6') return 1;
  if (c == '7') return 1;
  if (c == '8') return 1;
  if (c == '9') return 1;
  return 0;
}

int a_digit(u8 c) {
  if (c == '0') return 1;
  if (c == '1') return 1;
  if (c == '2') return 1;
  if (c == '3') return 1;
  if (c == '4') return 1;
  if (c == '5') return 1;
  if (c == '6') return 1;
  if (c == '7') return 1;
  if (c == '8') return 1;
  if (c == '9') return 1;
  return 0;
}

int number_len(u8 *buf, u64 sz) {
  u64 i;
  u8 byte;
  for (i = 0; i < sz; i++) {
    byte = buf[i];
    if (a_digit(byte)) continue;
    break;
  }
  return i;
}

int a_letter(u8 c) {
  if (c == 'A') return 1;
  if (c == 'B') return 1;
  if (c == 'C') return 1;
  if (c == 'D') return 1;
  if (c == 'E') return 1;
  if (c == 'F') return 1;
  if (c == 'G') return 1;
  if (c == 'H') return 1;
  if (c == 'I') return 1;
  if (c == 'J') return 1;
  if (c == 'K') return 1;
  if (c == 'L') return 1;
  if (c == 'M') return 1;
  if (c == 'N') return 1;
  if (c == 'O') return 1;
  if (c == 'P') return 1;
  if (c == 'Q') return 1;
  if (c == 'R') return 1;
  if (c == 'S') return 1;
  if (c == 'T') return 1;
  if (c == 'U') return 1;
  if (c == 'V') return 1;
  if (c == 'W') return 1;
  if (c == 'X') return 1;
  if (c == 'Y') return 1;
  if (c == 'Z') return 1;
  if (c == 'a') return 1;
  if (c == 'b') return 1;
  if (c == 'c') return 1;
  if (c == 'd') return 1;
  if (c == 'e') return 1;
  if (c == 'f') return 1;
  if (c == 'g') return 1;
  if (c == 'h') return 1;
  if (c == 'i') return 1;
  if (c == 'j') return 1;
  if (c == 'k') return 1;
  if (c == 'l') return 1;
  if (c == 'm') return 1;
  if (c == 'n') return 1;
  if (c == 'o') return 1;
  if (c == 'p') return 1;
  if (c == 'q') return 1;
  if (c == 'r') return 1;
  if (c == 's') return 1;
  if (c == 't') return 1;
  if (c == 'u') return 1;
  if (c == 'v') return 1;
  if (c == 'w') return 1;
  if (c == 'x') return 1;
  if (c == 'y') return 1;
  if (c == 'z') return 1;
  return 0;
}

int a_vowel(u8 c) {
  if ((c == 'a') || (c == 'A')) return 1;
  if ((c == 'e') || (c == 'E')) return 1;
  if ((c == 'i') || (c == 'I')) return 1;
  if ((c == 'o') || (c == 'O')) return 1;
  if ((c == 'u') || (c == 'U')) return 1;
  return 0;
}

int a_glide(u8 c) {
  if ((c == 'w') || (c == 'W')) return 1;
  if ((c == 'y') || (c == 'Y')) return 1;
  return 0;
}

int a_consonant(u8 c) {
  if ((a_letter(c)) && (!a_vowel(c))) return 1;
  return 0;
}

int a_letter_or_a_number(u8 c) {
  if (a_letter(c)) return 1;
  if (a_digit(c)) return 1;
  return 0;
}

int a_alphanumeric(u8 c) {
  return a_letter_or_a_number(c);
}

int a_space(u8 c) {
  if (c == SP) return 1;
  return 0;
}

int a_line(u8 c) {
  if (c == LF) return 1;
  return 0;
}

int aalphanumeric_len(u8 *buf, u64 sz) {
  u64 i;
  int have_alpha = 0;
  u8 byte;
  for (i = 0; i < sz; i++) {
    byte = buf[i];
    if (a_letter(byte)) {
      have_alpha++;
      continue;
    }
    if (a_digit(byte)) continue;
    return 0;
  }
  if (have_alpha) return 1;
  return 0;
}

void aletter_loop() {
  for (char letter = 'a'; letter < '{'; letter++) {
    printf("Letter %c\n", letter);
  }
}

int ascii_len(u8 *dat, u64 sz) {
  u8 b;
  u64 n = 0;
  for (n = 0; n < sz; n++) {
    b = dat[n];
    if (!a_ascii(b)) break;
  }
  return n;
}

#define WORD_LEN_MAX 26
#define WORD_MOST_VOWEL "euouae"
#define WORD_MOST_CONSONANT "tsktsk"

u64 word_len(u8 *buf, u64 sz) {
  if (!buf || (sz < 1) || (sz > WORD_LEN_MAX)) return 0;
  int i;
  char letter;
  int sillyness = 0;
  int srsly = 0;
  for (i = 0; i < sz; i++) {
    if (!a_letter(buf[i])) break;
    letter = buf[i];
    if ((a_vowel(letter)) || (a_glide(letter))) { sillyness++; srsly = 0; }
    if (a_consonant(letter)) { srsly++; sillyness = 0; }
    if ((sillyness > 6) || (srsly > 6)) { return i - 4; }
  }
  return i;
}

u64 line_len(u8 *buf, u64 sz) {
  int n;
  if (sz > 4096) sz = 4096;
  for (n = 0; n < sz; n++) {
    if (buf[n] == LF) break;
    if (buf[n] == NUL) break;
  }
  return n;
}

u64 text_len(u8 *buf, u64 sz) {
  u8 byte;
  u64 i;
  for (i = 0; i < sz; i++) {
    byte = buf[i];
    if (a_letter(byte)) continue;
    if (a_digit(byte)) continue;
    if (a_dodad(byte)) continue;
    if (a_blank(byte)) continue;
    int u = u_dohead(byte);
    if (u < 2) break;
    if ((i + u) > sz) break;
    if (u == 2) {
      if (is_unicode_tail(buf[i + 1])) {
        i += 1;
        continue;
      }
      break;
    } else if (u == 3) {
      if ((is_unicode_neckbeard(byte, buf[i + 1]))
       && (is_unicode_tail(buf[i + 2]))) {
        i += 2;
        continue;
      }
      break;
    } else if (u == 4) {
      if ((is_unicode_neckbeard(byte, buf[i + 1]))
       && (is_unicode_tail(buf[i + 2]))
       && (is_unicode_tail(buf[i + 3]))) {
        i += 3;
        continue;
      }
      break;
    }
  }
  return i;
}

u64 text_scan(u8 *buf, u64 sz) {
  if ((!buf) || (sz < 4)) exit(1);
  if (sz == 4) {
    printf("FourCC: %.*s\n", sz, buf);
    return 0;
  }
  printf("Text, %lu bytes\n", sz);
  for (u64 n = 0; n < sz;) {
    n += sentence_len(buf + n, sz - n);
    printf("\n");
  }
  return 0;
}

#endif

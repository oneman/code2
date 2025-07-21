
/* Standard 24bit 48khz gets us 384KB dat and 6KB nfo hz, amirite? yes */
typedef struct {
  u64 subframes;
  u8 lastbit;
} aes_stream

  u8 hdr;
  u8 dat[6];
  u8 nfo;

  //select subframe header
  if ((aes->frames % (192 * 2)) == 0) {
    if (aes->lastbit == 0) {
      hdr = 0b11101000;
    } else {
      hdr = 0b00010111;
    }
  } else {
    if ((aes->frames % 2) == 0) {
      if (aes->lastbit == 0) {
        hdr = 0b11100010;
      } else {
        hdr = 0b00011101;
      }
    } else {
      if (aes->lastbit == 0) {
        hdr = 0b11100100;
      } else {
        hdr = 0b00011011;
      }
    }
  }

  //copy sample?
  //valid bit = 0
  //user data bit = !
  //channel data bit from word;
  //parity bit = correction needed to make sample have even number of 1s and 0s



#include <stdio.h>
#include <unistd.h>
#include <gmp.h>

int main(int argc, char *argv[]) {
  printf("%d\n", 0b10000 | 0b01000 | 0b00010);
  mpz_t a, z;
  mpz_init_set_str(a, "1", 10);
  mpz_init_set_str(z, "1", 10);
  for (int i = 0; i < 213; i++) {
    mpz_ui_pow_ui(a, 26, i);
    //mpz_set_ui(a, i);
    char *bs = mpz_get_str(0, 2, a);
    int len = strlen(bs);
    int zeros = 0;
    int ones = 0;
    int changes = 0;
    for (int b = 0; b < len; b++) {
      if (bs[b] == '0') zeros++;
      if (bs[b] == '1') ones++;
      if ((b > 0) && ((bs[b] != bs[b - 1]))) changes++;
    }
    /*gmp_printf("1^%d: %Zd\n", i, a);*/
    printf("%zu bits %d zeros %d ones %d changes :: %s :: %d\n",
     strlen(bs), zeros, ones, changes, bs, i);
    free(bs);
  }
  mpz_clear(a);
  mpz_clear(z);
  return 0;
}

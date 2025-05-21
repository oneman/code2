#include <stdio.h>
#include <gmp.h>
#include <cairo.h>
#include <pipewire/pipewire.h>
 
int main(int argc, char *argv[]) {
  pw_init(&argc, &argv);
  fprintf(stdout, "Compiled with libpipewire %s\n"
                  "Linked with libpipewire %s\n",
                   pw_get_headers_version(),
                   pw_get_library_version());

  mpz_t n;
  mpz_init(n);
  mpz_clear(n);

  printf("cairo: %s\n", cairo_version_string());
  printf("gmp: %s\n", gmp_version);

  printf("ok\n");
  return 1;
}

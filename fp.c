#include <stdio.h>
#include <gmp.h>
#include <cairo.h>
#include <pipewire/pipewire.h>

/* ok ok ok */
#include <sys/types.h>
typedef float f32;
typedef double f64;
typedef u_int8_t u8;
typedef u_int16_t u16;
typedef u_int32_t u32;
typedef u_int64_t u64;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef size_t usize;
typedef ssize_t isize;

typedef struct {
  uint8_t r;
  uint8_t g;
  uint8_t b;
  uint8_t a;
} rgba32;

static char *letter_color[26] = {
  "amethyst", "blue", "caramel", "damnson", "ebony",
  "forest", "green", "honey", "iron", "jade",
  "khaki", "lime", "mellow", "navy", "orly",
  "pink", "quagmire", "red", "sky", "turquoise",
  "uranium", "version", "wine", "xanthin", "yellow",
  "zorange"
};

typedef enum {
  amethyst, blue, caramel, damnson, ebony,
  forest, green, honey, iron, jade,
  khaki, lime, mellow, navy, orly,
  pink, quagmire, red, sky, turquoise,
  uranium, version, wine, xanthin, yellow,
  zorange
} ncolor;

static const rgba32 colors26[26] = {
  {241,163,255}, {0,116,255}, {155,64,0}, {76,0,92}, {26,26,26},
  {0,92,48}, {42,207,72}, {255,205,153}, {126,126,126}, {149,255,181},
  {143,124,0}, {157,205,0}, {195,0,137}, {50,129,255}, {165,4,255},
  {255,169,187}, {66,102,0}, {255,0,0}, {94,241,243}, {0,153,143},
  {225,255,102}, {16,10,255}, {153,0,0}, {255,255,129},{255,225,0},
  {255,80,0}
};

#define CACHELINE_SZ 64
#define PAGE_SZ 4096
#define COMPUTER_SCREEN_WIDTH 1920
#define COMPUTER_SCREEN_HEIGHT 1080
#define COMPUTER_SCREEN_STRIDE COMPUTER_SCREEN_WIDTH * 4
#define COMPUTER_SCREEN_PIXELS COMPUTER_SCREEN_WIDTH * COMPUTER_SCREEN_HEIGHT

typedef struct {
  rgba32 *src;
  int w;
  int h;
  int pr[PAGE_SZ * PAGE_SZ];
  int nr;
  int nrj;
  rgba32 ds[COMPUTER_SCREEN_WIDTH * COMPUTER_SCREEN_HEIGHT];
} pxcop;

int pixel_region_count(pxcop *pc) {
  return pc->nr - pc->nrj;
}

int pixel_region_get(pxcop *pc, int x, int y, int w) {
  int n = (y * w) + x;
  int r = pc->pr[n];
  return r;
}

int pixel_region_new(pxcop *pc, int x, int y, int w) {
  int n = (y * w) + x;
  int r = ++pc->nr;
  pc->pr[n] = r;
  return r;
}

int pixel_region_join(pxcop *pc, int r, int x, int y, int w) {
  int n = (y * w) + x;
  int old_r = pc->pr[n];
  pc->pr[n] = r;
  if ((old_r) && (old_r != r)) {
    pc->nrj++;
    int start = 0;
    //if (y > 1) start = (y - 1) * w;
    for (int i = start; i < n; i++) {
      if (pc->pr[i] == old_r) {
        pc->pr[i] = r;
      }
    }
  }
  return r;
}

int rgbcmp(rgba32 *p1, rgba32 *p2) {
  if ((p1->r == p2->r) && (p1->g == p2->g) && (p1->b == p2->b)) return 1;
  return 0;
}

int px_scan(pxcop *pc, rgba32 *px, int w, int h) {
  if ((w < 1) || (h < 1) || !pc || !px) return 1;
  int x;
  int y;
  int r;
  u64 np = w * h;
  pc->w = w;
  pc->h = h;
  pc->src = px;
  printf("Area: %dx%d\n", w, h);
  printf("Pixels: %lu\n",  np);
  if (np > COMPUTER_SCREEN_PIXELS) {
    printf("So Many pixels!\n");
    return 0;
  }
  /*
  printf("4x Cell: %lu\n",  np / (4 * 4));
  printf("8x Cell: %lu\n",  np / (8 * 8));
  printf("16x Cell: %lu\n",  np / (16 * 16));
  printf("32x Cell: %lu\n",  np / (32 * 32));
  printf("64x Cell: %lu\n",  np / (64 * 64));
  */
  for (y = 0; y < h; y++) {
    for (x = 0; x < w; x++) {
      rgba32 *upleft = NULL;
      rgba32 *up = NULL;
      rgba32 *left = NULL;
      rgba32 *upright = NULL;
      rgba32 *cur = &px[(y * w) + x];
      if (x > 0) left = &px[(y * w) + (x - 1)];
      if (y > 0) {
        up = &px[((y - 1) * w) + x];
        if (x > 0) upleft = &px[((y - 1) * w) + (x - 1)];
        if (x < (w - 1)) upright = &px[((y - 1) * w) + (x + 1)];
      }
      if ((upleft) && (rgbcmp(cur, upleft))) {
        r = pixel_region_get(pc, x - 1, y - 1, w);
        pixel_region_join(pc, r, x, y, w);
      }
      if ((up) && (rgbcmp(cur, up))) {
        r = pixel_region_get(pc, x, y - 1, w);
        pixel_region_join(pc, r, x, y, w);
      }
      if ((left) && (rgbcmp(cur, left))) {
        r = pixel_region_get(pc, x - 1, y, w);
        pixel_region_join(pc, r, x, y, w);
      }
      if ((upright) && (rgbcmp(cur, upright))) {
        r = pixel_region_get(pc, x + 1, y - 1, w);
        pixel_region_join(pc, r, x, y, w);
      }
      if (!pixel_region_get(pc, x, y, w)) {
        pixel_region_new(pc, x, y, w);
      }
    }
  }
  return pixel_region_count(pc);
}

/* ko ko ko */
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
  cairo_surface_t *s = NULL;
  if (argc == 2) {
    s = cairo_image_surface_create_from_png(argv[1]);
    if (s) {
      if (cairo_surface_status(s)) return 1;
      pxcop *pc = malloc(sizeof(*pc));
      memset(pc, 0, sizeof(*pc));
      printf("scanning %s\n", argv[1]);
      int ret = px_scan(pc, (rgba32 *)cairo_image_surface_get_data(s),
        cairo_image_surface_get_width(s), cairo_image_surface_get_height(s));
      if (ret) { printf("Got %d regions\n", ret); }
      free(pc);
      cairo_surface_destroy(s);
    }
  }
  printf("ok\n");
  for (int L = 0; L < 26; L++) { printf("%s %d %d %d\n", letter_color[L], colors26[L].r,colors26[L].g,colors26[L].b); }
  return 1;
}

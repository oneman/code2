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
#define MAX_REGIONS 4096 * 4096
#define MONSTER_SCREEN_WIDTH 3440
#define MONSTER_SCREEN_HEIGHT 1440
#define MONSTER_SCREEN_STRIDE MONSTER_SCREEN_WIDTH * 4
#define MONSTER_SCREEN_PIXELS MONSTER_SCREEN_WIDTH * MONSTER_SCREEN_HEIGHT

typedef struct {
  rgba32 *src;
  int w;
  int h;
  int pr[MAX_REGIONS];
  int nr;
  int nrj;
  rgba32 ds[MONSTER_SCREEN_WIDTH * MONSTER_SCREEN_HEIGHT];
} pxcop;

void pxcop_reboot(pxcop *pc) {
  memset(pc, 0, sizeof(*pc));
}

int active_regions(pxcop *pc) {
  return pc->nr - pc->nrj;
}

int pixel_region(pxcop *pc, int x, int y, int w) {
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
  if ((p1->r == p2->r) &&
      (p1->g == p2->g) &&
      (p1->b == p2->b)) {
    return 0;
  }
  return 1;
}

int samergb(rgba32 *p1, rgba32 *p2) {
  if (rgbcmp(p1, p2) == 0) return 1;
  return 0;
}

int argbcmp(rgba32 *a, rgba32 *b) {
  if ((a->a == b->a) &&
      (a->r == b->r) &&
      (a->g == b->g) &&
      (a->b == b->b)) {
    return 0;
  }
  return 1;
}

int pixcmp(rgba32 *a, rgba32 *b) {
  if ((a->a == b->a) &&
      (a->r == b->r) &&
      (a->g == b->g) &&
      (a->b == b->b)) {
    return 0;
  }
  return 1;
}

void set_px_color(rgba32 *px, int x, int y, int w, rgba32 c) {
  rgba32 *pxl = &px[(y * w) + x];
  pxl->r = c.r;
  pxl->g = c.g;
  pxl->b = c.b;
}


void anal_dump(pxcop *pc) {
  static u64 N = 0;
  //static kr_file_set *fs = NULL;
  if (N == 0) {
    //static kr_fs_setup setup;
    //setup.nfiles++;
    //fs = kr_file_set_create(&setup);
  }
  N++;
  char path[64];
  snprintf(path, sizeof(path) - 1, "%s/%lu.png", "/home/demo/1", N);
  printf("Analysis file: %s\n", path);
  cairo_surface_t *cs;
  cairo_t *cr;
  cs = cairo_image_surface_create(CAIRO_FORMAT_RGB24, pc->w, pc->h);
  cr = cairo_create(cs);
  rgba32 *px = (rgba32 *)cairo_image_surface_get_data(cs);
  cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
  cairo_paint(cr);
  cairo_surface_flush(cs);
  for (int y = 0; y < pc->h; y++) {
    for (int x = 0; x < pc->w; x++) {
      int r = pixel_region(pc, x, y, pc->w);
      rgba32 c = colors26[r % 25];
      set_px_color(px, x, y, pc->w, c);
    }
  }
  cairo_surface_mark_dirty(cs);
  int scale = MONSTER_SCREEN_HEIGHT/pc->h;
  while ((scale * pc->w) > MONSTER_SCREEN_WIDTH) scale--;
  printf("Scale: %d %dx%d\n", scale, scale * pc->w, scale * pc->h);
  cairo_surface_t *cs2 = cairo_image_surface_create_for_data((u8 *)&pc->ds,
    CAIRO_FORMAT_RGB24,
    MONSTER_SCREEN_WIDTH,
    MONSTER_SCREEN_HEIGHT,
    MONSTER_SCREEN_STRIDE);
  cairo_t *cr2 = cairo_create(cs2);
  cairo_surface_mark_dirty(cs);
  for (int y = 0; y < pc->h; y++) {
    for (int x = 0; x < pc->w; x++) {
      rgba32 opixel = px[(y * pc->w) + x];
      for (int ys = 0; ys < scale; ys++) {
        for (int xs = 0; xs < scale; xs++) {
          pc->ds[(y * scale * MONSTER_SCREEN_WIDTH) + (ys * MONSTER_SCREEN_WIDTH) + (x * scale) + xs] = opixel;
        }
      }
    }
  }
  cairo_surface_mark_dirty(cs2);
  cairo_surface_write_to_png(cs2, path);
  cairo_destroy(cr);
  cairo_destroy(cr2);
  cairo_surface_destroy(cs);
  cairo_surface_destroy(cs2);
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
  if (np > MONSTER_SCREEN_PIXELS) {
    printf("So Many pixels!\n");
    return 0;
  }
  printf("4x Cell: %lu\n",  np / (4 * 4));
  printf("8x Cell: %lu\n",  np / (8 * 8));
  printf("16x Cell: %lu\n",  np / (16 * 16));
  printf("32x Cell: %lu\n",  np / (32 * 32));
  printf("64x Cell: %lu\n",  np / (64 * 64));
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
      if ((upleft) && (samergb(cur, upleft))) {
        r = pixel_region(pc, x - 1, y - 1, w);
        pixel_region_join(pc, r, x, y, w);
      }
      if ((up) && (samergb(cur, up))) {
        r = pixel_region(pc, x, y - 1, w);
        pixel_region_join(pc, r, x, y, w);
      }
      if ((left) && (samergb(cur, left))) {
        r = pixel_region(pc, x - 1, y, w);
        pixel_region_join(pc, r, x, y, w);
      }
      if ((upright) && (samergb(cur, upright))) {
        r = pixel_region(pc, x + 1, y - 1, w);
        pixel_region_join(pc, r, x, y, w);
      }
      if (!pixel_region(pc, x, y, w)) {
        pixel_region_new(pc, x, y, w);
      }
    }
  }
  u64 nr = active_regions(pc);
  printf(" Regions: [%lu]\n",  nr);
  anal_dump(pc);
  return active_regions(pc);
}

typedef struct {
  u8 *dat;
  size_t sz;
  size_t pos;
} bufnfo;

static cairo_status_t read_cb(void *user, u8 *data, u32 length) {
  bufnfo *n;
  n = (bufnfo *)user;
  if (n->pos + length > n->sz) return CAIRO_STATUS_READ_ERROR;
  memcpy(data, n->dat + n->pos, length);
  n->pos += length;
  return CAIRO_STATUS_SUCCESS;
}

size_t png_data_scan(u8 *buf, size_t sz) {
  size_t ret;
  cairo_surface_t *cs;
  bufnfo nfo;
  nfo.dat = buf;
  nfo.sz = sz;
  nfo.pos = 0;
  cs = cairo_image_surface_create_from_png_stream(read_cb, &nfo);
  if (cairo_surface_status(cs)) return 1;
  pxcop *pc = malloc(sizeof(*pc));
  pxcop_reboot(pc);
  ret = px_scan(pc,
      (rgba32 *)cairo_image_surface_get_data(cs),
      cairo_image_surface_get_width(cs),
      cairo_image_surface_get_height(cs));
  if (ret) {
    /*printf("Got %lu regions\n", ret);*/
  }
  free(pc);
  cairo_surface_destroy(cs);
  return ret;
}

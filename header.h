#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <xcb/xcb.h>
#include <gmp.h>
#include <cairo.h>
#include <cairo-xcb.h>
#include <pipewire/pipewire.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <inttypes.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <errno.h>
#include <signal.h>
#include <xkbcommon/xkbcommon.h>

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

int radical(int n) {
    if (n < 1) return 0;
    int radn = 1;
    if (n % 2 == 0) {
        radn *= 2;
        while (n % 2 == 0) n /= 2;
    }
    for (int i = 3; i * i <= n; i += 2) {
      if (n % i == 0) {
      radn *= i;
      while (n % i == 0) n /= i;
    }
  }
  return  n * radn;
}

#define W 1920
#define H 1080

static unsigned char img[W * H * 3];

float capsuleSDF(float px, float py, float ax, float ay, float bx, float by, float r) {
    float pax = px - ax, pay = py - ay, bax = bx - ax, bay = by - ay;
    float h = fmaxf(fminf((pax * bax + pay * bay) / (bax * bax + bay * bay), 1.0f), 0.0f);
    float dx = pax - bax * h, dy = pay - bay * h;
    return sqrtf(dx * dx + dy * dy) - r;
}

void alphablend(int x, int y, float alpha, float r, float g, float b) {
    unsigned char* p = img + (y * W + x) * 3;
    p[0] = (unsigned char)(p[0] * (1 - alpha) + r * alpha * 255);
    p[1] = (unsigned char)(p[1] * (1 - alpha) + g * alpha * 255);
    p[2] = (unsigned char)(p[2] * (1 - alpha) + b * alpha * 255);
}

void lineSDFAABB(float ax, float ay, float bx, float by, float r) {
    int x0 = (int)floor(fminf(ax, bx) - r);
    int x1 = (int) ceil(fmaxf(ax, bx) + r);
    int y0 = (int)floor(fminf(ay, by) - r);
    int y1 = (int) ceil(fmaxf(ay, by) + r);
    for (int y = y0; y <= y1; y++)
        for (int x = x0; x <= x1; x++)
            alphablend(x, y, fmaxf(fminf(0.5f - capsuleSDF(x, y, ax, ay, bx, by, r), 1.0f), 0.0f), 0.0f, 0.0f, 0.0f);
}
/*
int main() {
    memset(img, 255, sizeof(img));
    
    float cx = W * 0.5f, cy = H * 0.5f;
    for (int j = 0; j < 5; j++) {
        float r1 = fminf(W, H) * (j + 0.5f) * 0.085f;
        float r2 = fminf(W, H) * (j + 1.5f) * 0.085f;
        float t = j * PI / 64.0f, r = (j + 1) * 0.5f;
        for (int i = 1; i <= 64; i++, t += 2.0f * PI / 64.0f) {
            float ct = cosf(t), st = sinf(t);
            lineSDFAABB(cx + r1 * ct, cy - r1 * st, cx + r2 * ct, cy - r2 * st, r);
        }
    }
}*/

typedef struct {
  uint8_t r;
  uint8_t g;
  uint8_t b;
  uint8_t a;
} pxl;

typedef enum {
  amethyst, blue, caramel, damnson, ebony,
  forest, green, honey, iron, jade,
  khaki, lime, mellow, navy, orly,
  pink, quagmire, red, sky, turquoise,
  uranium, version, wine, xanthin, yellow,
  zorange
} ncolor;

typedef enum {
  alpha, bravo, charlie, delta, echo, foxtrot, golf, hotel, india, juliet, kilo,
  lima, mike, november, oscar, papa, quebec, romeo, sierra, tango, uniform,
  victor, whiskey, xray, yankee, zulu
} NATO;

static char *nato[26] = {
  "alpha", "bravo", "charlie", "delta", "echo", "foxtrot", "golf", "hotel",
  "india", "juliet", "kilo", "lima", "mike", "november", "oscar", "papa",
  "quebec", "romeo", "sierra", "tango", "uniform", "victor", "whiskey", "xray",
  "yankee", "zulu"
};
/*
static char *colorname[26] = {
  "amethyst", "blue", "caramel", "damnson", "ebony",
  "forest", "green", "honey", "iron", "jade",
  "khaki", "lime", "mellow", "navy", "orly",
  "pink", "quagmire", "red", "sky", "turquoise",
  "uranium", "version", "wine", "xanthin", "yellow",
  "zorange"
};
static const pxl colors26[26] = {
  {241,163,255}, {0,116,255}, {155,64,0}, {76,0,92}, {26,26,26},
  {0,92,48}, {42,207,72}, {255,205,153}, {126,126,126}, {149,255,181},
  {143,124,0}, {157,205,0}, {195,0,137}, {50,129,255}, {165,4,255},
  {255,169,187}, {66,102,0}, {255,0,0}, {94,241,243}, {0,153,143},
  {225,255,102}, {16,10,255}, {153,0,0}, {255,255,129},{255,225,0},
  {255,80,0}
};
*/
#define BIG_SZ 4210 * 2976 * 16

typedef struct {
  int nr;
  int nrj;
  int pr[BIG_SZ];
} pxr;

int pxr_count(pxr *pr) {
  return pr->nr - pr->nrj;
}

int pxr_get(pxr *pr, int x, int y, int w) {
  int n = (y * w) + x;
  int r = pr->pr[n];
  return r;
}

int pxr_new(pxr *pr, int x, int y, int w) {
  int n = (y * w) + x;
  int r = ++pr->nr;
  pr->pr[n] = r;
  /*printf("New Region: %d @ %dx%d\n", r, x, y);*/
  return r;
}

int pxr_join(pxr *pr, int r, int x, int y, int w) {
  int n = (y * w) + x;
  int old_r = pr->pr[n];
  pr->pr[n] = r;
  if ((old_r) && (old_r != r)) {
    pr->nrj++;
    int start = 0;
    for (int i = start; i < n; i++) {
      if (pr->pr[i] == old_r) {
        pr->pr[i] = r;
      }
    }
  }
  return r;
}

int pxlcmp(pxl *p1, pxl *p2) {
  if (((p1->r < 255) && (p2->r < 255)) && ((p1->g < 255) && (p2->g < 255))
   && ((p1->b < 255) && (p2->b < 255))) return 1;
  if ((p1->r == p2->r) && (p1->g == p2->g) && (p1->b == p2->b)) return 1;
  return 0;
}

int pxrscan(pxr *pr, pxl *px, int w, int h) {
  if ((w < 1) || (h < 1) || !pr || !px) return 1;
  int x;
  int y;
  int r;
  u64 np = w * h;
  printf("Area: %dx%d %lu MAX: %d\n",  w, h, np, BIG_SZ);
  if (np > BIG_SZ) { printf("To fn big many pixels!\n"); return 1; }
  for (y = 0; y < h; y++) {
    for (x = 0; x < w; x++) {
      pxl *upleft = NULL;
      pxl *up = NULL;
      pxl *left = NULL;
      pxl *upright = NULL;
      pxl *cur = &px[(y * w) + x];
      if (x > 0) left = &px[(y * w) + (x - 1)];
      if (y > 0) {
        up = &px[((y - 1) * w) + x];
        if (x > 0) upleft = &px[((y - 1) * w) + (x - 1)];
        if (x < (w - 1)) upright = &px[((y - 1) * w) + (x + 1)];
      }
      if ((upleft) && (pxlcmp(cur, upleft))) {
        r = pxr_get(pr, x - 1, y - 1, w);
        pxr_join(pr, r, x, y, w);
      }
      if ((up) && (pxlcmp(cur, up))) {
        r = pxr_get(pr, x, y - 1, w);
        pxr_join(pr, r, x, y, w);
      }
      if ((left) && (pxlcmp(cur, left))) {
        r = pxr_get(pr, x - 1, y, w);
        pxr_join(pr, r, x, y, w);
      }
      if ((upright) && (pxlcmp(cur, upright))) {
        r = pxr_get(pr, x + 1, y - 1, w);
        pxr_join(pr, r, x, y, w);
      }
      if (!pxr_get(pr, x, y, w)) {
        pxr_new(pr, x, y, w);
      }
    }
  }
  return pxr_count(pr);
}

int pxrprint(pxr *pr, pxl *px, int w, int h, char *filename) {
  if ((w < 1) || (h < 1) || !pr || !px) return 1;
  int x;
  int y;
  u64 r;
  u64 n = pr->nr + pr->nrj;
  char name[4096];
  memset(name, 0, 4096);
  sprintf(name, "%s.nfo", filename);  
  mkdir(name, S_IRWXU | S_IRWXG | S_IROTH);
  /*
  for (r = 0; r <= n; r++) {
    cairo_surface_t *s = NULL;
    s = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h);
    if (cairo_surface_status(s)) return 1;
    cairo_t *c = cairo_create(s);
    cairo_set_source_rgba(c, 1, 1, 1, 0);
    cairo_paint(c);
    cairo_destroy(c);
    cairo_surface_flush(s);
    pxl *npx = (pxl *)cairo_image_surface_get_data(s);
    int rpx = 0;
    for (y = 0; y < h; y++) {
      for (x = 0; x < w; x++) {
        if (r != pxr_get(pr, x, y, w)) continue;
        npx[(y * w) + x] = px[(y * w) + x];
        rpx++;
      }
    }
    if (rpx) {
      cairo_surface_mark_dirty(s);
      memset(name, 0, 4096);
      sprintf(name, "%s.nfo/%lu.png", filename, r);
      cairo_surface_write_to_png(s, name);
    }
    cairo_surface_destroy(s);
  }
  */
  for (r = 0; r <= n; r++) {
    int rt = -1;
    int rl = -1;
    int rr = 0;
    int rb = 0;
    for (y = 0; y < h; y++) {
      for (x = 0; x < w; x++) {
        if (r != pxr_get(pr, x, y, w)) continue;
        if (rt == -1) { rt = y; rl = x; }
        if (x < rl) rl = x;
        if (x > rr) rr = x;
        if (y > rb) rb = y;
      }
    }
    if ((rl == -1) && (rt == -1)) continue;
    int rw = (rr - rl) + 1;
    int rh = (rb - rt) + 1;
    printf("r: %lu %d,%d %dx%d\n", r, rl, rt, rw, rh);
    cairo_surface_t *s = NULL;
    s = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, rw, rh);
    if (cairo_surface_status(s)) return 1;
    cairo_t *c = cairo_create(s);
    cairo_set_source_rgb(c, 1, 0, 0);
    cairo_paint(c);
    pxl *npx = (pxl *)cairo_image_surface_get_data(s);
    for (y = 0; y < rh; y++) {
      for (x = 0; x < rw; x++) {
        npx[y * rw + x] = px[(y + rt) * w + (x + rl)];
      }
    }
    cairo_surface_mark_dirty(s);
    memset(name, 0, 4096);
    sprintf(name, "%s.nfo/%lu_out.png", filename, r);
    cairo_surface_write_to_png(s, name);
    cairo_surface_destroy(s);
  }
  return 0;
}
/*
int oldmain(int argc, char *argv[]) {
  mpz_t n;
  mpz_init(n);
  mpz_clear(n);
  pw_init(&argc, &argv);
  printf("pipewire %s/%s ", pw_get_headers_version(),
   pw_get_library_version());
  printf("cairo: %s gmp: %s\n", cairo_version_string(), gmp_version);
  cairo_surface_t *s = NULL;
  if (argc == 2) {
    s = cairo_image_surface_create_from_png(argv[1]);
    if (s) {
      if (cairo_surface_status(s)) return 1;
      pxr *pr;
      pr = malloc(sizeof(*pr));
      memset(pr, 0, sizeof(*pr));
      printf("scanning %s\n", argv[1]);
      pxrscan(pr, (pxl *)cairo_image_surface_get_data(s),
       cairo_image_surface_get_width(s),
       cairo_image_surface_get_height(s));
      pxrprint(pr, (pxl *)cairo_image_surface_get_data(s),
       cairo_image_surface_get_width(s),
       cairo_image_surface_get_height(s),
       argv[1]);
      free(pr);
      cairo_surface_destroy(s);
    }
  } else {
    for (int L = 0; L < 26; L++) { printf("%s %d %d %d\n",
      colorname[L], colors26[L].r, colors26[L].g, colors26[L].b);
    }
    int log[1024];
    memset(log,0,sizeof(log));
    u64 i = 2;
    for (;i < 1000000000;) {
      u64 n = i;
      u64 steps = 0;
      for (; n != 1;) {
        steps++;
        if ((n % 2) == 0) { n = n / 2; } else { n = (n * 3) + 1; }
      }
      log[steps]++;
      if (steps == 26) printf("742	 %10lu: %4lu steps #[%d]\n", i, steps, log[steps]);
    }
    for (i = 0; i < 1000; i++) {
      if (!log[i]) continue;
    //  printf("%10lu: %3d\n", i, log[i]);
    }
  }
  return 1;
}

*/

/*
int ethnet = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

printf("sock fd be %d\n", ethnet);


int backtrace(void *buffer[.size], int size);
char **backtrace_symbols(void *const buffer[.size], int size);

int timerfd_create(int clockid, int flags);
int timerfd_settime(int fd, int flags,
                    const struct itimerspec *new_value,
		    struct itimerspec *_Nullable old_value);
int timerfd_gettime(int fd, struct itimerspec *curr_value);

int signalfd(int fd, const sigset_t *mask, int flags);

int epoll_create1();
int epoll_ctl(int epfd, int op, int fd, struct epoll_event *_Nullable event);
int epoll_wait(int epfd, struct epoll_event events[.maxevents], int maxevents, int timeout);
int efd = eventfd(2601, EFD_NONBLOCK);
printf("efd %d cool lol\n", efd); 
      printf("wtf %s\n\r", strerror(errno));
*/

u32 cpdat(unsigned char *out, unsigned char *dat, int x, int y) {
  if (x < 0) x = 0;
  if (y < 0) y = 0;
  if (x > (1920*25)) x = 1920 * 25;
  if (y > (1080*25)) y = 1080 * 25;
  int stride = (1920*3*26);
  for (int line = 0; line < 1080; line++) {
    u32 yy = (line * stride) + (y * stride);
    u32 xx = x * 3;
    for (int px = 0; px < 1920; px++) {
      u32 xxx = xx + (px * 3);
      out[(line * (4 * 1920)) + (px * 4)] = dat[yy + xxx];
      out[(line * (4 * 1920)) + (px * 4) + 1] = dat[yy + xxx + 1];
      out[(line * (4 * 1920)) + (px * 4) + 2] = dat[yy + xxx + 2];
    }
  }
  return 1920*1080;
}

static xcb_visualtype_t *find_visual(xcb_connection_t *c, xcb_visualid_t visual)
{
    xcb_screen_iterator_t screen_iter = xcb_setup_roots_iterator(xcb_get_setup(c));

    for (; screen_iter.rem; xcb_screen_next(&screen_iter)) {
        xcb_depth_iterator_t depth_iter = xcb_screen_allowed_depths_iterator(screen_iter.data);
        for (; depth_iter.rem; xcb_depth_next(&depth_iter)) {
            xcb_visualtype_iterator_t visual_iter = xcb_depth_visuals_iterator(depth_iter.data);
            for (; visual_iter.rem; xcb_visualtype_next(&visual_iter))
                if (visual == visual_iter.data->visual_id)
                    return visual_iter.data;
        }
    }

    return NULL;
}

int yabemain(void) {
  int R;
  int FD;
  long SZ = 4205260800;
  unsigned char *DAT;
  if ((FD = memfd_create("pixmap-framebuffer", 0)) < 0) return 1;
  printf("mem FD\n");
  do { R = ftruncate(FD, SZ); } while (R < 0 && errno == EINTR);
  printf("ftruncate 4205260800\n");
	DAT = mmap(NULL, SZ, PROT_READ | PROT_WRITE, MAP_SHARED, FD, 0);
	printf("mmap DAT\n");

  srand(4205260800);
  int iteration = 0;
  int randx = 0;
  int randy = 0;

  randx = rand() % (1920*25);
  randy = rand() % (1080*25);
  printf("%d randy %d\n", randx, randy); 

	for (unsigned long i = 0; i < 676; i++) {
	  /* continue; */
	  char c1 = 96 + 1 + (i / 26);
	  char c2 = 96 + 1 + (i % 26);
	  char filename[256];
	  snprintf(filename, 256, "map/%c/%c/%s%s.png",
	                               c1, c2, nato[c1 - 97], nato[c2 - 97]);
	  cairo_surface_t *cst = cairo_image_surface_create_from_png(filename);
	  if (cairo_image_surface_get_width(cst) != 1920) exit(1);
    if (cairo_image_surface_get_height(cst) != 1080) exit(1);
    if (cairo_image_surface_get_stride(cst) != 1920 * 4) exit(1);
    if (cairo_image_surface_get_format(cst) != CAIRO_FORMAT_RGB24) {
      printf("not rite bro\n"); exit(1);
    }
    unsigned char *dat = cairo_image_surface_get_data(cst);
    unsigned long x0 = (i % 26) * (1920*3);
    /*y0 = (i / 26) * (161740800); (49920*3=149760) * 1080 < x 28080 */
    //printf("%zu // 676 :: %zu :: %zu :: %zu\n", i + 1, x0, y0, x0 * y0);
    printf("%zu :: %c %zu [%zu] :: %c %zu :: xof %zu pxyoff [[%zu]] %zu\n",
		    i + 1,
		    c1,
		    ((i / 26) + 1),
		    ((i / 26) + 0) * 161740800,
		    c2,
		    ((i % 26) + 1),
		    (((i / 26) + 0) * 161740800) + x0,
		    x0,
		    (676 - (i + 1)));
    //(4205260800/26/6220800 /26/(26*1920*3))//1026675 = 4205260800/4096
    // (4205260800/149760) == 28080
    int stride = 1920 * 26 * 3;
    for (int yy = 0; yy < 1080; yy++) {
      int pyy = yy * stride;
      for (int xx = 0; xx < 1920; xx++) {
        int pxy = pyy + (xx * 3);
        DAT[pxy] = dat[(yy * 4) + (xx * 4)];
        DAT[pxy + 1] = dat[(yy * 4) + (xx * 4) + 1];
        DAT[pxy + 2] = dat[(yy * 4) + (xx * 4) + 2];
        //DAT[(x0*y0) + pxy + (xx * 3) + 1] = dat[px * 4 + 1];
        //DAT[(x0*y0) + pxy + (xx * 3) + 2] = dat[px * 4 + 2];
      }
    }
    cairo_surface_destroy(cst);
	  /*
	  cairo_surface_t *cst = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 1920,
	                                                                        1080);
	  printf("cstat: %d\n", cairo_surface_status(cst));
    cairo_t *cr = cairo_create(cst);
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_paint(cr);
    cairo_new_path(cr);
    cairo_set_source_rgba(cr, (double)colors26[c1 - 97].r / 255,
     (double)colors26[c1 - 97].g / 255, (double)colors26[c1 - 97].b / 255, 1);
    cairo_rectangle(cr, 0, 0, 1920, 540);
    cairo_fill(cr);
    cairo_new_path(cr);
    cairo_set_source_rgba(cr, (double)colors26[c2 - 97].r / 255,
     (double)colors26[c2 - 97].g / 255, (double)colors26[c2 - 97].b / 255, 1);
    cairo_rectangle(cr, 0, 540, 1920, 540);
    cairo_fill(cr);
    cairo_surface_flush(cst);
    cairo_surface_write_to_png(cst, filename);
    cairo_destroy(cr);
    cairo_surface_destroy(cst);
    */
  }
  
  u32 d = 0;
  unsigned char bite = 0;
  u32 nd = 0;
  for (;;) {
    if (DAT[d] != bite) {
      bite = DAT[d];
      nd++;
      printf("we changed %u at %u\n", d, nd);
    }    
    d = d + 4096;
    if (d >= SZ) break;
  }
  
  printf("all set\n");

    xcb_connection_t *c;
    xcb_screen_t *screen;
    xcb_window_t window;
    uint32_t mask[2];
    xcb_visualtype_t *visual;
    xcb_generic_event_t *event;
    cairo_surface_t *surface;
    cairo_t *cr;

    c = xcb_connect(NULL, NULL);
    if (xcb_connection_has_error(c)) {
        fprintf(stderr, "Could not connect to X11 server");
        return 1;
    }

    mask[0] = 1;
    mask[1] = XCB_EVENT_MASK_EXPOSURE       | XCB_EVENT_MASK_BUTTON_PRESS  |
              XCB_EVENT_MASK_BUTTON_RELEASE | XCB_EVENT_MASK_BUTTON_MOTION |
              XCB_EVENT_MASK_ENTER_WINDOW   | XCB_EVENT_MASK_LEAVE_WINDOW  |
              XCB_EVENT_MASK_KEY_PRESS      | XCB_EVENT_MASK_KEY_RELEASE;
    screen = xcb_setup_roots_iterator(xcb_get_setup(c)).data;
    window = xcb_generate_id(c);
    xcb_create_window(c, XCB_COPY_FROM_PARENT, window, screen->root,
            0, 0, 1920, 1080, 0,
            XCB_WINDOW_CLASS_INPUT_OUTPUT,
            screen->root_visual,
            XCB_CW_OVERRIDE_REDIRECT | XCB_CW_EVENT_MASK,
            mask);
    xcb_map_window(c, window);

    visual = find_visual(c, screen->root_visual);
    if (visual == NULL) {
        fprintf(stderr, "Some weird internal error...?!");
        xcb_disconnect(c);
        return 1;
    }
    surface = cairo_xcb_surface_create(c, window, visual, 1920, 1080);
    cr = cairo_create(surface);
    cairo_set_source_rgb(cr, 0, 1, 0);
    cairo_paint(cr);
    cairo_surface_flush(surface);


cairo_surface_t *surfy = cairo_image_surface_create(CAIRO_FORMAT_RGB24, 1920,
  1080);
    cairo_t *cr_mid = cairo_create(surfy);
            cairo_set_source_rgb(cr_mid, 0, 0, 1);
            cairo_paint(cr_mid);
            cairo_surface_flush(surfy);
unsigned char *dat = cairo_image_surface_get_data(surfy);

            cairo_set_source_rgb(cr, 0, 1, 0);
            cairo_paint(cr);
            cairo_surface_flush(surface);

int popoff = 0;


  char *filnym = "/demo/2025-06-02-143000_1920x1080_scrot.png";
  cairo_surface_t *pngcst = cairo_image_surface_create_from_png(filnym);
  if (cairo_image_surface_get_format(pngcst) != CAIRO_FORMAT_RGB24) {
    printf("not rite bro\n"); exit(1);
  }
  unsigned char *pngdat = cairo_image_surface_get_data(pngcst);
//  cairo_t *cr_png = cairo_create(pngcst);
    
  cairo_set_source_surface(cr, pngcst, 0, 0);
  cairo_paint(cr);
  cairo_surface_flush(surface);
  
  xcb_flush(c);  
  usleep(17 * 1000); /* f'vsync this is Xorg */
  xcb_flush(c);
  usleep(3000 * 1000); /* f'vsync this is Xorg */

for(;;) {
  iteration++;
  if (popoff == 3) break;
    cpdat(dat, DAT, randx, randy);
    cairo_surface_mark_dirty(surfy);
    cairo_set_source_surface(cr, surfy, 0, 0);
    cairo_paint(cr);
    cairo_surface_flush(surface);
    xcb_flush(c);  
    usleep(17 * 1000); /* f'vsync this is Xorg */
    if (iteration % 3 == 0) {
      cairo_set_source_surface(cr, pngcst, 0, 0);
      cairo_paint(cr);
      cairo_surface_flush(surface);
    }
    usleep(17 * 1000); /* f'vsync this is Xorg */
    printf("x\n");
    /*while ((event = xcb_wait_for_event(c))) {*/
    while ((event = xcb_poll_for_event(c))) {
        switch (event->response_type & ~0x80) {
        case XCB_MOTION_NOTIFY:
          printf("butkey motion notif\n");
          break;
        case XCB_ENTER_NOTIFY:
          printf("enter\n");
          break;
        case XCB_LEAVE_NOTIFY:
          printf("leave\n");
          break;          
        case XCB_BUTTON_RELEASE:
          printf("but rel\n");
          break;
        case XCB_BUTTON_PRESS:
          printf("but press\n");
          popoff++;
          break;
        case XCB_KEY_PRESS:
          printf("key press\n");
          popoff++;
          break;
        case XCB_KEY_RELEASE:
          printf("key rel\n");
          break;
        case XCB_EXPOSE:
          printf("xpose\n");
            /* Avoid extra redraws by checking if this is
             * the last expose event in the sequence
             */
            if (((xcb_expose_event_t *) event)->count != 0)
                break;
            break;
        default:
          printf("def ev\n");
          break;
        }
        free(event);
        xcb_flush(c);
    }
    usleep(17 * 1000); /* f'vsync this is Xorg */
 
 
    randx = rand() % (1920*25);
    randy = rand() % (1080*25);
    printf("%d randy %d\n", randx, randy); 

 
    
    }
    
    cairo_surface_finish(surface);
    cairo_surface_destroy(surface);
    xcb_disconnect(c);
    return 0;
}

#define cunt const unsigned long int

cunt COMBLITTERSZ = 33642086400;
cunt COMBYTESZ = COMBLITTERSZ / 8;
/*
 4205260800 = 33642086400 / 8
 4205260800 = 49920 * 28080 * 3
 161740800 = 4205260800 / 26
 149760 = (((49920*28080*3)/26)/1080)
 49920 = 1920*26
 1080 = 161740800/(1920*3*26);
 1026675    = 33642086400/32768
 4205260800 = 33642086400/8
 4096       = 4205260800/1026675
 149760     = 4205260800/(26*1080)
 149760     = 1920*26*3
 28080      = 1080*26
 4205260800 = (1026675*4096)
 4205260800 = 149760*28080
 rad 390 (1026675*4096)/10782720
 585  = ((1920*3*26)*16)/4096
 4096 = ((1920*3*26)*16)/585
 93450240 = (373800960/4)
 1    = ((2^32)/(2^26))/32/2
 4096 = 4205260800/1755/585
 16 = (585*4096)/(1920*3*26)
 4205260800 = (1920*3*26*16)*1755
 4205260800 = 2396160*3*585
 16 = 2396160/(1920*3*26)
 0 = (1920*3*26*16)-(4096*585)
 thus every 2396160 we (16 * stride) are on a 4096 block and left
 26 = ((4205260800/1080)/(1920*3))/26
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <inttypes.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <errno.h>
#include <signal.h>
#include <wayland-client.h>
#include <wayland-client-protocol.h>
#include <xkbcommon/xkbcommon.h>

#include <krad/app/debug.h>
#include <krad/image/pool.h>
#include <krad/image/convert.h>

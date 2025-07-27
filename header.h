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

#include <stdio.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/time.h>

#if !defined(_app_spinlock_H)
# define _app_spinlock_H (1)

typedef int kr_spinlock;

void kr_spin_lock(kr_spinlock *lock);
void kr_spin_unlock(kr_spinlock *lock);
#define kr_spin_init kr_spin_unlock


#define MM __ATOMIC_SEQ_CST

void kr_spin_lock(kr_spinlock *lock) {
  int exp;
  for (;;) {
    exp = 0;
    if (__atomic_compare_exchange_n(lock, &exp, 1, 0, MM, MM)) break;
  }
}

void kr_spin_unlock(kr_spinlock *lock) {
  __atomic_store_n(lock, 0, MM);
}
#endif

#if !defined(_system_priority_H)
#define _system_priority_H (1)

#include <krad/system/types.h>

int kr_priority_set(kr_priority priortiy);

#include <errno.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/capability.h>
#include <krad/system/priority.h>
#include <linux/sched.h>
#include <krad/system/linux_priority.c>
#include <krad/app/debug.h>

typedef enum {
  KR_CAPS_UNKNOWN,
  KR_CAPS_NONE,
  KR_CAPS_REALTIME,
  KR_CAPS_REALTIME_IO
} kr_caps;

typedef struct {
  int policy;
  int nice;
  int rt_priority;
  int io_priority;
  int io_class;
} kr_priority_spec;

/*static kr_caps caps_level = KR_CAPS_UNKNOWN;*/

static const kr_priority_spec kr_priority_level[] = {
  [KR_PRIORITY_NORMAL] = {
    .policy = SCHED_OTHER,
    .nice = 0,
    .io_priority = 4,
    .io_class = IOPRIO_CLASS_BE
  },
  [KR_PRIORITY_REALTIME] = {
    .policy = SCHED_FIFO,
    .rt_priority = 20,
    .io_priority = 4,
    .io_class = IOPRIO_CLASS_RT
  },
  [KR_PRIORITY_REALTIME_IO] = {
    .policy = SCHED_FIFO,
    .rt_priority = 30,
    .io_priority = 0,
    .io_class = IOPRIO_CLASS_RT
  },
  [KR_PRIORITY_NONINTERACTIVE] = {
    .policy = SCHED_OTHER,
    .nice = 5,
    .io_priority = 7,
    .io_class = IOPRIO_CLASS_BE
  },
  [KR_PRIORITY_TRANSCODER] = {
    .policy = SCHED_BATCH,
    .nice = 10,
    .io_priority = 7,
    .io_class = IOPRIO_CLASS_BE
  }
};

/*
static const char *kr_ioprio_class[] = {
  [IOPRIO_CLASS_NONE] = "NONE",
  [IOPRIO_CLASS_IDLE] = "IDLE",
  [IOPRIO_CLASS_BE] = "BE",
  [IOPRIO_CLASS_RT] = "RT",
};

static const char *kr_sched_policy[] = {
  [SCHED_OTHER] = "OTHER",
  [SCHED_BATCH] = "BATCH",
  [SCHED_IDLE] = "IDLE",
  [SCHED_ISO] = "ISO",
  [SCHED_FIFO] = "FIFO",
  [SCHED_RR] = "RR"
};

static void kr_caps_debug() {
  switch (caps_level) {
    case KR_CAPS_UNKNOWN:
      printk("Cap: Unknown");
      break;
    case KR_CAPS_NONE:
      printk("Caps: None");
      break;
    case KR_CAPS_REALTIME:
      printk("Caps: Realtime");
      break;
    case KR_CAPS_REALTIME_IO:
      printk("Caps: Realtime and Realtime IO");
      break;
  }
}

static void kr_caps_get() {
  int ret;
  cap_t caps;
  cap_flag_value_t permitted;
  cap_flag_value_t effective;
  caps_level = KR_CAPS_NONE;
  caps = cap_get_proc();
  if (!caps) return;
  ret = cap_get_flag(caps, CAP_SYS_NICE, CAP_EFFECTIVE, &effective);
  if (ret) return;
  ret = cap_get_flag(caps, CAP_SYS_NICE, CAP_PERMITTED, &permitted);
  if (ret) return;
  if (effective && permitted) caps_level = KR_CAPS_REALTIME;
  ret = cap_get_flag(caps, CAP_SYS_ADMIN, CAP_EFFECTIVE, &effective);
  if (ret) return;
  ret = cap_get_flag(caps, CAP_SYS_ADMIN, CAP_PERMITTED, &permitted);
  if (ret) return;
  if (effective && permitted) caps_level = KR_CAPS_REALTIME_IO;
}*/

void kr_priority_debug(pid_t tid) {
  return;
} /*
  int ret;
  int which;
  int who;
  //int ioprio;
  int class;
  //const char *class_name;
  struct sched_attr attr;
  //const char *sched_policy_name;
  ret = 0;
  memset(&attr, 0, sizeof(attr));
  //class_name = "";
  //ioprio = 0;
  kr_caps_get();
  kr_caps_debug();
  ret = sched_getattr(tid, &attr, sizeof(struct sched_attr), 0);
  //sched_policy_name = kr_sched_policy[attr.sched_policy];
  if (ret == -1) {
    printke("sched_getattr: %s", strerror(errno));
  }
  which = IOPRIO_WHO_PROCESS;
  who = tid;
  ret = ioprio_get(which, who);
  if (ret == -1) {
    printke("ioprio_get: %s", strerror(errno));
  } else {
    //ioprio = IOPRIO_PRIO_DATA(ret);
    class = IOPRIO_PRIO_CLASS(ret);
    if (class == IOPRIO_CLASS_NONE) {
      if (attr.sched_policy == SCHED_IDLE)
        class = IOPRIO_CLASS_IDLE;
      else if (attr.sched_policy == SCHED_FIFO
       || attr.sched_policy == SCHED_RR)
        class = IOPRIO_CLASS_RT;
      else
        class = IOPRIO_CLASS_BE;
    }
    //class_name = kr_ioprio_class[class];
    printk("sched: [ policy: '%s' nice: '%d' priority: '%d' ]"
     " ioprio: [ class: '%s', prio: '%d' ]",  sched_policy_name,
     attr.sched_nice, attr.sched_priority, class_name, ioprio);
  }
}*/

int kr_priority_set(kr_priority priority) {
  return 1;
}/*
  int ret;
  int fail;
  int ioprio_val;
  struct sched_attr attr;
  const kr_priority_spec *prio;
  if ((priority < KR_PRIORITY_MIN) || (priority > KR_PRIORITY_MAX)) {
    printke("Priority: Invalid value %d", priority);
    return -1;
  }
  fail = 0;
  prio = &kr_priority_level[priority];
  memset(&attr, 0, sizeof(attr));
  attr.size = sizeof(attr);
  attr.sched_policy = prio->policy;
  attr.sched_nice = prio->nice;
  attr.sched_priority = prio->rt_priority;
  exit(1);
  ret = sched_setattr(0, &attr, 0);
  if (ret == -1) {
    fail = errno;
    printke("Priority: sched_setattr %s", strerror(fail));
  }
  if (!((priority == KR_PRIORITY_REALTIME ||
   priority == KR_PRIORITY_REALTIME_IO) &&
   caps_level != KR_CAPS_REALTIME_IO)) {
    ioprio_val = IOPRIO_PRIO_VALUE(prio->io_class, prio->io_priority);
    ret = ioprio_set(IOPRIO_WHO_PROCESS, 0, ioprio_val);
    if (ret == -1) {
      fail = errno;
      printke("Priority: ioprio_set %s", strerror(fail));
    }
  }
  if (fail) {
    if (caps_level == KR_CAPS_UNKNOWN) kr_caps_get();
  }
  return 0;
}*/

#endif

static kr_spinlock print_lock;

static uint64_t krad_unixtime();

static uint64_t krad_unixtime() {
  uint64_t seconds;
  struct timeval tv;
  gettimeofday(&tv, NULL);
  seconds = tv.tv_sec;
  return seconds;
}

void kr_print_noop(char* format, ...) {
  return;
}

void kr_print(char* format, ...) {
  va_list args;
  kr_spin_lock(&print_lock);
  va_start(args, format);
  vfprintf(stdout, format, args);
  va_end(args);
  fprintf(stdout, "\n");
  kr_spin_unlock(&print_lock);
}

void kr_print_err(char* format, ...) {
  va_list args;
  kr_spin_lock(&print_lock);
  fprintf(stderr, "Error: %"PRIu64" :: ", krad_unixtime());
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  fprintf(stderr, "\n");
  kr_spin_unlock(&print_lock);
}

void failfast(char* format, ...) {
  va_list args;
  kr_spin_lock(&print_lock);
  fprintf(stderr, "***FAIL!: %"PRIu64" :: ", krad_unixtime());
  va_start(args, format);
  vfprintf(stderr, format, args);
  va_end(args);
  fprintf(stderr, "\n");
  exit(1);
}

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
  if (pngdat) printf("yay pngdat %p\n", pngdat);
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


#if !defined(_mem_mem_H)
# define _mem_mem_H (1)

#include <stddef.h>

#define KR_KB 1024
#define KR_MB (1000 * KR_KB)

/*#define KR_DEBUG_MEM 1*/

#define member_sizeof(type, member) sizeof(((type *)0)->member)

#define kr_amem(y, z) struct { uint8_t (y)[z];}__attribute__((aligned(8)));

#if defined(KR_DEBUG_MEM)
#include <krad/app/debug.h>
#define KR_MEMALLOCSTR "Mem: alloc on %s %d"
#define kr_alloc(x) kr_alloc_real(x); printk(KR_MEMALLOCSTR, __FILE__, __LINE__)
#define KR_MEMALLOCZSTR "Mem: allocz on %s %d"
#define kr_allocz(x, y) kr_allocz_real(x, y); printk(KR_MEMALLOCZSTR, __FILE__, __LINE__)
#else
#define kr_allocz kr_allocz_real
#define kr_alloc kr_alloc_real
#endif

void kr_mem_debug(const char *type, size_t sz);
void *kr_allocz_real(size_t nelem, size_t elsize);
void *kr_alloc_real(size_t size);

#endif

#include <stdlib.h>
#include <krad/app/debug.h>

#if defined(KR_DEBUG_MEM)
static size_t kr_mem_sz = 0;
#endif

void kr_mem_debug(const char *type, size_t sz) {
#if defined(KR_DEBUG_MEM)
  size_t total;
  total = __atomic_add_fetch(&kr_mem_sz, sz, __ATOMIC_SEQ_CST);
  if (sz < KR_KB) {
    printk("Mem: %s %zu Bytes", type, sz);
  } else if (sz < KR_MB) {
    printk("Mem: %s %zu KB", type, sz / KR_KB);
  } else {
    printk("Mem: %s %zu MB", type, sz / KR_MB);
  }
  printk("Mem: total %zu MB", total / KR_MB);
#endif
}

void *kr_allocz_real(size_t nelem, size_t elsize) {
  kr_mem_debug("allocZ", nelem * elsize);
  return calloc(nelem, elsize);
}

void *kr_alloc_real(size_t size) {
  kr_mem_debug("alloc", size);
  return malloc(size);
}

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sched.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <sys/timerfd.h>
#include <sys/time.h>
#include <pthread.h>
#include <krad/app/debug.h>


#include <sys/signalfd.h>
#include <signal.h>
#include <errno.h>
#include <sys/epoll.h>

#if !defined(_loop_loop_H)
# define _loop_loop_H (1)

#include <krad/system/priority.h>

typedef struct kr_loop kr_loop;
typedef struct kr_loop_setup kr_loop_setup;
typedef struct kr_event kr_event;
typedef struct kr_mainloop_setup kr_mainloop_setup;
typedef int (kr_handler)(kr_event *);
typedef int (kr_startup_handler)(kr_loop *, void *);
typedef int (kr_shutdown_handler)(void *);

struct kr_event {
  int fd;
  uint32_t events;
  void *user;
  kr_handler *handler;
};

typedef enum {
  KR_SUPERLOOP,
  KR_SUBLOOP
} kr_loop_type;

struct kr_loop_setup {
  char name[16];
  kr_priority priority;
  kr_loop *master;
};

struct kr_mainloop_setup {
  void *user;
  kr_startup_handler *startup_handler;
  kr_shutdown_handler *shutdown_handler;
  kr_handler *signal_handler;
};

int kr_loop_del(kr_loop *loop, int fd);
int kr_loop_close(kr_loop *loop, int fd);
int kr_loop_mod(kr_loop *loop, kr_event *event);
int kr_loop_add(kr_loop *loop, kr_event *event);
int kr_loop_add_timeout(kr_loop *loop, int ms, kr_handler *handler, void *user);
int kr_loop_destroy(kr_loop *loop);
kr_loop *kr_loop_create(kr_loop_setup *setup);
int kr_loop_setup_init(kr_loop_setup *setup);
void kr_mainloop(kr_mainloop_setup *setup);

#endif


enum kr_loop_state {
  KR_LOOP_START = 0,
  KR_LOOP_LOOPING,
  KR_LOOP_DONE
};

#define KR_LOOP_NEVENTS 64
#define KR_LOOP_NHARNESSES 65536

struct kr_loop {
  pthread_t thread;
  kr_priority priority;
  kr_loop_type type;
  int ed;
  int pd;
  int state;
  char name[16];
  kr_event watch[KR_LOOP_NHARNESSES];
  kr_loop *master;
};

static int loop_destruct(kr_event *event) {
  kr_loop *loop;
  loop = (kr_loop *)event->user;
  loop->state = KR_LOOP_DONE;
  return 0;
}

static void loop_wait(kr_loop *loop, int ms) {
  int n;
  int i;
  int ret;
  int err;
  kr_event *event;
  struct epoll_event events[KR_LOOP_NEVENTS];
  n = epoll_wait(loop->pd, events, KR_LOOP_NEVENTS, ms);
  if (((ms == -1) && (n < 1)) || (n < 0)) {
    err = errno;
    printke("Loop: error on epoll wait %s", strerror(err));
    /*
    if (err == EINTR) continue;
    break;
    */
    return;
  }
  for (i = 0; i < n; i++) {
    event = (kr_event *)events[i].data.ptr;
    event->events = events[i].events;
    ret = event->handler(event);
    if (ret == -1) {
      loop->state = KR_LOOP_DONE;
      break;
    }
  }
}

static int subloop_event(kr_event *event) {
  kr_loop *loop;
  loop = (kr_loop *)event->user;
  loop_wait(loop, 0);
  return 0;
}

static void loop_cycle(kr_loop *loop) {
  while (loop->state == KR_LOOP_LOOPING) {
    loop_wait(loop, -1);
  }
}

static void *loop_thread(void *arg) {
  int ret;
  int err;
  kr_loop *loop;
  loop = (kr_loop *)arg;
  loop->state = KR_LOOP_LOOPING;
  ret = prctl(PR_SET_NAME, loop->name, NULL, NULL, NULL);
  if (ret == -1) {
    err = errno;
    printke("Loop: prctl %s", strerror(err));
  }
  if (loop->priority > KR_PRIORITY_MIN && loop->priority <= KR_PRIORITY_MAX) {
    ret = kr_priority_set(loop->priority);
    if (ret) {
      printke("Loop: setting priority");
    }
  }
  printk("Loop: %s starting", loop->name);
  loop_cycle(loop);
  printk("Loop: %s exiting", loop->name);
  loop->state = KR_LOOP_DONE;
  return NULL;
}

int kr_loop_del(kr_loop *loop, int fd) {
  int ret;
  int err;
  kr_event *watch;
  if (loop == NULL) return -1;
  if (fd < 0 || fd >= KR_LOOP_NHARNESSES) return -2;
  watch = &loop->watch[fd];
  if (watch == NULL) return -3;
  ret = epoll_ctl(loop->pd, EPOLL_CTL_DEL, fd, NULL);
  watch->fd = 0;
  if (ret != 0) {
    err = errno;
    printke("Loop: epoll ctl del %s", strerror(err));
    return -5;
  }
  return 0;
}

int kr_loop_close(kr_loop *loop, int fd) {
  int ret;
  ret = kr_loop_del(loop, fd);
  if (ret != 0) return ret;
  return close(fd);
}

int kr_loop_mod(kr_loop *loop, kr_event *event) {
  int ret;
  int err;
  kr_event *watch;
  struct epoll_event ev;
  if (loop == NULL) return -1;
  if (event == NULL) return -2;
  if (event->fd < 0 || event->fd >= KR_LOOP_NHARNESSES) return -3;
  watch = &loop->watch[event->fd];
  if (watch == NULL) return -4;
  *watch = *event;
  memset(&ev, 0, sizeof(ev));
  ev.events = watch->events;
  ev.data.ptr = watch;
  ret = epoll_ctl(loop->pd, EPOLL_CTL_MOD, watch->fd, &ev);
  if (ret != 0) {
    err = errno;
    printke("Loop: epoll ctl mod %s", strerror(err));
    return -5;
  }
  return 0;
}

int kr_loop_add(kr_loop *loop, kr_event *event) {
  int ret;
  struct epoll_event ev;
  if (loop == NULL) return -1;
  if (event == NULL) return -2;
  if (event->fd < 0 || event->fd >= KR_LOOP_NHARNESSES) return -3;
  if (event->handler == NULL) return -4;
  if (loop->watch[event->fd].fd == 0) {
    loop->watch[event->fd] = *event;
    memset(&ev, 0, sizeof(ev));
    ev.events = event->events;
    ev.data.ptr = &loop->watch[event->fd];
    ret = epoll_ctl(loop->pd, EPOLL_CTL_ADD, event->fd, &ev);
    if (ret != 0) {
      printke("Loop: epoll ctl add event fd to loop pd fail");
      return -5;
    }
    return 0;
  }
  return -6;
}

int kr_loop_add_timeout(kr_loop *loop, int ms, kr_handler *handler,
 void *user) {
  int tfd;
  int ret;
  kr_event event;
  struct itimerspec new_value;
  time_t sec;
  long nsec;
  if (loop == NULL) return -1;
  if (handler == NULL) return -2;
  if (ms <= 0) return -3;
  tfd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
  if (tfd < 0) {
    printke("Loop: timerfd_create fail");
    return -5;
  }
  memset(&new_value, 0, sizeof(new_value));
  sec = ms / 1000;
  nsec = (ms % 1000) * 1000000;
  new_value.it_value.tv_sec = sec;
  new_value.it_value.tv_nsec = nsec;
  new_value.it_interval.tv_sec = sec;
  new_value.it_interval.tv_nsec = nsec;
  ret = timerfd_settime(tfd, 0, &new_value, NULL);
  if (ret != 0) {
    printke("Loop: timerfd settime fail");
    close(tfd);
    return -5;
  }
  memset(&event, 0, sizeof(event));
  event.fd = tfd;
  event.handler = handler;
  event.user = user;
  event.events = EPOLLIN;
  ret = kr_loop_add(loop, &event);
  if (ret != 0) {
    close(tfd);
    return ret;
  }
  return 0;
}

int kr_loop_destroy(kr_loop *loop) {
  if (loop == NULL) return -1;
  printk("Loop: destroy");
  uint64_t u;
  int s;
  u = 666;
  if (loop->type == KR_SUPERLOOP) {
    s = write(loop->ed, &u, sizeof(uint64_t));
    if (s != sizeof(uint64_t)) {
      printk("Loop: error writing to loop ed");
    }
    pthread_join(loop->thread, NULL);
  }
  if (loop->type == KR_SUBLOOP) {
    kr_loop_del(loop->master, loop->pd);
  }
  close(loop->ed);
  close(loop->pd);
  free(loop);
  printk("Loop: destroyed");
  return 0;
}

kr_loop *kr_loop_create(kr_loop_setup *setup) {
  int ret;
  kr_loop *loop;
  kr_event harness;
  if (setup == NULL) return NULL;
  loop = calloc(1, sizeof(*loop));
  memcpy(loop->name, setup->name, sizeof(loop->name));
  loop->priority = setup->priority;
  if (!setup->master) {
    loop->type = KR_SUPERLOOP;
  } else {
    loop->type = KR_SUBLOOP;
  }
  loop->ed = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (loop->ed == -1) {
    printke("Loop: efd evenfd created failed");
    free(loop);
    return NULL;
  }
  loop->pd = epoll_create1(EPOLL_CLOEXEC);
  if (loop->pd == -1) {
    printke("Loop: pd epoll created failed");
    close(loop->ed);
    free(loop);
    return NULL;
  }
  harness.fd = loop->ed;
  harness.user = loop;
  harness.events = EPOLLIN;
  harness.handler = loop_destruct;
  ret = kr_loop_add(loop, &harness);
  if ((ret == 0) && (loop->type == KR_SUPERLOOP)) {
    pthread_create(&loop->thread, NULL, loop_thread, (void *)loop);
  }
  if ((ret != 0) || ((!loop->thread) && (loop->type == KR_SUPERLOOP))) {
    printf("Thread launch Error: %s", strerror(errno));
    close(loop->ed);
    close(loop->pd);
    free(loop);
    return NULL;
  }
  if (loop->type == KR_SUBLOOP) {
    loop->master = setup->master;
    harness.fd = loop->pd;
    harness.user = loop;
    harness.events = EPOLLIN;
    harness.handler = subloop_event;
    ret = kr_loop_add(setup->master, &harness);
    if (ret != 0) {
      printke("Subloop: could not add to my master %d", ret);
    }
  }
  return loop;
}

int kr_loop_setup_init(kr_loop_setup *setup) {
  if (!setup) return -1;
  memset(setup->name, 0, sizeof(setup->name));
  setup->master = NULL;
  setup->priority = KR_PRIORITY_NORMAL;
  return 0;
}

static int mainloop_signal_setup() {
  int sfd;
  sigset_t mask;
  sigemptyset(&mask);
  sigfillset(&mask);
  if (sigprocmask(SIG_BLOCK, &mask, NULL) != 0) {
    fprintf(stderr, "Mainloop: Could not set signal mask!");
    exit(1);
  }
  sfd = signalfd(-1, &mask, SFD_NONBLOCK | SFD_CLOEXEC);
  if (sfd == -1) {
    fprintf(stderr, "Mainloop: could not setup signalfd");
    exit(1);
  }
  return sfd;
}

void kr_mainloop(kr_mainloop_setup *setup) {
  int ret;
  int sfd;
  int fail;
  kr_loop *loop;
  kr_event signal_event;
  if (setup == NULL) return;
  fail = 0;
  sfd = -1;
  ret = 1;
  loop = calloc(1, sizeof(*loop));
  loop->pd = epoll_create1(EPOLL_CLOEXEC);
  if (loop->pd == -1) {
    printke("Loop: pd epoll created failed");
    fail = 1;
  }
  if (!fail) {
    sfd = mainloop_signal_setup();
    ret = setup->startup_handler(loop, setup->user);
    if (ret != 0) fail = 1;
  }
  if (!fail) {
    signal_event.fd = sfd;
    signal_event.events = EPOLLIN;
    signal_event.user = setup->user;
    signal_event.handler = setup->signal_handler;
    kr_loop_add(loop, &signal_event);
    loop->state = KR_LOOP_LOOPING;
    loop_cycle(loop);
    ret = setup->shutdown_handler(setup->user);
  }
  close(loop->pd);
  close(sfd);
  free(loop);
  exit(ret);
}

#if !defined(_mem_pool_H)
# define _mem_pool_H (1)

#define KR_PAGESIZE 4096

typedef struct kr_pool kr_pool;

#include <inttypes.h>
#include <stddef.h>

typedef struct {
  uint32_t slices;
  size_t size;
  int shared;
  void *overlay;
  size_t overlay_sz;
} kr_pool_setup;

int kr_pool_fd(kr_pool *pool);
int kr_pool_size(kr_pool *pool);
int kr_pool_slice_size(kr_pool *pool);
int kr_pool_offsetof(kr_pool *pool, void *slice);
void *kr_pool_iterate_active(kr_pool *pool, int *count);
int kr_pool_slice_ref(kr_pool *pool, void *slice);
int kr_pool_release(kr_pool *pool, void *slice);
int kr_pool_atomic_release(kr_pool *pool, void *slice);
void *kr_pool_atomic_slice(kr_pool *pool);
void *kr_pool_slice(kr_pool *pool);
void *kr_pool_slice_num(kr_pool *pool, int num);
int kr_pool_avail(kr_pool *pool);
int kr_pool_active(kr_pool *pool);
int kr_pool_slices(kr_pool *pool);
int kr_pool_overlay_get_copy(kr_pool *pool, void *overlay);
void *kr_pool_overlay_get(kr_pool *pool);
int kr_pool_overlay_set(kr_pool *pool, void *overlay);
int kr_pool_destroy(kr_pool *pool);
kr_pool *kr_pool_open(int fd, kr_pool_setup *setup);
kr_pool *kr_pool_create(kr_pool_setup *setup);

void kr_pool_debug(kr_pool *pool);

#endif

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <inttypes.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <assert.h>
#include <signal.h>
/*#include <sys/memfd.h>*/
#include <krad/app/spinlock.h>
#include <krad/app/debug.h>



#define KR_POOL_MAX 1048576
#define KR_POOL_64 64
#define CACHELINE_SZ KR_POOL_64
#define MM __ATOMIC_SEQ_CST

struct kr_pool {
  uint64_t *use;
  uint8_t *ref;
  void *data;
  uint32_t slices;
  uint32_t active;
  void *map;
  void *overlay;
  size_t slice_size;
  size_t info_size;
  size_t total_size;
  size_t overlay_sz;
  size_t overlay_actual_sz;
  size_t internal_sz;
  uint32_t use_max;
  uint8_t shared;
  int fd;
  kr_spinlock lock;
};

static kr_pool *pool_map(int fd, kr_pool_setup *setup);

int kr_pool_fd(kr_pool *pool) {
  if (pool == NULL) return -1;
  if (pool->shared != 1) return -1;
  return pool->fd;
}

int kr_pool_size(kr_pool *pool) {
  if (pool == NULL) return -1;
  return pool->total_size;
}

int kr_pool_slice_size(kr_pool *pool) {
  if (pool == NULL) return -1;
  return pool->slice_size;
}

int kr_pool_offsetof(kr_pool *pool, void *slice) {
  if (pool == NULL) return -1;
  if (slice == NULL) return -1;
  /* FIXME check that slice is in pool */
  return slice - pool->map;
}

int kr_pool_avail(kr_pool *pool) {
  if (pool == NULL) return -1;
  return pool->slices - pool->active;
}

int kr_pool_active(kr_pool *pool) {
  if (pool == NULL) return -1;
  return pool->active;
}

int kr_pool_slices(kr_pool *pool) {
  if (pool == NULL) return -1;
  return pool->slices;
}

void *kr_pool_iterate_active(kr_pool *pool, int *count) {
  uint64_t mask;
  int n;
  int i;
  int iter;
  if ((pool == NULL) || (count == NULL)) return NULL;
  if ((*count < 0) || (*count >= pool->slices)) return NULL;
  mask = 1;
  i = 0;
  iter = 0;
  n = *count;
  if (*count >= KR_POOL_64) {
    i = *count / KR_POOL_64;
    n = n % KR_POOL_64;
  }
  mask = mask << n;
  while (*count < pool->slices) {
    if ( (iter > 0) && ((*count % KR_POOL_64) == 0) ) {
      mask = 1;
      i++;
    }
    if ((pool->use[i] & mask) != 0) {
      return pool->data + (pool->slice_size * (*count)++);
    } else {
      (*count)++;
      mask = mask << 1;
    }
    iter++;
  }
  (*count) = 0;
  return NULL;
}

void *kr_pool_slice_num(kr_pool *pool, int num) {
  int i;
  int n;
  uint64_t mask;
  if (pool == NULL) return NULL;
  if ((num < 0) || (num >= pool->slices)) return NULL;
  mask = 1;
  i = 0;
  n = num;
  if (num >= KR_POOL_64) {
    i = num / KR_POOL_64;
    n = num % KR_POOL_64;
  }
  mask = mask << n;
  if ((pool->use[i] & mask) != 0) {
    return pool->data + (pool->slice_size * num);
  } else {
    pool->use[i] = pool->use[i] | mask;
    pool->active++;
    return pool->data + (pool->slice_size * num);
  }
}

/*
void *kr_pool_iterate_state(kr_pool *pool, int *count) {

  return NULL;
}

void *kr_pool_iterate_type(kr_pool *pool, int *count) {

  return NULL;
}

void *kr_pool_iterate_type_state(kr_pool *pool, int *count) {
  return NULL;
}
*/

void *kr_pool_overlay_get(kr_pool *pool) {
  if (pool == NULL) return NULL;
  if (pool->overlay == NULL) return NULL;
  if (pool->overlay_sz == 0) return NULL;
  return pool->overlay;
}

int kr_pool_overlay_set(kr_pool *pool, void *overlay) {
  if (pool == NULL) return -1;
  if (pool->overlay_sz == 0) return -2;
  memcpy(pool->overlay, overlay, pool->overlay_sz);
  return 0;
}

int kr_pool_overlay_get_copy(kr_pool *pool, void *overlay) {
  if ((pool == NULL) || (overlay == NULL)) return -2;
  if (pool->overlay == NULL) return -3;
  memcpy(overlay, pool->overlay, pool->overlay_sz);
  return pool->overlay_sz;
}

int kr_pool_release(kr_pool *pool, void *slice) {
  uint64_t mask;
  void *last;
  int i;
  size_t num;
  if ((pool == NULL) || (slice == NULL)) return -2;
  last = pool->data + (pool->slice_size * (pool->slices - 1));
  if (slice < pool->data || slice > last) {
   return -3;
  }
  kr_spin_lock(&pool->lock);
  num = (slice - pool->data) / pool->slice_size;
  i = 0;
  if (num >= KR_POOL_64) {
    i = num / KR_POOL_64;
    num = num % KR_POOL_64;
  }
  mask = (uint64_t)1 << num;
  if (((pool->use[i] & mask) != 0)) {
      pool->use[i] = pool->use[i] ^ mask;
      pool->active--;
      kr_spin_unlock(&pool->lock);
      return 0;
  }
  kr_spin_unlock(&pool->lock);
  return -1;
}

int kr_pool_slice_ref(kr_pool *pool, void *slice) {
  int i;
  int j;
  uint64_t mask;
  if ((pool == NULL) || (slice == NULL)) return -2;
  mask = 1;
  for (i = j = 0; i < pool->slices; i++) {
    if ( (i > 0) && ((i % KR_POOL_64) == 0)) {
      mask = 1;
      j++;
    }
    if (((pool->use[j] & mask) != 0)
        && (slice == (pool->data + (pool->slice_size * i)))) {
      pool->ref[i]++;
      return pool->ref[i];
    }
    mask = mask << 1;
  }
  return -1;
}

int kr_pool_atomic_release(kr_pool *pool, void *slice) {
  uint64_t mask;
  int i;
  size_t num;
  if ((pool == NULL) || (slice == NULL)) return -2;
  num = (slice - pool->data) / pool->slice_size;
  i = 0;
  if (num >= KR_POOL_64) {
    i = num / KR_POOL_64;
    num = num % KR_POOL_64;
  }
  mask = (uint64_t)1 << num;
  __atomic_xor_fetch(&pool->use[i], mask, MM);
  __atomic_sub_fetch(&pool->active, 1, MM);
  return 0;
}

void *kr_pool_atomic_slice(kr_pool *pool) {
  int i;
  int j;
  int ix;
  int len;
  uint64_t use;
  uint64_t new;
  uint64_t mask;
  if (pool == NULL) return NULL;
  for (i = 0; i < pool->slices; i += 64) {
    mask = 1;
    len = 64;
    if ((i + len) > pool->slices) len = pool->slices - i;
    ix = i / 64;
    __atomic_load(&pool->use[ix], &use, MM);
    for (j = 0; j < len; j++) {
      if ((use & mask) == 0) {
        new = use | mask;
        if (__atomic_compare_exchange(&pool->use[ix], &use, &new, 0, MM, MM)) {
          __atomic_add_fetch(&pool->active, 1, MM);
          return pool->data + (pool->slice_size * (i + j));
        } else {
          i -= 64;
          break;
        }
      }
      mask = mask << 1;
    }
  }
  return NULL;
}

void *kr_pool_slice(kr_pool *pool) {
  int i;
  int j;
  uint64_t mask;
  if (pool == NULL) return NULL;
  mask = 1;
  kr_spin_lock(&pool->lock);
  for (i = j = 0; i < pool->slices; i++) {
    if ( (i > 0) && ((i % KR_POOL_64) == 0)) {
      mask = 1;
      j++;
    }
    if ((pool->use[j] & mask) == 0) {
      pool->use[j] = pool->use[j] | mask;
      pool->active++;
      kr_spin_unlock(&pool->lock);
      return pool->data + (pool->slice_size * i);
    }
    mask = mask << 1;
  }
  kr_spin_unlock(&pool->lock);
  return NULL;
}

void kr_pool_debug(kr_pool *pool) {
  int i;
  if (pool == NULL) return;
  printk("\npool info");
  printk("pool slices: %d", kr_pool_slices(pool));
  printk("pool active: %d", kr_pool_active(pool));
  printk("pool avail: %d", kr_pool_avail(pool));
  printk("pool use_max: %d", pool->use_max);
  if (pool->use_max > 1) {
    for (i = 0; i < pool->use_max; i++) {
      if (pool->use[i])
        printk("pool use[%d]: %"PRIu64"", i, pool->use[i]);
    }
  } else {
    printk("pool use[0]: %"PRIu64"", pool->use[0]);
  }
  printk("pool info size: %zu", sizeof(kr_pool));
  printk("pool info act size: %zu", pool->info_size);
  printk("pool overlay size: %zu", pool->overlay_sz);
  printk("pool overlay act size: %zu", pool->overlay_actual_sz);
  printk("pool slice size: %zu", pool->slice_size);
  printk("pool slices size: %zu", pool->slice_size * pool->slices);
  printk("pool internal size: %zu", pool->internal_sz);
  printk("pool total size: %zu\n", pool->total_size);
}

int kr_pool_destroy(kr_pool *pool) {
  int ret;
  if (pool == NULL) return -1;
  /*kr_pool_debug(pool);*/
  if (pool->shared == 1) {
    close(pool->fd);
  }
  ret = munlock(pool->map, pool->total_size);
  if (ret) printke("Pool munlock: ret %d", ret);
  ret = munmap(pool->map, pool->total_size);
  if (ret) printke("Pool destroy: ret %d", ret);
  return ret;
}

static kr_pool *pool_map(int fd, kr_pool_setup *setup) {
  char filename[] = "/tmp/krad-shm-XXXXXX";
  static const int prot = PROT_READ | PROT_WRITE;
  int flags;
  int mod;
  int ret;
  kr_pool pool;
  if (setup == NULL) return NULL;
  if (setup->slices == 0) return NULL;
  if (setup->slices > KR_POOL_MAX) return NULL;
  if (setup->size == 0) return NULL;
  memset(&pool, 0, sizeof(kr_pool));
  pool.overlay_sz = setup->overlay_sz;
  pool.overlay_actual_sz = pool.overlay_sz;
  pool.info_size = sizeof(kr_pool);
  mod = pool.info_size % CACHELINE_SZ;
  if (mod) {
    pool.info_size += CACHELINE_SZ - mod;
  }
  mod = pool.overlay_sz % CACHELINE_SZ;
  if (mod) {
    pool.overlay_actual_sz += CACHELINE_SZ - mod;
  }
  pool.slices = setup->slices;
  pool.slice_size = setup->size;
  mod = pool.slice_size % CACHELINE_SZ;
  if (mod) {
    pool.slice_size += CACHELINE_SZ - mod;
  }
  pool.use_max = (1 + ( (pool.slices - 1) / KR_POOL_64));
  pool.internal_sz = sizeof(uint64_t) * pool.use_max;
  pool.internal_sz += sizeof(uint8_t) * pool.slices;
  mod = pool.internal_sz % CACHELINE_SZ;
  if (mod) {
    pool.internal_sz += CACHELINE_SZ - mod;
  }
  pool.total_size = (pool.slices * pool.slice_size);
  pool.total_size += pool.info_size;
  pool.total_size += pool.overlay_actual_sz;
  pool.total_size += pool.internal_sz;
  mod = pool.total_size % KR_PAGESIZE;
  if (mod) {
    pool.total_size += KR_PAGESIZE - mod;
  }
  if (!setup->shared) {
    pool.shared = 0;
    pool.fd = -1;
    flags = MAP_PRIVATE | MAP_ANONYMOUS;
  } else {
    pool.shared = 1;
    flags = MAP_SHARED;
    if (fd != -1) {
      pool.fd = fd;
    } else {
      if (0) {
        /*pool.fd = memfd_create("kr_pool", MFD_CLOEXEC | MFD_ALLOW_SEALING);*/
      } else {
        pool.fd = mkostemp(filename, O_CLOEXEC);
        if (pool.fd < 0) {
          printke("Pool: open %s failed: %m\n", filename);
          return NULL;
        }
        unlink(filename);
      }
    }
    if (ftruncate(pool.fd, pool.total_size) < 0) {
      printke("ftruncate failed: %m\n");
      close(pool.fd);
      return NULL;
    }
  }
  pool.map = mmap(NULL, pool.total_size, prot, flags, pool.fd, 0);
  if (pool.map == MAP_FAILED) {
    printke("Pool: mmap");
    if (pool.shared) close(pool.fd);
    return NULL;
  }
  ret = mlock(pool.map, pool.total_size);
  if (ret) printke("Pool mlock: %s", strerror(ret));
  kr_mem_debug("pool", pool.total_size);
  pool.data = pool.map + pool.internal_sz;
  pool.data += (pool.info_size + pool.overlay_actual_sz);
  if (pool.overlay_sz != 0) {
    pool.overlay = pool.map + pool.internal_sz + pool.info_size;
    if (setup->overlay != NULL) {
      kr_pool_overlay_set(&pool, setup->overlay);
    }
  }
  pool.use = pool.map;
  pool.ref = pool.map + ( sizeof(uint64_t) * pool.slices );
  memcpy(pool.map + pool.internal_sz, &pool, sizeof(kr_pool));
  return (kr_pool *)(pool.map + pool.internal_sz);
}

kr_pool *kr_pool_open(int fd, kr_pool_setup *setup) {
  if (fd < 0 || setup->shared != 1) return NULL;
  return pool_map(fd, setup);
}

kr_pool *kr_pool_create(kr_pool_setup *setup) {
  return pool_map(-1, setup);
}

typedef struct kr_pool kr_image_pool;

kr_image_pool *kr_image_pool_create(kr_image *image, size_t len);
int kr_image_pool_getimage(kr_image_pool *image_pool, kr_image *image);
int kr_image_pool_destroy(kr_image_pool *image_pool);

kr_image_pool *kr_image_pool_create(kr_image *image, size_t len) {
  kr_pool_setup setup;
  setup.slices = len;
  setup.overlay = image;
  setup.overlay_sz = sizeof(kr_image);
  setup.shared = 1;
  setup.size = image->info.w * image->info.h * 4;
  return kr_pool_create(&setup);
}

int kr_image_pool_getimage(kr_image_pool *image_pool, kr_image *image) {
  kr_pool_overlay_get_copy(image_pool, image);
  image->px[0] = kr_pool_slice(image_pool);
  if (image->px[0] == NULL) return 0;
  /* FIXME */
  image->px[1] = image->px[0] + (image->ps[0] * image->info.h);
  image->px[2] = image->px[1] + (image->ps[1] * (image->info.h/2));
  image->px[3] = 0;
  return 1;
}

int kr_image_pool_destroy(kr_image_pool *image_pool) {
  return kr_pool_destroy(image_pool);
}

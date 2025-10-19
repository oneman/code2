#include "header.h"

int CX = 0;
int CY = 0;
u8 K[8] = {0,0,0,0,0,0,0,0};
u8 *m = 0;
int X = 49920 - 49920;
int Y = 28080 - 28080;
int W = 0;
int H = 0;
u8 *P = 0;

void draw(void) {
  static int pulse = 0;
  static int pulsemod = 1;
  pulse += pulsemod;
  if (pulse == 0) pulsemod = 1;
  if (pulse == 60) pulsemod = -1;
  for (int y = 0; y < H; y++) {
    for (int x = 0; x < W; x++) {
      u32 pxy = 0;
      pxy += Y * 1920 * 26 * 3;
      pxy += y * 1920 * 26 * 3;
      pxy += X * 3;
      pxy += x * 3;
      P[(y * W * 4) + (x * 4) + 0] = m[pxy + 0];
      P[(y * W * 4) + (x * 4) + 1] = m[pxy + 1];
      P[(y * W * 4) + (x * 4) + 2] = m[pxy + 2];
      if ((x >= CX) && (y >= CY)) {
        if ((x <= (CX + 10)) && (y <= (CY + 12))) {
          P[(y * W * 4) + (x * 4) + 0] = 0;
          P[(y * W * 4) + (x * 4) + 1] = 0;
          P[(y * W * 4) + (x * 4) + 2] = 192 + pulse;
        }
      }
    }
  }
}

struct dctx {
  u32 fb_id[2];
  u32 current_fb_id;
  int crtid;
  u64 swap_count;
  u8 *pixmap1;
  u8 *pixmap2;
  int pixmap_sz;
  int complete;
};

static void vblank(int fd, unsigned int frame, unsigned int sec,
 unsigned int usec, void *data) {
  printf("vblank\n");
}

void pageflip(int fd, u32 frame, u32 sec, u32 usec, void *data) {
  struct dctx *context;
  unsigned int new_fb_id;
  context = (struct dctx *)data;
  if (context->complete == 1) { context->complete = 2; return; }
  if (context->current_fb_id == context->fb_id[0]) {
    new_fb_id = context->fb_id[1];
    P = context->pixmap2;
  } else {
    new_fb_id = context->fb_id[0];
    P = context->pixmap1;
  }
  draw();
  drmModePageFlip(fd, context->crtid, new_fb_id, DRM_MODE_PAGE_FLIP_EVENT,
   context);
  context->current_fb_id = new_fb_id;
}

#define USBKEY_LCTRL  0x01 /* controL*/
#define USBKEY_LSHIFT 0x02 /* Shift */
#define USBKEY_LALT   0x04 /* Alt */
#define USBKEY_LMETA  0x08 /* Meta */
#define USBKEY_RCTRL  0x10 /* contRol*/
#define USBKEY_RSHIFT 0x20 /* shifT */
#define USBKEY_RALT   0x40 /* alT */
#define USBKEY_RMETA  0x80 /* metA */
#define USBKEY_A 0x04 /* a A */
#define USBKEY_B 0x05 /* b B */
#define USBKEY_C 0x06 /* c C */
#define USBKEY_D 0x07 /* d D */
#define USBKEY_E 0x08 /* e E */
#define USBKEY_F 0x09 /* f F */
#define USBKEY_G 0x0a /* g G */
#define USBKEY_H 0x0b /* h H */
#define USBKEY_I 0x0c /* i I */
#define USBKEY_J 0x0d /* j J */
#define USBKEY_K 0x0e /* k K */
#define USBKEY_L 0x0f /* l L */
#define USBKEY_M 0x10 /* m M */
#define USBKEY_N 0x11 /* n N */
#define USBKEY_O 0x12 /* o O */
#define USBKEY_P 0x13 /* p P */
#define USBKEY_Q 0x14 /* q Q */
#define USBKEY_R 0x15 /* r R */
#define USBKEY_S 0x16 /* s S */
#define USBKEY_T 0x17 /* t T */
#define USBKEY_U 0x18 /* u U */
#define USBKEY_V 0x19 /* v V */
#define USBKEY_W 0x1a /* w W */
#define USBKEY_X 0x1b /* x X */
#define USBKEY_Y 0x1c /* y Y */
#define USBKEY_Z 0x1d /* z Z */
#define USBKEY_1 0x1e /* 1 ! */
#define USBKEY_2 0x1f /* 2 @ */
#define USBKEY_3 0x20 /* 3 # */
#define USBKEY_4 0x21 /* 4 $ */
#define USBKEY_5 0x22 /* 5 % */
#define USBKEY_6 0x23 /* 6 ^ */
#define USBKEY_7 0x24 /* 7 & */
#define USBKEY_8 0x25 /* 8 * */
#define USBKEY_9 0x26 /* 9 ( */
#define USBKEY_0 0x27 /* 0 ) */
#define USBKEY_ENTER 0x28 /* Enter */
#define USBKEY_ESC 0x29 /* Esc */
#define USBKEY_BACKSPACE 0x2a /* Backspace */
#define USBKEY_TAB 0x2b /* Tab */
#define USBKEY_SPACE 0x2c /* Spacebar */
#define USBKEY_MINUS 0x2d /* -_ */
#define USBKEY_EQUAL 0x2e /* =+ */
#define USBKEY_LEFTBRACE 0x2f /* [{ */
#define USBKEY_RIGHTBRACE 0x30 /*  } */
#define USBKEY_BACKSLASH 0x31 /* \| */
#define USBKEY_SEMICOLON 0x33 /* ;: */
#define USBKEY_APOSTROPHE 0x34 /* '" */
#define USBKEY_GRAVE 0x35 /* `~ */
#define USBKEY_COMMA 0x36 /* ,< */
#define USBKEY_DOT 0x37 /* .> */
#define USBKEY_SLASH 0x38 /* /? */
#define USBKEY_CAPSLOCK 0x39 /*  CapsLock */
#define USBKEY_F1 0x3a /* F1 */
#define USBKEY_F2 0x3b /* F2 */
#define USBKEY_F3 0x3c /* F3 */
#define USBKEY_F4 0x3d /* F4 */
#define USBKEY_F5 0x3e /* F5 */
#define USBKEY_F6 0x3f /* F6 */
#define USBKEY_F7 0x40 /* F7 */
#define USBKEY_F8 0x41 /* F8 */
#define USBKEY_F9 0x42 /* F9 */
#define USBKEY_F10 0x43 /* F10 */
#define USBKEY_F11 0x44 /* F11 */
#define USBKEY_F12 0x45 /* F12 */
#define USBKEY_SYSRQ 0x46 /* PrintScreen */
#define USBKEY_SCROLLLOCK 0x47 /* ScrollLock */
#define USBKEY_PAUSE 0x48 /* Pause */
#define USBKEY_INSERT 0x49 /* Insert */
#define USBKEY_HOME 0x4a /* Home */
#define USBKEY_PAGEUP 0x4b /* PageUp */
#define USBKEY_DELETE 0x4c /* Delete */
#define USBKEY_END 0x4d /* End */
#define USBKEY_PAGEDOWN 0x4e /* PageDown */
#define USBKEY_RIGHT 0x4f /* Right */
#define USBKEY_LEFT 0x50 /* Left */
#define USBKEY_DOWN 0x51 /* Down */
#define USBKEY_UP 0x52 /* Up */
#define USBKEY_NUMLOCK 0x53 /* NumLock */
#define USBKEY_KPSLASH 0x54 /* / */
#define USBKEY_KPASTERISK 0x55 /* * */
#define USBKEY_KPMINUS 0x56 /* - */
#define USBKEY_KPPLUS 0x57 /* + */
#define USBKEY_KPENTER 0x58 /* Enter */
#define USBKEY_KP1 0x59 /* 1 End */
#define USBKEY_KP2 0x5a /* 2 Down */
#define USBKEY_KP3 0x5b /* 3 PageDn */
#define USBKEY_KP4 0x5c /* 4 Left */
#define USBKEY_KP5 0x5d /* 5 */
#define USBKEY_KP6 0x5e /* 6 Right */
#define USBKEY_KP7 0x5f /* 7 Home */
#define USBKEY_KP8 0x60 /* 8 Up */
#define USBKEY_KP9 0x61 /* 9 PageUp */
#define USBKEY_KP0 0x62 /* 0 Insert */
#define USBKEY_KPDOT 0x63 /* . Delete */
#define USBKEY_KPEQUAL 0x67 /* = */

char *KT[] = {
  "",
  "",
  "",
  "",
  "aA", "bB", "cC", "dD", "eE", "fF", "gG", "hH", "iI", "jJ", "kK", "lL", "mM",
  "nN", "oO", "pP", "qQ", "rR", "sS", "tT", "uU", "vV", "wW", "xX", "yY", "zZ",
  "1!", "2@", "3#", "4$", "5%", "6^", "7&", "8*", "9(", "0)",
  "Enter", "Escape", "Backspace", "Tab",
  " ", "-_", "=+", "[{", "]}", "\\|", "", ";:", "'\"", "`~", ",<", ".>", "/?",
  "CapsLock",
  "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12",
  "Print", "ScrollLock", "Pause",
  "Insert", "Home", "PageUp", "Delete", "End", "PageDown",
  "Right", "Left", "Down", "Up",
  "NumLock",
  "/", "*", "-", "+",
  "KPEnter",
  "1", "2", "3", "4", "5", "6", "7", "8", "9", "0",
  ".",
  "",  "",  "",
  "="
};

int scan_xdigit(char c) {
  if ((c == 'A') || (c == 'a')) return 0x0A;
  if ((c == 'B') || (c == 'b')) return 0x0B;
  if ((c == 'C') || (c == 'c')) return 0x0C;
  if ((c == 'D') || (c == 'd')) return 0x0D;
  if ((c == 'E') || (c == 'e')) return 0x0E;
  if ((c == 'F') || (c == 'f')) return 0x0F;
  if (c == '0') return 0x0;
  if (c == '1') return 0x01;
  if (c == '2') return 0x02;
  if (c == '3') return 0x03;
  if (c == '4') return 0x04;
  if (c == '5') return 0x05;
  if (c == '6') return 0x06;
  if (c == '7') return 0x07;
  if (c == '8') return 0x08;
  if (c == '9') return 0x09;
  perror("scan_xdigit");
  exit(1);
  return 0;
}

char scanx(char *src) {
  char b = scan_xdigit(src[0]) << 4;
  b += scan_xdigit(src[1]);
  return b;
}

void sprintx(char *dst, u8 *src, int n)  {
  const char xx[]= "0123456789ABCDEF";
  for (; n > 0; --n) {
    unsigned char c = *src++;
    *dst++ = xx[c >> 4];
    *dst++ = xx[c & 0x0f];
  }
}

int EFAIL(char *msg) {
  if (errno) perror(msg);
  write(1, "\nFAIL\n", 6);
  write(1, msg, strlen(msg));
  write(1, "\n", 1);
  exit(1);
}

int main(int argc, char *argv[]) {
  u64 E = 0;
  time_t T = time(0);
  time_t T0 = T;
  u64 TE = E;
  char L[4096];
  mset(L, 0, 4096);
  int AETHER = htons(ETH_P_ALL);
  int R = snprintf(L, 80, "/*\n *\n * hai!\n * gmp %s cairo %s\n%s",
   gmp_version, cairo_version_string(), ctime(&T0));
  write(1, L, R);
  R = setuid(0);
  if (R) EFAIL("setuid run with sudo");
  R = setgid(0);
  if (R) EFAIL("setgid run with sudo");
  R = setvbuf(stdin, NULL, _IONBF, 0);
  if (R) EFAIL("setvbuf stdin _IONBF");
  R = setvbuf(stdout, NULL, _IONBF, 0);
  if (R) EFAIL("setvbuf stdout _IONBF");
  R = setvbuf(stderr, NULL, _IONBF, 0);
  if (R) EFAIL("setvbuf stderr _IONBF");
  struct signalfd_siginfo SNFO;
  sigset_t mask;
  sigemptyset(&mask);
  sigfillset(&mask);
  R = sigprocmask(SIG_BLOCK, &mask, NULL);
  if (R) EFAIL("sigprocmask");
  int SD = signalfd(-1, &mask, SFD_NONBLOCK | SFD_CLOEXEC);
  int MD = memfd_create("pixmap-framebuffer", MFD_CLOEXEC);
  int PD = epoll_create1(EPOLL_CLOEXEC);
  int ED = eventfd(26, EFD_NONBLOCK | EFD_CLOEXEC);
  int TD = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
  int ND = socket(AF_PACKET, SOCK_RAW | SOCK_NONBLOCK | SOCK_CLOEXEC, AETHER);
  int ID = inotify_init1(IN_NONBLOCK | IN_CLOEXEC);
  if ((6*6+6) > (SD + MD + PD + ED + TD + ND + ID)) EFAIL("6*6+6=42 PANICAN");
  int WD = inotify_add_watch(ID, "/dev", IN_CREATE);
  if (WD == -1) EFAIL("inotify_add_watch /dev IN_CREATE");
  struct inotify_event INEV;
  R = ftruncate(MD, 4205260800);
  if (R == -1) EFAIL("ftruncate 4205260800");
  m = mmap(NULL, 4205260800, PROT_READ | PROT_WRITE, MAP_SHARED, MD, 0);
  if (!m) EFAIL("mmap 4205260800");
  R = mlock(m, 4205260800);
  if (R == -1) EFAIL("mlock 4205260800");
  mset(m, 'K', 4205260800);
  write(1, "memgood\n", 8);
  /*
  pw_init(&argc, &argv);
  char *pw_hdr_ver = pw_get_headers_version();
  const char *pw_lib_ver = pw_get_library_version();
  if (strsz(pw_hdr_ver) != strsz(pw_lib_ver)) return 42;
  if (mcmp(pw_hdr_ver, pw_lib_ver, strsz(pw_lib_ver))) return 666;
  printf("Pipewire %s\n", pw_hdr_ver); */
  /* void mp_set_memory_functions
      void *(*alloc_func_ptr) (size_t),
      void *(*realloc_func_ptr) (void *, size_t, size_t),
      void (*free_func_ptr) (void *, size_t))
  */
  mpz_t a, b, c, d, r, r2;
  mpz_init_set_ui(b, 0);
  mpz_init_set_ui(c, 0);
  mpz_init_set_ui(r, 0);
  mpz_init_set_str(a, "4205260800", 10);
  mpz_init_set_str(d, "4205260799", 10);
  mpz_init_set_str(r2, "18446744073709551616", 10);
  mpz_mod(r, d, r2);
  /*gmp_printf("%Zd\n", r);*/
  mpz_t e;
  mpz_init(e);
  mpz_fac_ui(e, 26);
  /*gmp_printf("26!\n%Zd\n", e);*/
  mpz_t f;
  mpz_init(f);
  mpz_fac_ui(f, 126);
  /*gmp_printf("126!\n%Zd\n", f);*/
  mpz_t g;
  mpz_init(g);
  mpz_fac_ui(g, 209);
  /*gmp_printf("209!\n%Zd\n", g);*/
  mpz_clear(a);
  mpz_clear(b);
  mpz_clear(c);
  mpz_clear(d);
  mpz_clear(r);
  mpz_clear(r2);
  mpz_clear(e);
  mpz_clear(f);
  mpz_clear(g);
  T = time(0);
  R = snprintf(L, 80, "* hid scan %s", ctime(&T));
  write(1, L, R);
  int HiD[26];
  char HiD_type[26];
  for (int i = 0; i < 26; i++) {
    HiD[i] = 0;
    HiD_type[i] = 0;
    struct hidraw_report_descriptor rpt;
    struct hidraw_devinfo hinfo;
    mset(&rpt, 0, sizeof(rpt));
    mset(&rpt, 0, sizeof(hinfo));
    char devname[16];
    snprintf(devname, 16, "/dev/hidraw%d", i);
    R = open(devname, O_RDONLY | O_NONBLOCK);
    if (R < 0) continue;
    HiD[i] = R;
    R = ioctl(HiD[i], HIDIOCGRDESCSIZE, &rpt.size);
    if ((R < 0) || (rpt.size < 4)) EFAIL("HIDIOCGRDESCSIZE");
    R = ioctl(HiD[i], HIDIOCGRDESC, &rpt);
    if (R < 0) EFAIL("HIDIOCGRDESC");
    R = ioctl(HiD[i], HIDIOCGRAWNAME(256), &L);
    if (R < 0) EFAIL("HIDIOCGRAWNAME");
    /*printf("%s\n", L);*/
    R = ioctl(HiD[i], HIDIOCGRAWPHYS(256), &L);
    if (R < 0) EFAIL("HIDIOCGRAWPHYS");
    /*printf("%s\n", L);*/
    R = ioctl(HiD[i], HIDIOCGRAWINFO, &hinfo);
    if (R < 0) EFAIL("HIDIOCGRAWINFO");
    sprintx(L + 2, (u8 *)&hinfo.vendor, 1);
    sprintx(L + 0, (u8 *)&hinfo.vendor + 1, 1);
    L[4] = ':';
    sprintx(L + 7, (u8 *)&hinfo.product, 1);
    sprintx(L + 5, (u8 *)&hinfo.product + 1, 1);
    L[9] = '\n';
    write(1, L, 10);
    u8 *s = rpt.value;
    sprintx(L, s, rpt.size);
    write(1, L, rpt.size * 2);
    write(1, "\n", 1);
    if (rpt.value[0] + s[1] + s[2] == 15) {
      if (s[3] == 2) HiD_type[i] = 'm';
      if (s[3] == 6) HiD_type[i] = 'k';
    }
  }
  T = time(0);
  R = snprintf(L, 80, "* loading %s", ctime(&T));
  write(1, L, R);
  for (int i = 0; i < 676; i++) {
    char c1 = 96 + 1 + (i / 26);
    char c2 = 96 + 1 + (i % 26);
    char filename[256];
    char *map_dir = getenv("OVERRIDE_PROGRAM_PIXMAP_BASEDIR");
    if (map_dir) printf("using %s for dat pix map\n", map_dir);
    if (!map_dir) map_dir = "/map";
    snprintf(filename, 256, "%s/%c/%c/%s%s.png", map_dir, c1, c2,
     nato[c1 - 97], nato[c2 - 97]);
    cairo_surface_t *cst = cairo_image_surface_create_from_png(filename);
    cairo_status_t cairo_errno = cairo_surface_status(cst);
    if (cairo_errno) {
      printf("PC_LOAD_LETTER\n");
      break;
    }
    if (i % 26 == 0) {
      T = time(0);
      R = snprintf(L, 80, "* loading %3d %s %s", i, filename, ctime(&T));
      write(1, L, R);
    }
    if ((cairo_image_surface_get_width(cst) != 1920)
     || (cairo_image_surface_get_height(cst) != 1080)
     || (cairo_image_surface_get_stride(cst) != 1920 * 4)
     || (cairo_image_surface_get_format(cst) != CAIRO_FORMAT_RGB24)) {
      EFAIL("13 cairo bad map png format 1920x1080 rgb24 bro");
    }
    u8 *dat = cairo_image_surface_get_data(cst);
    for (int y = 0; y < 1080; y++) {
      for (int x = 0; x < 1920; x++) {
        u32 pxy = 0;
        pxy += x * 3;
        pxy += y * 1920 * 26 * 3;
        if (i / 26) pxy += 1920 * 1080 * 3 * 26 * (i / 26);
        if (i % 26) pxy += 1920 * 3 * (i % 26);
        m[pxy] = dat[(y * 1920 * 4) + (x * 4)];
        m[pxy + 1] = dat[(y * 1920 * 4) + (x * 4) + 1];
        m[pxy + 2] = dat[(y * 1920 * 4) + (x * 4) + 2];
      }
    }
    cairo_surface_destroy(cst);
  }
  u32 fb_id = 0;
  u32 fb2_id = 0;
  int DD = open("/dev/dri/card0", O_RDWR);
  if (DD < 0) EFAIL("drmOpen failed");
  drmVersionPtr DV = drmGetVersion(DD);
  printf("opened device `%s` on driver `%s` (version %d.%d.%d at %s)\n",
   DV->desc, DV->name, DV->version_major, DV->version_minor,
   DV->version_patchlevel, DV->date);
  drmFreeVersion(DV);
  drmModeRes *DRES = drmModeGetResources(DD);
  if (DRES == NULL) EFAIL("drmModeGetResources");
  drmModeConnector *DCON = NULL;
  for (int i = 0; i < DRES->count_connectors; ++i) {
    DCON = drmModeGetConnector(DD, DRES->connectors[i]);
    if (DCON != NULL) {
      printf("connector %d found\n", DCON->connector_id);
      if (DCON->connection == DRM_MODE_CONNECTED
       && DCON->count_modes > 0) {
        break;
      }
      drmModeFreeConnector(DCON);
    } else { EFAIL("get a null connector pointer"); }
    if (i == DRES->count_connectors) EFAIL("No active connector found");
  }
  drmModeModeInfo DM = DCON->modes[0];
  W = DM.hdisplay;
  H = DM.vdisplay;
  T = time(0);
  printf("%d x %d\n%s", W, H, ctime(&T));
  drmModeEncoder *DENC = NULL;
  for (int i = 0; i < DRES->count_encoders; ++i) {
    DENC = drmModeGetEncoder(DD, DRES->encoders[i]);
    if (DENC != NULL) {
      printf("encoder %d found\n", DENC->encoder_id);
      if (DENC->encoder_id == DCON->encoder_id) {
        break;
      }
      drmModeFreeEncoder(DENC);
    } else {
      EFAIL("get a null encoder pointer");
    }
  }
  struct drm_mode_create_dumb cd_arg;
  mset(&cd_arg, 0, sizeof(cd_arg));
  cd_arg.bpp = 32;
  cd_arg.width = W;
  cd_arg.height = H;
  R = ioctl(DD, DRM_IOCTL_MODE_CREATE_DUMB, &cd_arg);
  if (R) EFAIL("DRM_IOCTL_MODE_CREATE_DUMB");
  struct drm_mode_map_dumb md_arg;
  mset(&md_arg, 0, sizeof(md_arg));
  md_arg.handle = cd_arg.handle;
  R = ioctl(DD, DRM_IOCTL_MODE_MAP_DUMB, &md_arg);
  if (R) EFAIL("DRM_IOCTL_MODE_MAP_DUMB");
  u8 *pixmap1 = mmap(0, cd_arg.size, PROT_READ | PROT_WRITE, MAP_SHARED, DD,
                       md_arg.offset);
  mset(pixmap1, 0xFF, cd_arg.pitch * H);
  R = drmModeAddFB(DD, W, H, 24, 32, cd_arg.pitch, cd_arg.handle, &fb_id);
  if (R) EFAIL("drmModeAddFB");

  drmModeCrtcPtr crtc;
  mset(&crtc, 0, sizeof(crtc));
  crtc = drmModeGetCrtc(DD, DENC->crtc_id);
  if (!crtc) EFAIL("drmModeGetCrtc");
  R = drmModeSetCrtc(DD, DENC->crtc_id, fb_id, 0, 0, &DCON->connector_id, 1,
   &DM);
  if (R) EFAIL("drmModeSetCrtc failed");

  struct drm_mode_create_dumb cd2_arg;
  mset(&cd2_arg, 0, sizeof(cd2_arg));
  cd2_arg.bpp = 32;
  cd2_arg.width = W;
  cd2_arg.height = H;
  R = ioctl(DD, DRM_IOCTL_MODE_CREATE_DUMB, &cd2_arg);
  if (R) EFAIL("DRM_IOCTL_MODE_CREATE_DUMB");
  struct drm_mode_map_dumb md2_arg;
  mset(&md2_arg, 0, sizeof(md2_arg));
  md2_arg.handle = cd2_arg.handle;
  R = ioctl(DD, DRM_IOCTL_MODE_MAP_DUMB, &md2_arg);
  if (R) EFAIL("DRM_IOCTL_MODE_MAP_DUMB");
  u8 *pixmap2 = mmap(0, cd2_arg.size, PROT_READ | PROT_WRITE, MAP_SHARED, DD,
                          md2_arg.offset);
  mset(pixmap2, 0xFF, cd2_arg.pitch * H);
  R = drmModeAddFB(DD, W, H, 24, 32, cd2_arg.pitch, cd2_arg.handle, &fb2_id);
  if (R) EFAIL("drmModeAddFB failed");

  struct dctx dctx;
  mset(&dctx, 0, sizeof dctx);
  dctx.pixmap1 = pixmap1;
  dctx.pixmap2 = pixmap2;
  dctx.pixmap_sz = H * cd2_arg.pitch;
  dctx.fb_id[0] = fb_id;
  dctx.fb_id[1] = fb2_id;
  dctx.current_fb_id = fb_id;
  dctx.crtid = DENC->crtc_id;
  dctx.swap_count = 0;
  dctx.complete = 0;
  int DE = DRM_MODE_PAGE_FLIP_EVENT;
  R = drmModePageFlip(DD, DENC->crtc_id, fb2_id, DE, &dctx);
  if (R) EFAIL("failed to page flip");

  drmEventContext evctx;
  mset(&evctx, 0, sizeof evctx);
  evctx.version = DRM_EVENT_CONTEXT_VERSION;
  evctx.vblank_handler = vblank;
  evctx.page_flip_handler = pageflip;

  struct epoll_event ev;
  ev.events = EPOLLIN;

  ev.data.fd = SD;
  R = epoll_ctl(PD, EPOLL_CTL_ADD, ev.data.fd, &ev);
  if (R) EFAIL("epoll_ctl");

  ev.data.fd = ED;
  R = epoll_ctl(PD, EPOLL_CTL_ADD, ev.data.fd, &ev);
  if (R) EFAIL("epoll_ctl");

  ev.data.fd = TD;
  R = epoll_ctl(PD, EPOLL_CTL_ADD, ev.data.fd, &ev);
  if (R) EFAIL("epoll_ctl");

  ev.data.fd = ND;
  R = epoll_ctl(PD, EPOLL_CTL_ADD, ev.data.fd, &ev);
  if (R) EFAIL("epoll_ctl");
  
  ev.data.fd = ID;
  R = epoll_ctl(PD, EPOLL_CTL_ADD, ev.data.fd, &ev);
  if (R) EFAIL("epoll_ctl");

  ev.data.fd = DD;
  R = epoll_ctl(PD, EPOLL_CTL_ADD, ev.data.fd, &ev);
  if (R) EFAIL("epoll_ctl");

  for (int i = 0; i < 26; i++) {
    if (HiD[i] > 0) {
      ev.data.fd = HiD[i];
      R = epoll_ctl(PD, EPOLL_CTL_ADD, ev.data.fd, &ev);
      if (R) EFAIL("epoll_ctl");
    }
  }
  for (;;) {
    R = epoll_wait(PD, &ev, 1, 2601);
    int fd = ev.data.fd;
    if (R < 0) EFAIL("epoll_wait");
    if (R == 0) continue;
    if ((ev.events & EPOLLERR) || (ev.events & EPOLLHUP)) {
      for (int h = 0; h < 26; h++) {
        if (fd == HiD[h]) {
          R = close(fd);
          if (R) EFAIL("close");
          HiD[h] = 0;
        }
      }
      if (fd <= DD) {
        if (ev.events & EPOLLHUP) EFAIL("EPOLLHUP");
        if (ev.events & EPOLLERR) EFAIL("EPOLLERR");
      }
    }
    if (!(ev.events & EPOLLIN)) continue;
    if (fd == SD) {
      for (;;) {
        R = read(SD, &SNFO, sizeof(SNFO));
        if (R > 0) continue;
        if ((R == -1) && (errno != EAGAIN)) EFAIL("read SD");
        break;
      }
    }
    if (fd == ED) {
      read(ED, &E, 8);
    }
    if (fd == TD) {
      read(TD, &TE, 8);
    }
    if (fd == ND) {
      for (;;) {
        R = read(ND, L, sizeof(L));
        if (R > 0) continue;
        if ((R == -1) && (errno != EAGAIN)) EFAIL("read ND");
        break;
      }
    }
    if (fd == ID) {
      for (;;) {
        R = read(ID, &INEV, sizeof(INEV));
        if ((R == -1) && (errno != EAGAIN)) EFAIL("read ID");
        if (R > 0) {
          if ((INEV.wd == WD) && (INEV.mask & IN_CREATE)) {
            if (INEV.len > 6 && INEV.len < 9) {
              struct hidraw_report_descriptor rpt;
              struct hidraw_devinfo hinfo;
              char devname[16];
              snprintf(devname, 16, "/dev/%s", INEV.name);
              if (!(mcmp(devname + 6, "hidraw", 6))) continue;
              int hfd = open(devname, O_RDONLY | O_NONBLOCK);
              if (hfd > 0) {
                for (int h = 0; h < 26; h++) {
                  if (!HiD[h]) HiD[h] = hfd;
                  R = ioctl(HiD[h], HIDIOCGRDESCSIZE, &rpt.size);
                  if ((R < 0) || (rpt.size < 4)) EFAIL("HIDIOCGRDESCSIZE");
                  R = ioctl(HiD[h], HIDIOCGRDESC, &rpt);
                  if (R < 0) EFAIL("HIDIOCGRDESC");
                  R = ioctl(HiD[h], HIDIOCGRAWNAME(256), &L);
                  if (R < 0) EFAIL("HIDIOCGRAWNAME");
                  printf("%s\n", L);
                  R = ioctl(HiD[h], HIDIOCGRAWPHYS(256), &L);
                  if (R < 0) EFAIL("HIDIOCGRAWPHYS");
                  R = ioctl(HiD[h], HIDIOCGRAWINFO, &hinfo);
                  if (R < 0) EFAIL("HIDIOCGRAWINFO");
                  sprintx(L + 2, (u8 *)&hinfo.vendor, 1);
                  sprintx(L + 0, (u8 *)&hinfo.vendor + 1, 1);
                  L[4] = ':';
                  sprintx(L + 7, (u8 *)&hinfo.product, 1);
                  sprintx(L + 5, (u8 *)&hinfo.product + 1, 1);
                  L[9] = '\n';
                  write(1, L, 10);
                  u8 *s = rpt.value;
                  sprintx(L, s, rpt.size);
                  write(1, L, rpt.size * 2);
                  write(1, "\n", 1);
                  if (rpt.value[0] + s[1] + s[2] == 15) {
                    if (s[3] == 2) HiD_type[h] = 'm';
                    if (s[3] == 6) HiD_type[h] = 'k';
                    ev.data.fd = HiD[h];
                    R = epoll_ctl(PD, EPOLL_CTL_ADD, ev.data.fd, &ev);
                    if (R) EFAIL("epoll_ctl");
                  }
                }
              }
            }
            continue;
          }
          break;
        }
      }
    }
    for (int h = 0; h < 26; h++) {
      if (fd != HiD[h]) continue;
      if (HiD_type[h] == 'k') {
        R = read(fd, &K, 8);
        if (R != 8) EFAIL("read keyboard");
        for (int k = 0; k < 6; k++) {
          int kd = K[2 + k];
          if (kd == 0) continue;
          if (((kd >= USBKEY_A && kd <= USBKEY_0))
           || (kd >= USBKEY_SPACE && kd <= USBKEY_SLASH)
           || (kd >= USBKEY_KPSLASH && kd <= USBKEY_KPPLUS)
           || (kd >= USBKEY_KP1 && kd <= USBKEY_KPEQUAL)) {
           char keychar = KT[kd][0];
           if (keychar == 0) continue;
           if (((K[0] & 0b00000010) || (K[0] & 0b00100000))
            && (KT[kd][1] != 0)) { keychar = KT[kd][1]; }
          }
          if ((kd >= USBKEY_ENTER && kd <= USBKEY_TAB)
           || (kd >= USBKEY_CAPSLOCK && kd <= USBKEY_NUMLOCK)
           || (kd == USBKEY_KPENTER)) {
            int mov = 1;
            if ((K[0] & 0b00000010) || (K[0] & 0b00100000)) { mov = 260; }
            if (kd == USBKEY_F4) dctx.complete = 1;
            if (((K[0] & 0b00000001) || (K[0] & 0b00010000))
             && ((K[0] & 0b00000100) || (K[0] & 0b01000000))) { }
            if (kd == USBKEY_UP) { if ((Y - mov) >= 0) Y -= mov; }
            if (kd == USBKEY_LEFT) { if ((X - mov) >= 0) X -= mov; }
            if (kd == USBKEY_DOWN) { if ((Y + mov ) <= (28080 - H)) Y += mov; }
            if (kd == USBKEY_RIGHT) { if ((X + mov) <= (49920 - W)) X += mov; }
          }
        }
      }
      if (HiD_type[h] == 'm') {
        i8 M[4] = {0,0,0,0};
        R = read(fd, &M, 4);
        if (R != 4) EFAIL("read mouse");
        CX += M[1];
        CY += M[2];
      }
    }
    if (fd == DD) {
      R = drmHandleEvent(DD, &evctx);
    }
    if (dctx.complete == 2) break;
  }
  R = drmModeSetCrtc(DD, crtc->crtc_id, crtc->buffer_id,
   crtc->x, crtc->y, &DCON->connector_id, 1, &crtc->mode);
  if (R) EFAIL("drmModeSetCrtc() restore original crtc failed");
  drmModeRmFB(DD, fb2_id);
  drmModeRmFB(DD, fb_id);
  drmModeFreeEncoder(DENC);
  drmModeFreeConnector(DCON);
  drmModeFreeResources(DRES);
  close(DD);
  T = time(0);
  printf("done\n%s\n", ctime(&T));
  write(1, "OKrad\n", 6);
  exit(0);
}

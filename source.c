#include "header.h"

u8 *m = 0;
int X = 49920 - 49920;
int Y = 28080 - 28080;
int W = 0;
int H = 0;
u8 *P = 0;

void draw(void) {
  for (int y = 0; y < H; y++) {
    for (int x = 0; x < W; x++) {
      int pxy = ((y + Y) * (1920 * 26 * 3)) + (X * 3) + (x * 3);
      P[(y * W * 4) + (x * 4) + 0] = m[pxy + 0];
      P[(y * W * 4) + (x * 4) + 1] = m[pxy + 1];
      P[(y * W * 4) + (x * 4) + 2] = m[pxy + 2];
    }
  }
}

struct dctx {
  u32 fb_id[2];
  u32 current_fb_id;
  int crtid;
  struct timeval start;
  u64 swap_count;
  u8 *pixmap1;
  u8 *pixmap2;
  int pixmap_sz;
};

void pageflip(int fd, u32 frame, u32 sec, u32 usec, void *data) {
  struct dctx *context;
  unsigned int new_fb_id;
  struct timeval end;
  double t;
  context = (struct dctx *)data;
  if (context->current_fb_id == context->fb_id[0]) {
    new_fb_id = context->fb_id[1];
    mset(context->pixmap2, 255 - context->swap_count, context->pixmap_sz);
    P = context->pixmap2;
  } else {
    new_fb_id = context->fb_id[0];
    mset(context->pixmap1, 255 - (context->swap_count * 2), context->pixmap_sz);
    P = context->pixmap1;
  }
  draw();
  drmModePageFlip(fd, context->crtid, new_fb_id, DRM_MODE_PAGE_FLIP_EVENT,
   context);
  context->current_fb_id = new_fb_id;
  context->swap_count++;
  if ((context->swap_count % 60) == 0) {
    gettimeofday(&end, NULL);
    t = end.tv_sec + end.tv_usec * 1e-6
     - (context->start.tv_sec + context->start.tv_usec * 1e-6);
    printf("freq: %.02fHz\n", 60 / t);
    context->swap_count = 0;
    context->start = end;
  }
}
/*
    if (mouse) {
      char b[4];
      ret = read(id, b, 4);
      if (ret != 4) {
        perror("read mouse problem");
        return 1;
      }
      char out[8];
      sprintx(out, b, 4);
      write(1, out, 8);
      write(1, "\n", 1);
      printf("%d %d %d %d\n", scanx(out), scanx(out + 2),
       scanx(out + 4), scanx(out + 6));
    }

    if (keyboard) {
      char b[8];
      ret = read(id, b, 8);
      if (ret != 8) {
        perror("read keyboard problem");
        return 1;
      }
      char out[16];
      sprintx(out, b, 8);
*//*
      write(1, out, 16);
      write(1, "\n", 1);
      printf("%d %d %d %d %d %d %d %d\n",
       scanx(out + 0), scanx(out + 2),
       scanx(out + 4), scanx(out + 6),
       scanx(out + 8), scanx(out + 10),
       scanx(out + 12), scanx(out + 14));
      printmod(b);
*//*
      if ((b[0]) && (b[2])) {
	if (controling(b[0])) printf("Control-");
	if (alting(b[0])) printf("Alt-");
	if (metaing(b[0])) printf("Meta-");
      }
      if (b[2]) {
        if (textkey(b[2])) {
          if (shifting(b[0])) {
            printf("%c\n", usbkeyboardlabeltable[b[2]][1]);
          } else {
            printf("%c\n", usbkeyboardlabeltable[b[2]][0]);
          }
        } else {
          printf("%s\n", usbkeyboardlabeltable[b[2]]);
        }
      }
    }
*/

char *usbkeyboardlabeltable[] = {
  "", "", "", "",
  "aA", "bB", "cC", "dD", "eE", "fF", "gG", "hH", "iI", "jJ", "kK", "lL", "mM",
  "nN", "oO", "pP", "qQ", "rR", "sS", "tT", "uU", "vV", "wW", "xX", "yY", "zZ",
  "1!","2@","3#","4$","5%","6^","7&","8*","9(","0)",
  "Enter", "Escape", "Backspace", "Tab",
  " ",
  "-_", "=+", "[{", "]}",	"\\|", "", ";:", "'\"", "`~", ",<", ".>", "/?",
  "CapsLock",
  "F1","F2","F3","F4","F5","F6","F7","F8","F9","F10","F11","F12",
  "Print", "ScrollLock", "Pause",
  "Insert", "Home", "PageUp", "Delete", "End", "PageDown",
  "Right", "Left", "Down", "Up",
  "NumLock", "/", "*", "-", "+", "Enter",
  "1", "2", "3", "4", "5", "6", "7", "8", "9", "0",
  ".", "", "Menu"
};

int textkey(char k) {
  if (!k) return 0;
  if (k > 3) {
    if (k < 40) return 1;
    if ((k > 44) && (k < 57)) return 1;
  }
  return 0; 
}

void printmod(char *k) {
  int p = 0;
  static char out[64];
  mset(out, 0, sizeof(out));
  if (k[1] != 0) {
    printf("keyboard scan error");
    exit(1);
  }
  if ((k[0] & 0b00000001) || (k[0] & 0b00010000)) {
    p += snprintf(out + p, sizeof(out) - p, "Control");
  }
  if ((k[0] & 0b00000010) || (k[0] & 0b00100000)) {
    p += snprintf(out + p, sizeof(out) - p, "Shift");
  }
  if ((k[0] & 0b00000100) || (k[0] & 0b01000000)) {
    p += snprintf(out + p, sizeof(out) - p, "Alt");
  }
  if ((k[0] & 0b00001000) || (k[0] & 0b10000000)) {
    p += snprintf(out + p, sizeof(out) - p, "Meta");
  }
  if (p) printf("%*s\n", p, out);
}

int controling(char k) {
  if ((k & 0b00000001) || (k & 0b00010000)) return 1;
  return 0;
}

int shifting(char k) {
  if ((k & 0b00000010) || (k & 0b00100000)) return 1;
  return 0;
}

int alting(char k) {
  if ((k & 0b00000100) || (k & 0b01000000)) return 1;
  return 0;
}

int metaing(char k) {
  if ((k & 0b00001000) || (k & 0b10000000)) return 1;
  return 0;
}

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
  perror(msg);
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
  int R = snprintf(L, 80, "program 2601 begins %sgmp %s cairo %s\n", ctime(&T0),
   gmp_version, cairo_version_string());
  write(1, L, R);
  R = setuid(0);
  if (R) EFAIL("1.0 setuid run with sudo");
  R = setgid(0);
  if (R) EFAIL("1.5 setgid run with sudo");
  R = setvbuf(stdin, NULL, _IONBF, 0);
  if (R) EFAIL("2 setvbuf stdin _IONBF");
  R = setvbuf(stdout, NULL, _IONBF, 0);
  if (R) EFAIL("3 setvbuf stdout _IONBF");
  R = setvbuf(stderr, NULL, _IONBF, 0);
  if (R) EFAIL("4 setvbuf stderr _IONBF");
  sigset_t mask;
  sigemptyset(&mask);
  sigfillset(&mask);
  R = sigprocmask(SIG_BLOCK, &mask, NULL);
  if (R) EFAIL("5 sigprocmask");
  int SD = signalfd(-1, &mask, SFD_NONBLOCK | SFD_CLOEXEC);
  int MD = memfd_create("pixmap-framebuffer", MFD_CLOEXEC);
  int PD = epoll_create1(EPOLL_CLOEXEC);
  int ED = eventfd(26, EFD_NONBLOCK | EFD_CLOEXEC);
  int TD = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
  int ND = socket(AF_PACKET, SOCK_RAW | SOCK_NONBLOCK | SOCK_CLOEXEC, AETHER);
  int ID = inotify_init1(IN_NONBLOCK | IN_CLOEXEC);
  if ((6*6+6) != (SD + MD + PD + ED + TD + ND + ID)) EFAIL("6*6+6=42 PANICAN");
  int WD = inotify_add_watch(ID, "/dev", IN_CREATE);
  if (WD == -1) EFAIL("6 inotify_add_watch /dev IN_CREATE");
  R = ftruncate(MD, 4205260800);
  if (R == -1) EFAIL("7 ftruncate 4205260800");
  m = mmap(NULL, 4205260800, PROT_READ | PROT_WRITE, MAP_SHARED, MD, 0);
  if (!m) EFAIL("8 mmap 4205260800");
  R = mlock(m, 4205260800);
  if (R == -1) EFAIL("9 mlock 4205260800");
  mset(m, 'K', 4205260800);
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
  int HiD[26];
  char HiD_type[26];
  for (int i = 0; i < 26; i++) {
    HiD[i] = 0;
    HiD_type[i] = 0;
  }
  for (int i = 0; i < 26; i++) {
    struct hidraw_report_descriptor rpt;
    mset(&rpt, 0, sizeof(rpt));
    char devname[16];
    snprintf(devname, 16, "/dev/hidraw%d", i);
    R = open(devname, O_RDONLY | O_NONBLOCK);
    if (R < 0) continue;
    HiD[i] = R;
    R = ioctl(HiD[i], HIDIOCGRDESCSIZE, &rpt.size);
    if ((R < 0) || (rpt.size < 4)) EFAIL("HIDIOCGRDESCSIZE");
    R = ioctl(HiD[i], HIDIOCGRDESC, &rpt);
    if (R < 0) EFAIL("HIDIOCGRDESC");
    u8 *s = rpt.value;
    sprintx(L, s, rpt.size);
    write(1, L, rpt.size * 2);
    write(1, "\n", 1);
    if ((s[0] != 5) || (s[1] != 1) || (s[2] != 9)) continue;
    if ((s[3] != 2) && (s[3] != 6)) continue;
    if (s[3] == 2) HiD_type[i] = 'm';
    if (s[3] == 6) HiD_type[i] = 'k';
  }
  for (int i = 0; i < 26; i++) {
    if (HiD[i] > 0) {
      if (HiD_type[i] == 'k') printf("keyboard on fd %d\n", HiD[i]);
      if (HiD_type[i] == 'm') printf("mouse on fd %d\n", HiD[i]);
      if (HiD_type[i] == 0) {
        R = close(HiD[i]);
        if (R == -1) { perror("close"); }
        /*printf("something else was on fd %d\n", HiD[i]);*/
        HiD[i] = 0;
      }
    }
  }
  T = time(0);
  printf("%sloading pixmap from pngs\n", ctime(&T));
  for (int i = 0; i < 676; i++) {
  /* continue; */
    char c1 = 96 + 1 + (i / 26);
    char c2 = 96 + 1 + (i % 26);
    char filename[256];
    char *map_dir = getenv("OVERRIDE_PROGRAM_PIXMAP_BASEDIR");
    if (map_dir) printf("using %s for dat pix map\n", map_dir);
    if (!map_dir) map_dir = "/map";
    snprintf(filename, 256, "%s/%c/%c/%s%s.png", map_dir, c1, c2,
     nato[c1 - 97], nato[c2 - 97]);
     /*printf("Loading %3d %s\n", i, filename);*/
     cairo_surface_t *cst = cairo_image_surface_create_from_png(filename);
     cairo_status_t cairo_errno = cairo_surface_status(cst);
     if (cairo_errno) {
       printf("PC_LOAD_LETTER %s\n", cairo_status_to_string(cairo_errno));
       continue;
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
        int pxy = ((i % 26) * (1920 * 3)) + (y * (1920 * 26 * 3)) + (x * 3);
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
  drmModeModeInfo M = DCON->modes[0];
  W = M.hdisplay;
  H = M.vdisplay;
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
  mset(pixmap1, 126, cd_arg.pitch * H);
  R = drmModeAddFB(DD, W, H, 24, 32, cd_arg.pitch, cd_arg.handle, &fb_id);
  if (R) EFAIL("drmModeAddFB");

  drmModeCrtcPtr crtc;
  mset(&crtc, 0, sizeof(crtc));
  crtc = drmModeGetCrtc(DD, DENC->crtc_id);
  if (!crtc) EFAIL("drmModeGetCrtc");
  R = drmModeSetCrtc(DD, DENC->crtc_id, fb_id, 0, 0, &DCON->connector_id, 1, &M);
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
  mset(pixmap2, 126, cd2_arg.pitch * H);
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
  gettimeofday(&dctx.start, NULL);
  int DE = DRM_MODE_PAGE_FLIP_EVENT;
  R = drmModePageFlip(DD, DENC->crtc_id, fb2_id, DE, &dctx);
  if (R) EFAIL("failed to page flip");

  struct termios old_tio, new_tio;
  tcgetattr(STDIN_FILENO, &old_tio);
  new_tio = old_tio;
  new_tio.c_lflag &= (~ICANON & ~ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);

  drmEventContext evctx;
  mset(&evctx, 0, sizeof evctx);
  evctx.version = DRM_EVENT_CONTEXT_VERSION;
  evctx.vblank_handler = NULL;
  evctx.page_flip_handler = pageflip;

  struct epoll_event ev;
  ev.events = EPOLLIN;

  ev.data.fd = SD;
  R = epoll_ctl(PD, EPOLL_CTL_ADD, ev.data.fd, &ev);
  if (R) EFAIL("epoll_ctl");

  ev.data.fd = DD;
  R = epoll_ctl(PD, EPOLL_CTL_ADD, ev.data.fd, &ev);
  if (R) EFAIL("epoll_ctl");


  for (u64 i = 0; i < 676; i++) {
    X += 1920;
    if (X == 1920 * 26) { X = 0; Y += 1080; }
    if (Y == 1080 * 26) { Y = 0; }
    R = epoll_wait(PD, &ev, 1, 2601);
    if (R < 0) EFAIL("epoll_wait");
    if (R == 0) continue;
    if (ev.events & EPOLLERR) EFAIL("EPOLLERR");
    if (ev.events & EPOLLHUP) EFAIL("EPOLLHUP");
    if (ev.events & EPOLLIN) {
      int fd = ev.data.fd;
      if (fd == SD) {
        for (;;) {
          R = read(SD, L, sizeof(L));
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
          R = read(ID, L, sizeof(L));
          if (R > 0) continue;
          if ((R == -1) && (errno != EAGAIN)) EFAIL("read ID");
          break;
        }
      }
      for (int ui = 0; ui < 26; ui++) {
        if (HiD[ui] > 0) {
          if (HiD_type[ui] == 'k') {}
          if (HiD_type[ui] == 'm') {}
        }
      }
      if (fd == DD) {
        R = drmHandleEvent(DD, &evctx);
      }
    }
  }
  /* done for; restore */
  tcsetattr(STDIN_FILENO, TCSANOW, &old_tio);
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

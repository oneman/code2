#include "header.h"
/*
typedef enum {
  KR_V4L2_VOID,
  KR_V4L2_OPEN,
  KR_V4L2_CAPTURE
} kr_v4l2_state;

typedef struct {
  int width;
  int height;
  int num;
  int den;
  int format;
} kr_v4l2_path_info;

typedef struct {
  int dev;
  int priority;
  kr_v4l2_state state;
} kr_v4l2_info;
#define KR_V4L2_NFRAMES 12

typedef struct kr_v4l2 kr_v4l2;

typedef struct {
  struct v4l2_buffer buf;
  kr_frame frame;
  kr_v4l2 *kv;
} kv_frame;

struct kr_v4l2 {
  int fd;
  kr_v4l2_info *info;
  kr_v4l2_path_info *path_info;
  kr_adapter *adapter;
  kr_adapter_path *adapter_path;
  uint32_t nframes;
  kv_frame frames[KR_V4L2_NFRAMES];
  uint64_t total_frames;
};

static int read_frame(kr_v4l2 *kv, kr_frame **frame);
static int xioctl(int fd, int request, void *arg);
static int user_event(kr_event *event);
static int device_event(kr_event *event);
static int frame_release(void *ptr);
static int capture_off(kr_v4l2 *kv);
static int capture_on(kr_v4l2 *kv);
static void unmap_frames(kr_v4l2 *kv);
static int map_frames(kr_v4l2 *kv);
static int set_params(kr_v4l2 *kv);
static void teardown(kr_v4l2 *kv);
static int kv_init(kr_v4l2 *kv);

static int xioctl(int fd, int request, void *arg) {
  int r;
  do r = ioctl(fd, request, arg);
  while (-1 == r && EINTR == errno);
  return r;
}

static int frame_release(void *ptr) {
  kv_frame *kvframe;
  if (ptr == NULL) return -1;
  kvframe = (kv_frame *)ptr;
  if (-1 == xioctl(kvframe->kv->fd, VIDIOC_QBUF, &kvframe->buf)) {
    printke("V4L2: VIDIOC_QBUF");
    return -1;
  }
  return 0;
}

static int read_frame(kr_v4l2 *kv, kr_frame **frame) {
  struct v4l2_buffer buf;
  if (kv == NULL) return -1;
  if (frame == NULL) return -1;
  memset(&buf, 0, sizeof(buf));
  buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buf.memory = V4L2_MEMORY_MMAP;
  if (-1 == xioctl(kv->fd, VIDIOC_DQBUF, &buf)) {
    switch (errno) {
      case EAGAIN:
        return 0;
      default:
        printke("Krad V4L2: VIDIOC_DQBUF");
        return -1;
    }
  }
  kv->frames[buf.index].frame.info.ts = buf.timestamp;
  *frame = &kv->frames[buf.index].frame;
  *//*kr_frame_debug_time(*frame, "V4L2", 4);*//*
  return 1;
}

static int user_event(kr_event *event) {
  if (event == NULL) return -1;
  kr_v4l2 *kv;
  int ret;
  kr_frame frame;
  kv = (kr_v4l2 *)event->user;
  if (event->events & EPOLLIN) {
    //frame = NULL;
    ret = kv->adapter_path->read(kv->adapter_path->user, (kr_av *)&frame);
    *//*printk("V4L2: user event read ret %d", ret);*//*
    if (ret != 1) {
      printke("V4L2: problem reading frame back");
      return -1;
    }
    return ret;
  } else {
    printke("V4L2: Got an unreadable user event");
  }
  return 0;
}

static int device_event(kr_event *event) {
  if (event == NULL) return -1;
  kr_v4l2 *kv;
  int ret;
  kr_frame *frame;
  char dbgstr[64];
  kv = (kr_v4l2 *)event->user;
  if (event->events & EPOLLIN) {
    ret = read_frame(kv, &frame);
    if (ret != 1) {
      printke("V4L2: problem reading frame");
      return -1;
    }
    kv->total_frames++;
    snprintf(dbgstr, sizeof(dbgstr), "V4L2 CAP ;%010"PRIu64"",
     kv->total_frames);
    kr_image_debug_render(&frame->image, 1, dbgstr);
    ret = kv->adapter_path->write(kv->adapter_path->user, (kr_av *)frame);
    *//*printk("V4L2: device event write ret %d", ret);*//*
    if (ret != 1) {
      frame->image.release(frame->image.owner);
    }
    return ret;
  } else {
    printke("V4L2: Got an unreadable event");
  }
  return 0;
}

static int capture_off(kr_v4l2 *kv) {
  enum v4l2_buf_type type;
  if (kv == NULL) return -1;
  if (kv->fd == -1) return -1;
  type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if (kv->info->state != KR_V4L2_CAPTURE) {
    return 0;
  }
  if (-1 == xioctl(kv->fd, VIDIOC_STREAMOFF, &type)) {
    printke("V4L2: VIDIOC_STREAMOFF");
    kv->info->state = KR_V4L2_VOID;
    return -1;
  }
  kv->info->state = KR_V4L2_VOID;
  return 0;
}

static int capture_on(kr_v4l2 *kv) {
  uint32_t i;
  struct v4l2_buffer buf;
  enum v4l2_buf_type type;
  if (kv == NULL) return -1;
  if (kv->fd == -1) return -1;
  if (kv->nframes == 0) return -1;
  type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  for (i = 0; i < kv->nframes; i++) {
    memset(&buf, 0, sizeof(buf));
    buf.type = type;
    buf.index = i;
    buf.memory = V4L2_MEMORY_MMAP;
    if (-1 == xioctl(kv->fd, VIDIOC_QBUF, &buf)) {
      printke("V4L2: VIDIOC_QBUF");
      return -1;
    }
  }
  if (-1 == xioctl(kv->fd, VIDIOC_STREAMON, &type)) {
    printke("V4L2: VIDIOC_STREAMON");
    return -1;
  }
  kv->info->state = KR_V4L2_CAPTURE;
  return 0;
}

static void unmap_frames(kr_v4l2 *kv) {
  int i;
  kv_frame *kvframe;
  if (kv->nframes > 0) {
    capture_off(kv);
    for (i = 0; i < kv->nframes; i++) {
      kvframe = &kv->frames[i];
      if (-1 == munmap(kvframe->frame.image.px[0], kvframe->buf.length)) {
        printke("V4L2: munmap %d", i);
      }
    }
    kv->nframes = 0;
  }
}

static int map_frames(kr_v4l2 *kv) {
  int i;
  struct v4l2_buffer buf;
  struct v4l2_requestbuffers req;
  kr_frame *frame;
  kr_image *image;
  memset(&req, 0, sizeof(req));
  req.count = KR_V4L2_NFRAMES;
  req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  req.memory = V4L2_MEMORY_MMAP;
  if (-1 == xioctl(kv->fd, VIDIOC_REQBUFS, &req)) {
    if (EINVAL == errno) {
      printke("V4L2: device does not support memory mapping");
    } else {
      printke("V4L2: VIDIOC_REQBUFS");
    }
    return -1;
  }
  if (req.count < 2) {
    printke("V4L2: Insufficient buffer memory");
    return -1;
  }
  kv->nframes = req.count;
  printk("V4L2: %d buffer frames", kv->nframes);
  for (i = 0; i < kv->nframes; i++) {
    memset(&buf, 0, sizeof(buf));
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = i;
    if (-1 == xioctl(kv->fd, VIDIOC_QUERYBUF, &buf)) {
      printke("V4L2: VIDIOC_QUERYBUF");
      kv->nframes = 0;
      return -1;
    }
    frame = &kv->frames[i].frame;
    image = &frame->image;
    kv->frames[i].kv = kv;
    kv->frames[i].buf = buf;
    image->px[0] = mmap(NULL, buf.length,
     PROT_READ | PROT_WRITE, MAP_SHARED, kv->fd, buf.m.offset);
    if (MAP_FAILED == image->px[0]) {
      printke("V4L2: mmap failure");
      kv->nframes = 0;
      return -1;
    }
    image->px[1] = NULL;
    image->px[2] = NULL;
    image->px[3] = NULL;
    image->ps[0] = kv->path_info->width * 2;
    image->ps[1] = 0;  *//* kv->path_info->width/2; *//*
    image->ps[2] = 0;  *//* kv->path_info->width/2; *//*
    image->ps[3] = 0;
    image->info.w = kv->path_info->width;
    image->info.h = kv->path_info->height;
    image->info.fmt = KR_PIXELS_YUV422P;
    image->release = frame_release;
    image->owner = &kv->frames[buf.index];
  }
  return 0;
}

static int set_params(kr_v4l2 *kv) {
  kr_v4l2_path_info *info;
  info = kv->path_info;
  struct v4l2_format format;
  struct v4l2_streamparm sps;
  memset(&sps, 0, sizeof(sps));
  memset(&format, 0, sizeof(format));
  format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  format.fmt.pix.width = info->width;
  format.fmt.pix.height = info->height;
  format.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
  format.fmt.pix.field = V4L2_FIELD_ANY;
  if (-1 == xioctl(kv->fd, VIDIOC_S_FMT, &format)) {
    printke("V4L2: Unable to set desired video format");
    return -1;
  } else {
    printk("V4L2: Set %dx%d", info->width, info->height);
  }
  sps.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if (-1 == xioctl(kv->fd, VIDIOC_G_PARM, &sps)) {
    printke("V4L2: Unable to get capture parameters");
    return -1;
  }
  sps.parm.capture.timeperframe.numerator = info->den;
  sps.parm.capture.timeperframe.denominator = info->num;
  if (-1 == xioctl(kv->fd, VIDIOC_S_PARM, &sps)) {
    printke("V4L2: Unable to set desired capture parameters");
    printke("V4L2: error %d, %s", errno, strerror(errno));
    return -1;
  }
  return 0;
}

static void teardown(kr_v4l2 *kv) {
  if (kv->fd > -1) {
    capture_off(kv);
    unmap_frames(kv);
    close(kv->fd);
    kv->fd = -1;
    kv->info->state = KR_V4L2_VOID;
  }
}

static int kv_init(kr_v4l2 *kv) {
  struct stat st;
  char device[128];
  struct v4l2_capability cap;
  kv->total_frames = 0;
  snprintf(device, sizeof(device), "/dev/video%d", kv->info->dev);
  if (-1 == stat(device, &st)) {
    printke("V4L2: Can't reckon '%s': %d, %s", device, errno, strerror(errno));
    return -1;
  }
  if (!S_ISCHR(st.st_mode)) {
    printke("V4L2: %s is not a device", device);
    return -2;
  }
  kv->fd = open(device, O_RDWR | O_NONBLOCK, 0);
  if (-1 == kv->fd) {
    printke("V4L2: Access denied '%s': %d, %s", device, errno,
     strerror(errno));
    return -3;
  }
  if (-1 == xioctl(kv->fd, VIDIOC_QUERYCAP, &cap)) {
    printke("V4L2: VIDIOC_QUERYCAP");
    teardown(kv);
    return -4;
  } else {
    if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
      printke("V4L2: %s is not a video capture device", device);
      teardown(kv);
      return -5;
    }
    if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
      printke("V4L2: %s no have streamin support we need", device);
      teardown(kv);
      return -6;
    }
  }
  kv->info->state = KR_V4L2_OPEN;
  return 0;
}

int kr_v4l2_lctl(kr_adapter_path *path, kr_patchset *patchset) {
  if (path == NULL) return -1;
  if (patchset == NULL) return -2;
  printk("V4L2: Adapter path control");
  printk("V4L2: Adapter path controlled");
  return 0;
}

int kr_v4l2_unlink(kr_adapter_path *path) {
  if (path == NULL) return -1;
  printk("V4L2: unlink (not implemented)");
  return 0;
}

int kr_v4l2_link(kr_adapter *adapter, kr_adapter_path *path) {
  int ret;
  kr_v4l2 *kv;
  kr_event harness;
  if (adapter == NULL) return -1;
  kv = (kr_v4l2 *)adapter->handle;
  if (path == NULL) return -2;
  printk("V4L2: Adapter path create");
  kv->adapter_path = path;
  kv->path_info = &path->info->v4l2_in;
  ret = set_params(kv);
  if (ret != 0) return ret;
  ret = map_frames(kv);
  if (ret != 0) return ret;
  ret = capture_on(kv);
  if (ret != 0) return ret;
  harness.user = kv;
  harness.fd = kv->fd;
  harness.handler = device_event;
  harness.events = EPOLLIN;
  kr_loop_add(adapter->loop, &harness);
  harness.fd = path->fd;
  harness.handler = user_event;
  harness.events = EPOLLIN;
  kr_loop_add(adapter->loop, &harness);
  printk("V4L2: Adapter path created");
  return ret;
}

int kr_v4l2_ctl(kr_adapter *adapter, kr_patchset *patchset) {
  if (adapter == NULL) return -1;
  if (patchset == NULL) return -2;
  printk("V4L2: Adapter control");
  printk("V4L2: Adapter controlled");
  return 0;
}

int kr_v4l2_close(kr_adapter *adapter) {
  int ret;
  kr_v4l2 *kv;
  if (adapter == NULL) return -1;
  kv = (kr_v4l2 *)adapter->handle;
  if (kv == NULL) return -1;
  printk("V4L2: Adapter closing");
  teardown(kv);
  ret = kr_loop_destroy(adapter->loop);
  if (ret != 0) {
    printke("V4L2: trouble unlooping");
  }
  free(kv);
  adapter->handle = NULL;
  printk("V4L2: adapter closed");
  return 0;
}

int kr_v4l2_open(kr_adapter *adapter) {
  int ret;
  kr_v4l2 *kv;
  kr_loop_setup loop_setup;
  if (adapter == NULL) return -1;
  printk("V4L2: Adapter opening");
  adapter->handle = kr_allocz(1, sizeof(kr_v4l2));
  kv = (kr_v4l2 *)adapter->handle;
  kv->adapter = adapter;
  kv->info = &adapter->info->v4l2;
  ret = kv_init(kv);
  if (ret != 0) return ret;
  adapter->fd = kv->fd;
  kr_loop_setup_init(&loop_setup);
  snprintf(loop_setup.name, sizeof(loop_setup.name), "kr_v4l2_%d", kv->info->dev);
  adapter->loop = kr_loop_create(&loop_setup);
  printk("V4L2: adapter opened");
  return 0;
}
#include <alsa/asoundlib.h>
typedef struct {
  int card;
  char name[40];
} kr_alsa_info;
struct kr_alsa {
  kr_alsa_info info;
  snd_ctl_t *ctl;
};
kr_alsa *kr_alsa_create(int card) {
  kr_alsa *alsa;
  int ret;
  const char *name;
  char dev_name[8];
  int pcm_device;
  int pcm_playback;
  int pcm_capture;
  snd_ctl_card_info_t *card_info;
  snd_pcm_info_t *pcm_info;
  kr_alsa_info *info;
  pcm_device = -1;
  alsa = kr_allocz(1, sizeof(kr_alsa));
  if (!alsa) return NULL;
  info = &alsa->info;
  info->card = card;
  sprintf(dev_name, "hw:%d", info->card);
  snd_ctl_card_info_alloca(&card_info);
  snd_pcm_info_alloca(&pcm_info);
  ret = snd_ctl_open(&alsa->ctl, dev_name, 0);
  if (ret != 0) {
    printk("Could not open %s", dev_name);
    return alsa;
  }
  snd_ctl_card_info_clear(card_info);
  snd_ctl_card_info(alsa->ctl, card_info);
  name = snd_ctl_card_info_get_name(card_info);
  strncpy(info->name, name, sizeof(info->name));
  printk("Krad ALSA: Created %s", info->name);
  do {
    ret = snd_ctl_pcm_next_device(alsa->ctl, &pcm_device);
    if ((ret == 0) && (pcm_device >= 0)) {
      pcm_capture = 0;
      pcm_playback = 0;
      memset(pcm_info, 0, snd_pcm_info_sizeof());
      snd_pcm_info_set_device(pcm_info, pcm_device);
      snd_pcm_info_set_stream(pcm_info, SND_PCM_STREAM_PLAYBACK);
      snd_pcm_info_set_subdevice(pcm_info, 0);
      ret = snd_ctl_pcm_info(alsa->ctl, pcm_info);
      if (ret == 0) {
        pcm_playback = 1;
      }
      snd_pcm_info_set_stream(pcm_info, SND_PCM_STREAM_CAPTURE);
      ret = snd_ctl_pcm_info(alsa->ctl, pcm_info);
      if (ret == 0) {
        pcm_capture = 1;
      }
      name = snd_pcm_info_get_name(pcm_info);
      if (name == NULL) {
        name = snd_pcm_info_get_id(pcm_info);
      }
      printk(" %s Capture: %d Playback: %d", name, pcm_capture, pcm_playback);
    }
  } while (pcm_device >= 0);
  return alsa;
}
#include <xcb/xcb.h>
int xmain (int argc, char **argv) {
  xcb_connection_t *c;
  xcb_screen_t *s;
  xcb_window_t w;
  xcb_gcontext_t g;
  xcb_generic_event_t *e;
  uint32_t mask;
  uint32_t values[2];
  int done;
  int i;
  xcb_rectangle_t r = { 60, 60, 320, 180 };
  c = xcb_connect(NULL, NULL);
  if (!c) {
    printf("Cannot open display\n");
    exit(1);
  }
  s = xcb_setup_roots_iterator(xcb_get_setup(c)).data;
  mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
  values[0] = s->white_pixel;
  values[1] = XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_KEY_PRESS;
  w = xcb_generate_id(c);
  xcb_create_window(c, XCB_COPY_FROM_PARENT, w, s->root, 10, 10, 640, 360, 1,
   XCB_WINDOW_CLASS_INPUT_OUTPUT, s->root_visual, mask, values);
  mask = XCB_GC_FOREGROUND | XCB_GC_GRAPHICS_EXPOSURES;
  values[0] = s->black_pixel;
  values[1] = 0;
  g = xcb_generate_id(c);
  xcb_create_gc(c, g, w, mask, values);
  xcb_map_window(c, w);
  xcb_flush(c);
  done = 0;
  i = 0;
  while (!done && (e = xcb_wait_for_event(c))) {
    switch (e->response_type) {
      case XCB_EXPOSE:
        printf("EXPOSE %d \r", i++);
        fflush(stdout);
        xcb_poly_fill_rectangle(c, w, g, 1, &r);
        xcb_flush(c);
        break;
      case XCB_KEY_PRESS:
        done = 1;
        break;
    }
    free(e);
  }
  xcb_disconnect(c);
  return 0;
}
* above we have some of our old code for v4l2, alsa and xcb
*/

struct flip_context {
  u32 fb_id[2];
  u32 current_fb_id;
  int crtc_id;
  struct timeval start;
  u64 swap_count;
  char *pixmap1;
  char *pixmap2;
  int pixmap_sz;
};

void pageflip(int fd, u32 frame, u32 sec, u32 usec, void *data) {
  struct flip_context *context;
  unsigned int new_fb_id;
  struct timeval end;
  double t;
  context = (struct flip_context *)data;
  if (context->current_fb_id == context->fb_id[0]) {
    new_fb_id = context->fb_id[1];
    mset(context->pixmap2, 255 - context->swap_count, context->pixmap_sz);
  } else {
    new_fb_id = context->fb_id[0];
    mset(context->pixmap1, 255 - context->swap_count, context->pixmap_sz);
  }
  drmModePageFlip(fd, context->crtc_id, new_fb_id, DRM_MODE_PAGE_FLIP_EVENT,
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

void sprintx(char *dst, const char *src, int n)  {
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
  time_t T = time(0);
  time_t T0 = T;
  char L[80];
  mset(L, 0, 80);
  int R = snprintf(L, 80, "program begins %sgmp %s cairo %s\n", ctime(&T0),
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
  int ED = eventfd(2601, EFD_NONBLOCK | EFD_CLOEXEC);
  int TD = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
  int ND = socket(AF_PACKET,
                  SOCK_RAW | SOCK_NONBLOCK | SOCK_CLOEXEC, htons(ETH_P_ALL));
  int ID = inotify_init1(IN_NONBLOCK | IN_CLOEXEC);
  if ((6*6+6) != (SD + MD + PD + ED + TD + ND + ID)) EFAIL("6*6+6=42 PANICAN");
  int WD = inotify_add_watch(ID, "/dev", IN_CREATE);
  if (WD == -1) EFAIL("6 inotify_add_watch /dev IN_CREATE");
  R = ftruncate(MD, 4205260800);
  if (R == -1) EFAIL("7 ftruncate 4205260800");
  char *DAT = mmap(NULL, 4205260800, PROT_READ | PROT_WRITE, MAP_SHARED, MD, 0);
  if (!DAT) EFAIL("8 mmap 4205260800");
  R = mlock(DAT, 4205260800);
  if (R == -1) EFAIL("9 mlock 4205260800");
  mset(DAT, 'K', 4205260800);
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
  mpz_t a, b, c, d, r, x;
  mpz_init_set_ui(b, 0);
  mpz_init_set_ui(c, 0);
  mpz_init_set_ui(r, 0);
  mpz_init_set_str(a, "4205260800", 10);
  mpz_init_set_str(d, "4205260799", 10);
  mpz_init_set_str(x, "18446744073709551616", 10);
  mpz_mod(r, d, x);
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
  mpz_clear(x);
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
    unsigned char *s = rpt.value;
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
    unsigned char *dat = cairo_image_surface_get_data(cst);
    unsigned long x0 = (i % 26) * (1920*3);
    int stride = 1920 * 26 * 3;
    for (int yy = 0; yy < 1080; yy++) {
      int pyy = yy * stride;
      for (int xx = 0; xx < 1920; xx++) {
        int pxy = x0 + pyy + (xx * 3);
        DAT[pxy] = dat[(yy * 4) + (xx * 4)];
        DAT[pxy + 1] = dat[(yy * 4) + (xx * 4) + 1];
        DAT[pxy + 2] = dat[(yy * 4) + (xx * 4) + 2];
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
  int W = M.hdisplay;
  int H = M.vdisplay;
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
  char *pixmap1 = mmap(0, cd_arg.size, PROT_READ | PROT_WRITE, MAP_SHARED, DD,
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
  char *pixmap2 = mmap(0, cd2_arg.size, PROT_READ | PROT_WRITE, MAP_SHARED, DD,
                          md2_arg.offset);
  mset(pixmap2, 126, cd2_arg.pitch * H);
  R = drmModeAddFB(DD, W, H, 24, 32, cd2_arg.pitch, cd2_arg.handle, &fb2_id);
  if (R) EFAIL("drmModeAddFB failed");

  struct flip_context flip_context;
  mset(&flip_context, 0, sizeof flip_context);
  flip_context.pixmap1 = pixmap1;
  flip_context.pixmap2 = pixmap2;
  flip_context.pixmap_sz = H * cd2_arg.pitch;
  flip_context.fb_id[0] = fb_id;
  flip_context.fb_id[1] = fb2_id;
  flip_context.current_fb_id = fb2_id;
  flip_context.crtc_id = DENC->crtc_id;
  flip_context.swap_count = 0;
  gettimeofday(&flip_context.start, NULL);
  R = drmModePageFlip(DD, DENC->crtc_id, fb2_id, DRM_MODE_PAGE_FLIP_EVENT, &flip_context);
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
  ev.data.fd = DD;
  R = epoll_ctl(PD, EPOLL_CTL_ADD, ev.data.fd, &ev);
  if (R) EFAIL("epoll_ctlfail");
  ev.data.fd = SD;
  R = epoll_ctl(PD, EPOLL_CTL_ADD, ev.data.fd, &ev);
  if (R) EFAIL("epoll_ctlfail");

  for (u64 i = 0;i < 676;i++) {
    /*T = time(0);
    mset(L, 0, 80);
    R = snprintf(L, 80, "%s program loop %lu\n", ctime(&T), i);
    write(1, L, R);*/
    R = epoll_wait(PD, &ev, 1, 1000);
    if (R == -1) { printf("epoll_waitfail: %s\n", strerror(errno)); exit(1); }
    if (R == 0) { printf("epoll_wait, long time, 1000ms!\n"); continue; }
    if (ev.events & EPOLLIN) {
      if (ev.data.fd == DD) { R = drmHandleEvent(DD, &evctx); continue; }
      if (ev.data.fd == SD) { break; }
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

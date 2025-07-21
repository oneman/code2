#include <stdio.h>
#include <gmp.h>
#include <cairo.h>
#include <pipewire/pipewire.h>
#include <sys/types.h>
#include <sys/stat.h>
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

typedef struct kr_adapter kr_adapter;
typedef struct kr_adapter_spec kr_adapter_spec;
typedef struct kr_adapter_path kr_adapter_path;

typedef enum {
  KR_ADAPTER_PROCESS, /* Process all paths: jack,alsa,decklink */
  KR_ADAPTER_DISCONNECTED,
  KR_ADAPTER_RECONNECTED
} kr_adapter_event_type;

typedef struct {
  kr_adapter *adapter;
  void *user;
  kr_adapter_event_type type;
} kr_adapter_event;

typedef void (kr_adapter_event_cb)(kr_adapter_event *);

struct kr_adapter {
  void *handle;
  int fd;
  void *user;
  kr_adapter_event_cb *event_cb;
  //kr_xpdr_path_info *info;
  //kr_loop *loop;
  //kr_loop *master_loop;
};

//typedef int (kr_avio)(void *, kr_av *);

struct kr_adapter_path {
  void *handle;
  void *user;
  int fd;
  //kr_avio *read;
  //kr_avio *write;
  kr_av_handler *av_handler;
  //kr_xpdr_path_info *info;
  int aux_fds[4];
  int naux_fds;
};

typedef int (kr_adapter_lctl)(kr_adapter_path *, kr_patchset *);
typedef int (kr_adapter_unlink)(kr_adapter_path *);
typedef int (kr_adapter_link)(kr_adapter *, kr_adapter_path *);
typedef int (kr_adapter_ctl)(kr_adapter *, kr_patchset *);
typedef int (kr_adapter_close)(kr_adapter *);
typedef int (kr_adapter_open)(kr_adapter *);

struct kr_adapter_spec {
  kr_adapter_lctl *lctl;
  kr_adapter_unlink *unlink;
  kr_adapter_link *link;
  kr_adapter_ctl *ctl;
  kr_adapter_close *close;
  kr_adapter_open *open;
};

typedef enum {
  KR_WL_DISCONNECTED = 1,
  KR_WL_CONNECTED
} kr_wayland_state;

typedef struct {
  char display_name[128];
  kr_wayland_state state;
} kr_wayland_info;

typedef struct {
  int width;
  int height;
  int fullscreen;
} kr_wayland_path_info;

int kr_wl_lctl(kr_adapter_path *, kr_patchset *);
int kr_wl_unlink(kr_adapter_path *);
int kr_wl_link(kr_adapter *, kr_adapter_path *);
int kr_wl_ctl(kr_adapter *, kr_patchset *);
int kr_wl_close(kr_adapter *);
int kr_wl_open(kr_adapter *);

static void handle_seat_capabilities(void *data, struct wl_seat *seat,
 enum wl_seat_capability caps) {
  kr_wayland *kw;
  kw = (kr_wayland *)data;
  if ((caps & WL_SEAT_CAPABILITY_POINTER) && !kw->pointer) {
    kw->pointer = wl_seat_get_pointer(seat);
    /*wl_pointer_set_user_data(wayland->pointer, wayland);*/
    wl_pointer_add_listener(kw->pointer, &kw->pointer_listener, kw);
  } else if (!(caps & WL_SEAT_CAPABILITY_POINTER) && kw->pointer) {
    wl_pointer_destroy(kw->pointer);
    kw->pointer = NULL;
  }
  if ((caps & WL_SEAT_CAPABILITY_KEYBOARD) && !kw->keyboard) {
    kw->keyboard = wl_seat_get_keyboard(seat);
    /*wl_keyboard_set_user_data(wayland->keyboard, wayland);*/
    wl_keyboard_add_listener(kw->keyboard, &kw->keyboard_listener, kw);
  } else if (!(caps & WL_SEAT_CAPABILITY_KEYBOARD) && kw->keyboard) {
    wl_keyboard_destroy(kw->keyboard);
    kw->keyboard = NULL;
  }
}

static void keyboard_handle_modifiers(void *data, struct wl_keyboard *keyboard,
 uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched,
 uint32_t mods_locked, uint32_t group) {
  /* Nothing here */
}

static void keyboard_handle_keymap(void *data, struct wl_keyboard *keyboard,
 uint32_t format, int fd, uint32_t size) {
  kr_wayland *wayland = data;
  char *map_str;
  if (!data) {
    close(fd);
    return;
  }
  if (format != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1) {
    close(fd);
    return;
  }
  map_str = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
  if (map_str == MAP_FAILED) {
    close(fd);
    return;
  }
  wayland->xkb.keymap = xkb_map_new_from_string(wayland->xkb.context,
   map_str, XKB_KEYMAP_FORMAT_TEXT_V1, 0);
  munmap(map_str, size);
  close(fd);
  if (!wayland->xkb.keymap) {
    fprintf(stderr, "failed to compile keymap\n");
    return;
  }
  wayland->xkb.state = xkb_state_new(wayland->xkb.keymap);
  if (!wayland->xkb.state) {
    fprintf(stderr, "failed to create XKB state\n");
    xkb_map_unref(wayland->xkb.keymap);
    wayland->xkb.keymap = NULL;
    return;
  }
  wayland->xkb.control_mask =
    1 << xkb_map_mod_get_index(wayland->xkb.keymap, "Control");
  wayland->xkb.alt_mask =
    1 << xkb_map_mod_get_index(wayland->xkb.keymap, "Mod1");
  wayland->xkb.shift_mask =
    1 << xkb_map_mod_get_index(wayland->xkb.keymap, "Shift");
}

static void keyboard_handle_enter(void *data, struct wl_keyboard *keyboard,
 uint32_t serial, struct wl_surface *surface, struct wl_array *keys) {
  int i;
  kr_wayland *wayland;
  wayland = data;
  if (!surface) {
    return;
  }
  for (i = 0; i < KR_WL_MAX_WINDOWS; i++) {
    if (wayland->window[i].active == 1) {
      if (wayland->window[i].surface == surface) {
        if (wayland->key_window != &wayland->window[i]) {
          wayland->key_window = &wayland->window[i];
          break;
        }
      }
    }
  }
  if (i == KR_WL_MAX_WINDOWS) {
    return;
  }
}

static void keyboard_handle_leave(void *data, struct wl_keyboard *keyboard,
 uint32_t serial, struct wl_surface *surface) {
  int i;
  kr_wayland *wayland;
  wayland = data;
  for (i = 0; i < KR_WL_MAX_WINDOWS; i++) {
    if (wayland->window[i].active == 1) {
      if (wayland->window[i].surface == surface) {
        if (wayland->key_window == &wayland->window[i]) {
          wayland->key_window = NULL;
          break;
        }
      }
    }
  }
  if (i == KR_WL_MAX_WINDOWS) {
    return;
  }
}

static void keyboard_handle_key(void *data, struct wl_keyboard *keyboard,
 uint32_t serial, uint32_t time, uint32_t key, uint32_t state_w) {
  kr_wayland *wayland;
  kr_wayland_event wayland_event;
  kr_wayland_path *window;
  uint32_t code, num_syms;
  enum wl_keyboard_key_state state = state_w;
  const xkb_keysym_t *syms;
  xkb_keysym_t sym;
  int ret;
  //struct itimerspec its;
  //input->display->serial = serial;
  wayland = data;
  if (wayland->key_window == NULL) {
    return;
  } else {
    window = wayland->key_window;
  }
  code = key + 8;
  if (!wayland->xkb.state)
    return;
  num_syms = xkb_key_get_syms(wayland->xkb.state, code, &syms);
  sym = XKB_KEY_NoSymbol;
  if (num_syms == 1) {
    sym = syms[0];
  }
  if (((sym == XKB_KEY_F11) || (sym == XKB_KEY_f))
    && (state == WL_KEYBOARD_KEY_STATE_PRESSED)) {
    if (window->fullscreen == 1) {
      window->fullscreen = 0;
      //wl_shell_surface_set_toplevel(window->shell_surface);
    } else {
      window->fullscreen = 1;
      //wl_shell_surface_set_fullscreen(window->shell_surface, 1, 0, NULL);
    }
  }
/*
  if (sym == XKB_KEY_F5 && input->modifiers == MOD_ALT_MASK) {
    if (state == WL_KEYBOARD_KEY_STATE_PRESSED)
      window_set_maximized(window,
               window->type != TYPE_MAXIMIZED);
  } else if (sym == XKB_KEY_F11 &&
       window->fullscreen_handler &&
       state == WL_KEYBOARD_KEY_STATE_PRESSED) {
    window->fullscreen_handler(window, window->user_data);
  } else if (sym == XKB_KEY_F4 &&
       input->modifiers == MOD_ALT_MASK &&
       state == WL_KEYBOARD_KEY_STATE_PRESSED) {
    if (window->close_handler)
      window->close_handler(window->parent,
                window->user_data);
    else
      display_exit(window->display);
  } else if (window->key_handler) {
    (*window->key_handler)(window, input, time, key,
               sym, state, window->user_data);
  }

  if (state == WL_KEYBOARD_KEY_STATE_RELEASED &&
      key == input->repeat_key) {
    its.it_interval.tv_sec = 0;
    its.it_interval.tv_nsec = 0;
    its.it_value.tv_sec = 0;
    its.it_value.tv_nsec = 0;
    timerfd_settime(input->repeat_timer_fd, 0, &its, NULL);
  } else if (state == WL_KEYBOARD_KEY_STATE_PRESSED) {
    input->repeat_sym = sym;
    input->repeat_key = key;
    input->repeat_time = time;
    its.it_interval.tv_sec = 0;
    its.it_interval.tv_nsec = 25 * 1000 * 1000;
    its.it_value.tv_sec = 0;
    its.it_value.tv_nsec = 400 * 1000 * 1000;
    timerfd_settime(input->repeat_timer_fd, 0, &its, NULL);
  }
*/
  if (state == WL_KEYBOARD_KEY_STATE_PRESSED) {
    wayland_event.key_event.down = 1;
  } else {
    wayland_event.key_event.down = 0;
  }
  if ((wayland->key_window != NULL)
      && (wayland->key_window->user_callback != NULL)) {
    wayland_event.type = KR_WL_KEY;
    wayland_event.key_event.key = sym;
    ret =
     wayland->key_window->user_callback(wayland->key_window,
     &wayland_event);
    if (ret < 0) {
    }
  }
}

static void pointer_handle_enter(void *data, struct wl_pointer *pointer,
 uint32_t serial, struct wl_surface *surface, wl_fixed_t x, wl_fixed_t y) {
  int ret;
  int i;
  kr_wayland *wayland;
  kr_wayland_event wayland_event;
  if (!surface) {
    return;
  }
  wayland = data;
  for (i = 0; i < KR_WL_MAX_WINDOWS; i++) {
    if (wayland->window[i].active == 1) {
      if (wayland->window[i].surface == surface) {
        if (wayland->pointer_window != &wayland->window[i]) {
          wayland->pointer_window = &wayland->window[i];
          break;
        }
      }
    }
  }
  if (i == KR_WL_MAX_WINDOWS) {
    return;
  }
  wayland->pointer_window->pointer_x = wl_fixed_to_int(x);
  wayland->pointer_window->pointer_y = wl_fixed_to_int(y);
  wayland->pointer_window->pointer_in = 1;
  wayland->pointer_window->pointer_out = 0;
  wl_pointer_set_cursor(pointer, serial, NULL, 0, 0);
  if (wayland->pointer_window->user_callback != NULL) {
    wayland_event.type = KR_WL_POINTER;
    wayland_event.pointer_event.x = wayland->pointer_window->pointer_x;
    wayland_event.pointer_event.y = wayland->pointer_window->pointer_y;
    wayland_event.pointer_event.click = wayland->pointer_window->click;
    wayland_event.pointer_event.pointer_in = wayland->pointer_window->pointer_in;
    wayland_event.pointer_event.pointer_out = wayland->pointer_window->pointer_out;
    ret =
     wayland->pointer_window->user_callback(wayland->pointer_window,
      &wayland_event);
    if (ret < 0) {
    }
  }
}

static void pointer_handle_leave(void *data, struct wl_pointer *pointer,
 uint32_t serial, struct wl_surface *surface) {
  int ret;
  int i;
  kr_wayland *wayland;
  kr_wayland_event wayland_event;
  wayland = data;
  if (!surface) {
    return;
  }
  for (i = 0; i < KR_WL_MAX_WINDOWS; i++) {
    if (wayland->window[i].active == 1) {
      if (wayland->window[i].surface == surface) {
        if (wayland->pointer_window == &wayland->window[i]) {
          break;
        }
      }
    }
  }
  if (i == KR_WL_MAX_WINDOWS) {
    return;
  }
  wayland->pointer_window->pointer_x = -1;
  wayland->pointer_window->pointer_y = -1;
  wayland->pointer_window->pointer_in = 0;
  wayland->pointer_window->pointer_out = 1;
  wayland->pointer_window->click = 0;
  if (wayland->pointer_window->user_callback != NULL) {
    wayland_event.type = KR_WL_POINTER;
    wayland_event.pointer_event.x = wayland->pointer_window->pointer_x;
    wayland_event.pointer_event.y = wayland->pointer_window->pointer_y;
    wayland_event.pointer_event.click = wayland->pointer_window->click;
    wayland_event.pointer_event.pointer_in = wayland->pointer_window->pointer_in;
    wayland_event.pointer_event.pointer_out = wayland->pointer_window->pointer_out;
    ret =
     wayland->pointer_window->user_callback(wayland->pointer_window,
     &wayland_event);
    if (ret < 0) {
    }
  }
  wayland->pointer_window = NULL;
}

static void pointer_handle_motion(void *data, struct wl_pointer *pointer,
 uint32_t time, wl_fixed_t x, wl_fixed_t y) {
  int ret;
  kr_wayland *wayland;
  kr_wayland_event wayland_event;
  wayland = data;
  wayland->pointer_window->pointer_x = wl_fixed_to_int(x);
  wayland->pointer_window->pointer_y = wl_fixed_to_int(y);
  wayland->pointer_window->pointer_in = 0;
  wayland->pointer_window->pointer_out = 0;
  if ((wayland->pointer_window != NULL)
      && (wayland->pointer_window->user_callback != NULL)) {
    wayland_event.type = KR_WL_POINTER;
    wayland_event.pointer_event.x = wayland->pointer_window->pointer_x;
    wayland_event.pointer_event.y = wayland->pointer_window->pointer_y;
    wayland_event.pointer_event.click = wayland->pointer_window->click;
    wayland_event.pointer_event.pointer_in = 0;
    wayland_event.pointer_event.pointer_out = 0;
    ret =
     wayland->pointer_window->user_callback(wayland->pointer_window,
     &wayland_event);
    if (ret < 0) {
    }
  }
}

static void pointer_handle_button(void *data, struct wl_pointer *pointer,
 uint32_t serial, uint32_t time, uint32_t button, uint32_t state_w) {
  int ret;
  kr_wayland *wayland;
  kr_wayland_event wayland_event;
  enum wl_pointer_button_state state = state_w;
  wayland = data;
  if (wayland->pointer_window != NULL) {
    if (state == WL_POINTER_BUTTON_STATE_PRESSED) {
      wayland->pointer_window->click = 1;
    }
    if (state == WL_POINTER_BUTTON_STATE_RELEASED) {
      wayland->pointer_window->click = 0;
    }
    wayland_event.type = KR_WL_POINTER;
    wayland_event.pointer_event.x = wayland->pointer_window->pointer_x;
    wayland_event.pointer_event.y = wayland->pointer_window->pointer_y;
    wayland_event.pointer_event.click = wayland->pointer_window->click;
    wayland_event.pointer_event.pointer_in = 0;
    wayland_event.pointer_event.pointer_out = 0;
    if (wayland->pointer_window->user_callback != NULL) {
      ret =
       wayland->pointer_window->user_callback(wayland->pointer_window,
       &wayland_event);
      if (ret < 0) {
      }
    }
  }
}

static void pointer_handle_axis(void *data, struct wl_pointer *pointer,
 uint32_t time, uint32_t axis, wl_fixed_t value) {
  /* Nothing here */
}


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
#include "wayland.h"

#define KR_WL_MAX_WINDOWS 4
#define KR_WL_NFRAMES 2
#define KR_WL_FIRST_FRAME_TIMEOUT_MS 2601

#include "xdg-shell-client-protocol.h"
#include "xdg-shell-protocol.c"

typedef struct kr_wayland kr_wayland;
typedef struct kr_wayland_path kr_wayland_path;

typedef enum {
  KR_WL_FRAME,
  KR_WL_POINTER,
  KR_WL_KEY
} kr_wayland_event_type;

typedef struct {
  int x;
  int y;
  int click;
  int pointer_in;
  int pointer_out;
} kr_wayland_pointer_event;

typedef struct {
  int key;
  int down;
} kr_wayland_key_event;

typedef struct {
  uint8_t *buffer;
} kr_wayland_frame_event;

typedef struct {
  kr_wayland_event_type type;
  kr_wayland_pointer_event pointer_event;
  kr_wayland_key_event key_event;
  kr_wayland_frame_event frame_event;
} kr_wayland_event;

struct kr_wayland_path {
  int pointer_x;
  int pointer_y;
  int click;
  int pointer_in;
  int pointer_out;
  int active;
  int fullscreen;
  int nframes;
  struct wl_surface *surface;
  struct xdg_surface *xdg_surface;
  struct xdg_toplevel *xdg_toplevel;
  struct wl_callback *frame_cb;
  struct wl_callback_listener frame_listener;
  struct xdg_surface_listener xdg_surface_listener;
  struct wl_buffer *buffer[KR_WL_NFRAMES];
  kr_frame frames[KR_WL_NFRAMES];
  kr_image_pool *pool;
  int (*user_callback)(void *, kr_wayland_event *);
  void *user;
  kr_wayland_path_info *info;
  kr_wayland *wayland;
  kr_adapter_path *adapter_path;
};

struct kr_wayland {
  int fd;
  kr_wayland_path window[KR_WL_MAX_WINDOWS];
  kr_wayland_path *key_window;
  kr_wayland_path *pointer_window;
  struct wl_display *display;
  struct wl_registry *registry;
  struct wl_compositor *compositor;
  struct xdg_wm_base *xdg_shell;
  struct wl_shm *shm;
  uint32_t formats;
  struct wl_seat *seat;
  struct wl_pointer *pointer;
  struct wl_keyboard *keyboard;
  struct wl_shm_listener shm_listener;
  struct wl_seat_listener seat_listener;
  struct wl_pointer_listener pointer_listener;
  struct wl_keyboard_listener keyboard_listener;
  struct wl_registry_listener registry_listener;
  struct {
    struct xkb_context *context;
    struct xkb_keymap *keymap;
    struct xkb_state *state;
    xkb_mod_mask_t control_mask;
    xkb_mod_mask_t alt_mask;
    xkb_mod_mask_t shift_mask;
  } xkb;
  kr_wayland_info *info;
  kr_adapter *adapter;
};

static void handle_shm_format(void *data, struct wl_shm *wl_shm, uint32_t format);
static void handle_global(void *data, struct wl_registry *registry,
 uint32_t id, const char *interface, uint32_t version);
static int first_frame_timeout(kr_event *timeout);
static int output_frame(kr_wayland_path *window, int num);
static int request_frame(kr_wayland_path *window);
static void handle_frame_done(void *ptr, struct wl_callback *cb, uint32_t time);
static int handle_frame(kr_av_event *event);
static void handle_disconnect(kr_wayland *kw);
static void handle_connect(kr_wayland *kw);
static void kw_init(kr_wayland *kw);
static void kw_connect(kr_wayland *kw);
static void kw_teardown(kr_wayland *kw);
static int handle_in(kr_wayland *kw);
static int handle_out(kr_wayland *kw);
static int handle_event(kr_event *fd_event);

static void
xdg_surface_handle_configure(void *data,
			     struct xdg_surface *xdg_surface,
			     uint32_t serial)
{

  printk("Wayland: xdg_surface_handle_configure");
  kr_wayland_path *window = data;
	xdg_surface_ack_configure(window->xdg_surface, serial);

	/*if (window->state_changed_handler)
		window->state_changed_handler(window, window->user_data);

	window_uninhibit_redraw(window);*/
}

static const struct xdg_surface_listener xdg_surface_listener = {
	xdg_surface_handle_configure
};

static void
xdg_toplevel_handle_configure(void *data, struct xdg_toplevel *xdg_toplevel,
			      int32_t width, int32_t height,
			      struct wl_array *states)
{
  /*
  struct window *window = data;
	uint32_t *p;

	window->maximized = 0;
	window->fullscreen = 0;
	window->resizing = 0;
	window->focused = 0;

	wl_array_for_each(p, states) {
		uint32_t state = *p;
		switch (state) {
		case XDG_TOPLEVEL_STATE_MAXIMIZED:
			window->maximized = 1;
			break;
		case XDG_TOPLEVEL_STATE_FULLSCREEN:
			window->fullscreen = 1;
			break;
		case XDG_TOPLEVEL_STATE_RESIZING:
			window->resizing = 1;
			break;
		case XDG_TOPLEVEL_STATE_ACTIVATED:
			window->focused = 1;
			break;
		default:
			break;
		}
	}

	if (window->frame) {
		if (window->maximized) {
			frame_set_flag(window->frame->frame, FRAME_FLAG_MAXIMIZED);
		} else {
			frame_unset_flag(window->frame->frame, FRAME_FLAG_MAXIMIZED);
		}

		if (window->focused) {
			frame_set_flag(window->frame->frame, FRAME_FLAG_ACTIVE);
		} else {
			frame_unset_flag(window->frame->frame, FRAME_FLAG_ACTIVE);
		}
	}

	if (width > 0 && height > 0) {
		int margin = window_get_shadow_margin(window);

		window_schedule_resize(window,
				       width + margin * 2,
				       height + margin * 2);
	} else if (window->saved_allocation.width > 0 &&
		   window->saved_allocation.height > 0) {
		window_schedule_resize(window,
				       window->saved_allocation.width,
				       window->saved_allocation.height);
	}
 */
}

static void
xdg_toplevel_handle_close(void *data, struct xdg_toplevel *xdg_surface)
{
}

static void
xdg_toplevel_handle_configure_bounds(void *data, struct xdg_toplevel *xdg_toplevel,
				     int32_t width, int32_t height)
{
}


static void
xdg_toplevel_handle_wm_capabilities(void *data, struct xdg_toplevel *xdg_toplevel,
				    struct wl_array *caps)
{
}

static const struct xdg_toplevel_listener xdg_toplevel_listener = {
	xdg_toplevel_handle_configure,
	xdg_toplevel_handle_close,
	xdg_toplevel_handle_configure_bounds,
	xdg_toplevel_handle_wm_capabilities,
};

#include "input.c"

static void handle_shm_format(void *data, struct wl_shm *wl_shm, uint32_t format) {
  kr_wayland *kw;
  kw = (kr_wayland *)data;
  kw->formats |= (1 << format);
  printk("Wayland: shm format %#010x", format);
}

static void
xdg_wm_base_ping(void *data, struct xdg_wm_base *shell, uint32_t serial)
{
	xdg_wm_base_pong(shell, serial);
}

static const struct xdg_wm_base_listener based_listener = {
	xdg_wm_base_ping,
};

static void handle_global(void *data, struct wl_registry *registry,
 uint32_t id, const char *interface, uint32_t version) {
  kr_wayland *kw;
  kw = (kr_wayland *)data;
  printk("Wayland: global %s", interface);
  if (strcmp(interface, "wl_compositor") == 0) {
    kw->compositor = wl_registry_bind(kw->registry, id,
     &wl_compositor_interface, 1);
  } else if (strcmp(interface, "xdg_wm_base") == 0) {
    kw->xdg_shell = wl_registry_bind(kw->registry, id, &xdg_wm_base_interface, MIN(version, 5));
    xdg_wm_base_add_listener(kw->xdg_shell, &based_listener, kw);
  } else if (strcmp(interface, "wl_seat") == 0) {
    kw->seat = wl_registry_bind(kw->registry, id, &wl_seat_interface, 1);
    wl_seat_add_listener(kw->seat, &kw->seat_listener, kw);
  } else if (strcmp(interface, "wl_shm") == 0) {
    kw->shm = wl_registry_bind(kw->registry, id, &wl_shm_interface, 1);
    wl_shm_add_listener(kw->shm, &kw->shm_listener, kw);
  }
}

static int first_frame_timeout(kr_event *timeout) {
  int ret;
  kr_wayland_path *window;
  window = (kr_wayland_path *) timeout->user;
  printk("Wayland: first frame timeout");
  if (window->nframes == 0) {
    printk("Wayland: No frame from compositor in %d ms", KR_WL_FIRST_FRAME_TIMEOUT_MS);
    output_frame(window, 1);
  }
  ret = kr_loop_close(window->wayland->adapter->loop, timeout->fd);
  if (ret != 0) {
    printke("Wayland: problem closing first frame timerfd");
    return -1;
  }
  return 0;
}

static int output_frame(kr_wayland_path *window, int num) {
  if ((num < 0) || (num >= KR_WL_NFRAMES)) return -1;
  wl_surface_attach(window->surface, window->buffer[num], 0, 0);
  wl_surface_damage(window->surface, 0, 0, window->info->width, window->info->height);
  wl_surface_commit(window->surface);
  wl_display_flush(window->wayland->display);
  return 0;
}

static int request_frame(kr_wayland_path *window) {
  int ret;
  void *user;
  kr_frame *frame;
  user = window->adapter_path->user;
  frame = &window->frames[0];
  printk("Wayland: request frame");
  ret = window->adapter_path->write(user, (kr_av *)frame);
  printk("Wayland: request frame wrote");
  if (ret != 1) {
    printke("Wayland: request frame write ret %d", ret);
  }
  return ret;
}

static void handle_frame_done(void *ptr, struct wl_callback *cb, uint32_t time) {
  printk("Wayland: frame done");
  kr_wayland_path *window;
  window = (kr_wayland_path *)ptr;
  wl_callback_destroy(cb);
  request_frame(window);
}

static int handle_frame(kr_av_event *event) {
  kr_wayland_path *window;
  printk("Wayland: write frame");
  window = (kr_wayland_path *)event->frame->image.owner;
  if (window == NULL) {
    printke("Wayland: handle_frame with no window");
    return -1;
  }
  window->nframes++;
  window->frame_cb = wl_surface_frame(window->surface);
  window->frame_listener.done = handle_frame_done;
  wl_callback_add_listener(window->frame_cb, &window->frame_listener, window);
  output_frame(window, 0);
  return 0;
}

static int user_event(kr_event *event) {
  if (event == NULL) return -1;
  kr_wayland_path *window;
  int ret;
  kr_frame frame;
  window = (kr_wayland_path *)event->user;
  printk("Wayland: user event");
  if (event->events & EPOLLIN) {
    ret = window->adapter_path->read(window->adapter_path->user, (kr_av *)&frame);
    printk("Wayland: user event read ret %d", ret);
    if (ret != 1) {
      printke("Wayland: problem reading frame back");
      return -1;
    }
    return ret;
  } else {
    printke("Wayland: Got an unreadable user event");
  }
  return 0;
}

static int handle_out(kr_wayland *kw) {
  int ret;
  //printk("Wayland: handle out");
  while (wl_display_prepare_read(kw->display) != 0) {
    //printk("Wayland: dispatch pending");
    ret = wl_display_dispatch_pending(kw->display);
    if (ret == -1) {
      printke("Wayland: Error on dispatch pending");
    }
  }
  //printk("Wayland: pre flush");
  ret = wl_display_flush(kw->display);
  //printk("Wayland: post flush");
  if ((ret == -1) && (errno != EAGAIN)) {
    ret = errno;
    printke("Wayland: Error on display flush: %s", strerror(ret));
    return -1;
  }
  /* Can poll now */
  //printk("Wayland: done handle out");
  return 0;
}

static int handle_in(kr_wayland *kw) {
  int ret;
  //printk("Wayland: handle_in");
  ret = wl_display_read_events(kw->display);
  if (ret == -1) {
    ret = errno;
    printke("Wayland: Error on read events: %s", strerror(ret));
    return -1;
  }
  ret = wl_display_dispatch_pending(kw->display);
  if (ret == -1) {
    printke("Wayland: Error on dispatch pending");
    return -1;
  }
  return 0;
}

static int handle_event(kr_event *fd_event) {
  if (fd_event == NULL) return -1;
  kr_wayland *kw;
  //printk("Wayland: handle_event");
  int ret;
  ret = 0;
  kw = (kr_wayland *)fd_event->user;
  if (fd_event->events & EPOLLIN) {
    ret = handle_in(kw);
    if (ret == 0) {
      ret = handle_out(kw);
    }
  }
  if (fd_event->events & EPOLLERR || fd_event->events & EPOLLHUP) {
    handle_disconnect(kw);
  }
  return ret;
}

static void handle_disconnect(kr_wayland *kw) {
  printk("Wayland: handle disconnect");
  kw->info->state = KR_WL_DISCONNECTED;
  kr_loop_del(kw->adapter->loop, kw->fd);
  kw->fd = -1;
  kw->adapter->fd = -1;
  kw_teardown(kw);
}

static void handle_connect(kr_wayland *kw) {
  printk("Wayland: handle_connect");
  kr_event harness;
  kw->fd = wl_display_get_fd(kw->display);
  kw->xkb.context = xkb_context_new(0);
  kw->registry = wl_display_get_registry(kw->display);
  wl_registry_add_listener(kw->registry, &kw->registry_listener, kw);
  wl_display_roundtrip(kw->display);
  kw->adapter->fd = kw->fd;
  handle_out(kw);
  harness.user = kw;
  harness.fd = kw->fd;
  harness.handler = handle_event;
  harness.events = EPOLLIN;
  kr_loop_add(kw->adapter->loop, &harness);
}

static void kw_teardown(kr_wayland *kw) {
  printk("Wayland: teardown");
  if (kw == NULL) return;
  if (kw->pointer != NULL) {
    wl_pointer_destroy(kw->pointer);
    kw->pointer = NULL;
  }
  if (kw->keyboard != NULL) {
    wl_keyboard_destroy(kw->keyboard);
    kw->keyboard = NULL;
  }
  if (kw->xkb.state) {
    xkb_state_unref(kw->xkb.state);
    kw->xkb.state = NULL;
  }
  if (kw->xkb.keymap) {
    xkb_map_unref(kw->xkb.keymap);
    kw->xkb.keymap = NULL;
  }
  if (kw->seat) {
    wl_seat_destroy(kw->seat);
    kw->seat = NULL;
  }
  if (kw->xkb.context) {
    xkb_context_unref(kw->xkb.context);
    kw->xkb.context = NULL;
  }
  if (kw->shm) {
    wl_shm_destroy(kw->shm);
    kw->shm = NULL;
  }
  if (kw->xdg_shell) {
    xdg_wm_base_destroy(kw->xdg_shell);
    kw->xdg_shell = NULL;
  }
  if (kw->compositor) {
    wl_compositor_destroy(kw->compositor);
    kw->compositor = NULL;
  }
  if (kw->registry) {
    wl_registry_destroy(kw->registry);
    kw->registry = NULL;
  }
  if (kw->display) {
    wl_display_disconnect(kw->display);
    kw->display = NULL;
  }
  kw->info->state = KR_WL_DISCONNECTED;
}

static void kw_connect(kr_wayland *kw) {
  printk("Wayland: handle_event");
  char *display_name;
  display_name = NULL;
  if (kw->info->state == KR_WL_CONNECTED) return;
  kw_init(kw);
  if (strnlen(kw->info->display_name, sizeof(kw->info->display_name))) {
    display_name = kw->info->display_name;
  }
  kw->display = wl_display_connect(display_name);
  if (display_name == NULL) {
    display_name = "default";
  }
  if (kw->display == NULL) {
    kw->info->state = KR_WL_DISCONNECTED;
    printke("Wayland: Unable to connect to %s display", display_name);
    return;
  }
  kw->info->state = KR_WL_CONNECTED;
  printk("Wayland: Connected to %s display", display_name);
  handle_connect(kw);
}

static void kw_init(kr_wayland *kw) {
  kw->formats = 0;
  kw->pointer_listener.enter = pointer_handle_enter;
  kw->pointer_listener.leave = pointer_handle_leave;
  kw->pointer_listener.motion = pointer_handle_motion;
  kw->pointer_listener.button = pointer_handle_button;
  kw->pointer_listener.axis = pointer_handle_axis;
  kw->keyboard_listener.keymap = keyboard_handle_keymap;
  kw->keyboard_listener.enter = keyboard_handle_enter;
  kw->keyboard_listener.leave = keyboard_handle_leave;
  kw->keyboard_listener.key = keyboard_handle_key;
  kw->keyboard_listener.modifiers = keyboard_handle_modifiers;
  kw->seat_listener.capabilities = handle_seat_capabilities;
  kw->shm_listener.format = handle_shm_format;
  kw->registry_listener.global = handle_global;
}

int kr_wl_lctl(kr_adapter_path *path, kr_patchset *patchset) {
  if (path == NULL) return -1;
  if (patchset == NULL) return -2;
  printk("Wayland: window control");
  return 0;
}

int kr_wl_unlink(kr_adapter_path *path) {
  int i;
  kr_wayland_path *window;
  kr_wayland *kw;
  if (path == NULL) return -1;
  printk("FIXME xdg destroy Wayland: window remove");
  window = (kr_wayland_path *)path->handle;
  kw = window->wayland;
  for (i = 0; i < KR_WL_MAX_WINDOWS; i++) {
    if (&kw->window[i] == window) {
      break;
    }
  }
  xdg_toplevel_destroy(window->xdg_toplevel);
  xdg_surface_destroy(window->xdg_surface);
  wl_surface_destroy(window->surface);
  /*
  FIXME this should be done on a callback from a sync
  for (i = 0; i < KR_WL_NIMAGES; i++) {
    wl_buffer_destroy(window->buffer[i]);
  }
  kr_pool_destroy(window->pool);
  kw->window[i].active = 0;
  */
  //wl_display_roundtrip(kw->display);
  return 0;
}

int window_input(void *u, kr_wayland_event *e) {
  kr_wayland_path *win = u;
  switch (e->type) {
    case KR_WL_KEY:
      printk("Got key %d", e->key_event.key);
      break;
    case KR_WL_POINTER:
      break;
    default:
      printk("Got unhandled window input event");
      break;
  }
  return 0;
}

int kr_wl_link(kr_adapter *adapter, kr_adapter_path *path) {
  printk("Wayland: new link");
  kr_wayland *kw;
  kr_wayland_path *window;
  kr_wayland_path_info *info;
  struct wl_region *opaque;
  struct wl_shm_pool *pool;
  kr_frame frame;
  kr_image *image;
  kr_event harness;
  int i;
  if (adapter == NULL) return -1;
  if (path == NULL) return -2;
  kw = (kr_wayland *)adapter->handle;
  /* FIXME make me better */
  if (kw->info->state != KR_WL_CONNECTED) return -3;
  info = &path->info->wl_out;
  if ((info->width == 0) || (info->height == 0)
   || (info->width > 8192) || (info->height > 8192)) {
    printke("Wayland: window too big");
    return -2;
  }
  for (i = 0; i < KR_WL_MAX_WINDOWS; i++) {
    if (kw->window[i].active == 0) {
      break;
    }
  }
  if (i == KR_WL_MAX_WINDOWS) {
    return -1;
  }
  window = &kw->window[i];
  window->wayland = kw;
  window->info = &path->info->wl_out;
  window->user_callback = window_input;
  window->user = path->user;
  window->nframes = 0;
  memset(&frame, 0, sizeof(frame));
  frame.type = KR_IMAGE_RAW;
  image = &frame.image;
  image->info.w = info->width;
  image->info.h = info->height;
  image->info.fmt = KR_PIXELS_ARGB;
  image->ps[0] = image->info.w * 4;
  window->pool = kr_image_pool_create(image, 2);
  if (window->pool == NULL) {
    printke("Wayland: error creating kr frame pool");
    return -1;
  }
  printk("Wayland: created image pool");
  pool = wl_shm_create_pool(window->wayland->shm, kr_pool_fd(window->pool),
   kr_pool_size(window->pool));
  if (pool == NULL) {
    printke("Wayland: error creating wl_shm_pool");
    return -1;
  }
  printk("Wayland: created shm pool");
  for (i = 0; i < KR_WL_NFRAMES; i++) {
    printk("Wayland: creating frame %d", i);
    image->px[0] = kr_pool_slice(window->pool);
    image->owner = window;
    image->ready = handle_frame;
    int offset = kr_pool_offsetof(window->pool, image->px[0]);
    //printk("offset is %d", offset);
    window->buffer[i] = wl_shm_pool_create_buffer(pool, offset, image->info.w,
      image->info.h, image->ps[0], WL_SHM_FORMAT_XRGB8888);
    window->frames[i] = frame;
    if (window->buffer[i] == NULL) {
      printke("Wayland: error creating wl buffer from wl shm pool");
    }
  }
  printk("Wayland: pool to destroy?");
  wl_shm_pool_destroy(pool);
  printk("Wayland: pool destroy");

  printk("Wayland: create surface");
  window->surface = wl_compositor_create_surface(kw->compositor);
  window->xdg_surface = xdg_wm_base_get_xdg_surface(kw->xdg_shell, window->surface);
  window->xdg_toplevel = xdg_surface_get_toplevel(window->xdg_surface);
  xdg_surface_add_listener(window->xdg_surface, &xdg_surface_listener, window);
  xdg_toplevel_add_listener(window->xdg_toplevel, &xdg_toplevel_listener, window);

  wl_surface_commit(window->surface);

  wl_display_roundtrip(kw->display);
  printk("Wayland: create region");
  opaque = wl_compositor_create_region(kw->compositor);
  wl_display_roundtrip(kw->display);
  printk("Wayland: set op reg");
  wl_region_add(opaque, 0, 0, window->info->width, window->info->height);
  wl_surface_set_opaque_region(window->surface, opaque);
  wl_region_destroy(opaque);
  wl_display_roundtrip(kw->display);
  printk("Wayland: region destroy");

  window->active = 1;
  path->handle = window;
  window->adapter_path = path;
  harness.user = window;
  harness.fd = path->fd;
  harness.handler = user_event;
  harness.events = EPOLLIN;
  printk("Wayland: loop adding regular");
  kr_loop_add(adapter->loop, &harness);
  printk("Wayland: loop adding timeout");
  kr_loop_add_timeout(adapter->loop, KR_WL_FIRST_FRAME_TIMEOUT_MS,
    first_frame_timeout, window);
  request_frame(window);
  printk("Wayland: Window created");
  return 0;
}

int kr_wl_ctl(kr_adapter *adp, kr_patchset *patchset) {
  if (adp == NULL) return -1;
  if (patchset == NULL) return -2;
  printk("Wayland: Adapter controlled");
  return 0;
}

int kr_wl_close(kr_adapter *adapter) {
  kr_wayland *kw;
  int ret;
  if (adapter == NULL) return -1;
  printk("Wayland: Adapter Closing");
  kw = (kr_wayland *)adapter->handle;
  if (kw->display) {
    wl_display_flush(kw->display);
  }
  kw_teardown(kw);
  ret = kr_loop_destroy(adapter->loop);
  if (ret != 0) {
    printke("Wayland: trouble unlooping");
  }
  free(kw);
  adapter->handle = NULL;
  printk("Wayland: Adapter Closed");
  return 0;
}

int kr_wl_open(kr_adapter *adapter) {
  if (adapter == NULL) return -1;
  printk("Wayland: Adapter opening");
  kr_wayland *kw;
  kr_loop_setup loop_setup;
  if (adapter->handle) {
    printk("Wayland: Adapter re-opened");
  } else {
    adapter->handle = kr_allocz(1, sizeof(kr_wayland));
    kr_loop_setup_init(&loop_setup);
    snprintf(loop_setup.name, sizeof(loop_setup.name), "kr_wayland");
    adapter->loop = kr_loop_create(&loop_setup);
    kw = (kr_wayland *)adapter->handle;
    kw->adapter = adapter;
    kw->info = &adapter->info->wl;
    kw->info->state = KR_WL_DISCONNECTED;
    printk("Wayland: Adapter opened");
  }
  kw_connect((kr_wayland *)adapter->handle);
  return 0;
}

#define TEST_WINDOWS 1
#define NUM_VECTORS 4

typedef struct kr_wayland_test kr_wayland_test;
typedef struct kr_wayland_test_window kr_wayland_test_window;

struct kr_wayland_test_window {
  kr_wayland_path *window;
  kr_wayland_path_info info;
  void *buffer;
  char name[32];
  kr_vector *vector;
  int pointer_in;
  int pointer_x;
  int pointer_y;
  int rotin;
  int delayor;
};

struct kr_wayland_test {
  kr_wayland *wayland;
  kr_wayland_test_window windows[TEST_WINDOWS];
};

static int destroy = 0;

void signal_recv (int sig) {
  destroy = 1;
}

int kr_wl_test_frame_cb(void *user, kr_wayland_event *event) {

  kr_wayland_test_window *wayland_test_window;
  int updated;
  cairo_surface_t *cst;
  cairo_t *cr;
  float rot;

  wayland_test_window = (kr_wayland_test_window *)user;

  updated = 0;

  if (wayland_test_window == NULL) {
    /* Its bad */
    return -1;
  }

  cst = cairo_image_surface_create_for_data ((unsigned char *)event->frame_event.buffer,
                         CAIRO_FORMAT_ARGB32,
                         wayland_test_window->info.width,
                         wayland_test_window->info.height,
                         wayland_test_window->info.width * 4);

  cr = cairo_create (cst);

  cairo_save (cr);
  cairo_set_source_rgba (cr, BGCOLOR_CLR);
  cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
  cairo_paint (cr);
  cairo_restore (cr);


  if (((wayland_test_window->vector[2].subunit.x - wayland_test_window->vector[0].subunit.x) < 200) == 0) {

    uint32_t *p;
    int i;
    int end;
    int offset;
    int time;

    time = rand();
    p = (uint32_t *)event->frame_event.buffer;
    end = wayland_test_window->info.width * wayland_test_window->info.height;
    offset = time >> 4;
    for (i = 0; i < end; i++) {
      p[i] = (i + offset) * 0x0080401;

      //if ((wayland_test_window->vector[0].subunit.x % 3) == 0) {
      //  offset = time >> 5;
      //}

    }

  } else {

    cairo_save (cr);
    cairo_set_source_rgba (cr, BGCOLOR_CLR);
    cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
    cairo_paint (cr);
    cairo_restore (cr);


  }

  if ((wayland_test_window->pointer_in == 1) && (wayland_test_window->rotin)) {

    if (wayland_test_window->delayor > 0) {
      krad_compositor_subunit_set_xy(&wayland_test_window->vector[3].subunit,
       wayland_test_window->pointer_x + 6, wayland_test_window->pointer_y + 7, wayland_test_window->delayor);
      wayland_test_window->delayor = 0;
    }

    kr_vector_render(&wayland_test_window->vector[3], cr);

  }

  if (wayland_test_window->pointer_in == 1) {
    krad_compositor_subunit_set_xy(&wayland_test_window->vector[0].subunit,
     wayland_test_window->pointer_x, wayland_test_window->pointer_y, 0);
    if (wayland_test_window->rotin) {
      rot = wayland_test_window->vector[2].subunit.rotation += 7.0;
      krad_compositor_subunit_set_rotation(&wayland_test_window->vector[2].subunit,
       rot, 0);
      cairo_save(cr);
      cairo_translate(cr, wayland_test_window->vector[2].subunit.x, wayland_test_window->vector[0].subunit.y);
      cairo_translate(cr, wayland_test_window->pointer_x / 2, wayland_test_window->pointer_y / 2);
      cairo_rotate(cr, rot - 9.0 * (M_PI/180.0));
      cairo_translate(cr, wayland_test_window->pointer_x / -2, wayland_test_window->pointer_y / -2);
      cairo_translate(cr, wayland_test_window->vector[0].subunit.x * -1, wayland_test_window->vector[2].subunit.x * -1);
    }
    kr_vector_render(&wayland_test_window->vector[0], cr);
    kr_vector_render(&wayland_test_window->vector[2], cr);
    if ((!wayland_test_window->rotin) && (wayland_test_window->delayor > 0)) {
      krad_compositor_subunit_set_xy(&wayland_test_window->vector[2].subunit,
       wayland_test_window->pointer_x + 6, wayland_test_window->pointer_y + 7, wayland_test_window->delayor);
      wayland_test_window->delayor = 0;
    }
    if (wayland_test_window->rotin) {
     wayland_test_window->delayor = rand() % 240;
      wayland_test_window->delayor--;
      cairo_restore(cr);
    }
  }

  kr_vector_render(&wayland_test_window->vector[1], cr);

  cairo_surface_flush (cst);
  cairo_destroy (cr);
  cairo_surface_destroy (cst);

  updated = 1;
  return updated;
}

int kr_wl_test_pointer_cb(void *user, kr_wayland_event *event) {

  kr_wayland_test_window *wayland_test_window;

  if (user == NULL) {
    return -1;
  }

  wayland_test_window = (kr_wayland_test_window *)user;

  if (event->pointer_event.pointer_in) {
  //  printf("pointer in\n");
    wayland_test_window->pointer_in = 1;
  }

  if (event->pointer_event.pointer_out) {
  //  printf("pointer out\n");
    wayland_test_window->pointer_in = 0;
    return 0;
  }

 // printf("pointer event: %s: %dx%d - click: %d\n", wayland_test_window->name,
 //  event->pointer_event.x, event->pointer_event.y, event->pointer_event.click);

  wayland_test_window->pointer_x = event->pointer_event.x;
  wayland_test_window->pointer_y = event->pointer_event.y;

  if (event->pointer_event.click) {
    krad_compositor_subunit_set_xy(&wayland_test_window->vector[1].subunit,
     wayland_test_window->pointer_x, wayland_test_window->pointer_y, 20);
  }

  return 0;
}

int kr_wl_test_key_cb(void *user, kr_wayland_event *event) {

  kr_wayland_test_window *wayland_test_window;
  float rot;

  if (user == NULL) {
    return -1;
  }

  wayland_test_window = (kr_wayland_test_window *)user;

 // printf("key event: %s: %d - %d %c\n", wayland_test_window->name,
 //  event->key_event.down, event->key_event.key, event->key_event.key);

  if (event->key_event.key == 'q') {
    destroy = 1;
  }
  if (event->key_event.key == 'c') {
    kr_vector_type_set(&wayland_test_window->vector[0], "circle");
  }
  if (event->key_event.key == 'x') {
    kr_vector_type_set(&wayland_test_window->vector[0], "hex");
  }
  if (event->key_event.key == 't') {
    kr_vector_type_set(&wayland_test_window->vector[0], "triangle");
  }
  if (event->key_event.key == 'a') {
    rot = wayland_test_window->vector[0].subunit.rotation -= 6.0;
    krad_compositor_subunit_set_rotation(&wayland_test_window->vector[0].subunit, rot, 6);
  }
  if (event->key_event.key == 'd') {
    rot = wayland_test_window->vector[0].subunit.rotation += 46.0;
    krad_compositor_subunit_set_rotation(&wayland_test_window->vector[0].subunit, rot, 56);
  }

  if ((event->key_event.key == 'r') && (event->key_event.down)) {
    wayland_test_window->rotin = 1;
  }
  if ((event->key_event.key == 'r') && (!event->key_event.down)) {
    wayland_test_window->rotin = 0;
  }

  return 0;
}

int kr_wl_test_cb(void *user, kr_wayland_event *event) {
  switch (event->type) {
    case KR_WL_FRAME:
      return kr_wl_test_frame_cb(user, event);
    case KR_WL_POINTER:
      return kr_wl_test_pointer_cb(user, event);
    case KR_WL_KEY:
      return kr_wl_test_key_cb(user, event);
  }
  return 0;
}

void wayland_test_loop(kr_wayland_test *wayland_test) {

  int count;
  int ret;
  count = 0;

  while (!destroy) {
    ret = kr_wayland_process(wayland_test->wayland);
    if (ret < 0) {
      break;
    }
    count++;
  }
}

void wayland_test_destroy (kr_wayland_test *wayland_test) {

  int i;
  int ret;

  for (i = 0; i < TEST_WINDOWS; i++) {
    printf("Destroying window %d\n", i);
    ret = kr_wayland_unlink(&wayland_test->windows[i].window);
    if (ret < 0) {
      fprintf(stderr, "Could not destroy window %dx%d\n",
       wayland_test->windows[i].info.width, wayland_test->windows[i].info.height);
      exit(1);
    }
    kr_vectors_free(wayland_test->windows[i].vector, NUM_VECTORS);
  }

  kr_wayland_destroy(&wayland_test->wayland);
  free(wayland_test);
}

kr_wayland_test *wayland_test_create(int width, int height) {

  int i;
  kr_wayland_test *wayland_test;
  kr_wayland_path_setup window_params;

  wayland_test = calloc(1, sizeof(kr_wayland_test));

  wayland_test->wayland = kr_wayland_create(NULL);

  if (wayland_test->wayland == NULL) {
    fprintf(stderr, "Could not connect to wayland\n");
    exit(1);
  }

  signal (SIGINT, signal_recv);
  signal (SIGTERM, signal_recv);

  for (i = 0; i < TEST_WINDOWS; i++) {
    wayland_test->windows[i].info.width = width;
    wayland_test->windows[i].info.height = height;
    window_params.info = wayland_test->windows[i].info;
    window_params.user = &wayland_test->windows[i];
    window_params.callback = kr_wl_test_cb;

    wayland_test->windows[i].vector = kr_vectors_create(NUM_VECTORS);
    kr_vector_type_set(&wayland_test->windows[i].vector[0], "hex");
    kr_vector_type_set(&wayland_test->windows[i].vector[1], "viper");
    kr_vector_type_set(&wayland_test->windows[i].vector[2], "triangle");
    kr_vector_type_set(&wayland_test->windows[i].vector[3], "hex");
    krad_compositor_subunit_set_red(&wayland_test->windows[i].vector[0].subunit, 0.2, 799);
    krad_compositor_subunit_set_blue(&wayland_test->windows[i].vector[0].subunit, 0.5, 555);

    krad_compositor_subunit_set_green(&wayland_test->windows[i].vector[2].subunit, 0.2, 199);
    krad_compositor_subunit_set_blue(&wayland_test->windows[i].vector[2].subunit, 0.2, 255);

    krad_compositor_subunit_set_green(&wayland_test->windows[i].vector[3].subunit, 0.4, 199);
    krad_compositor_subunit_set_red(&wayland_test->windows[i].vector[3].subunit, 0.2, 255);

    wayland_test->windows[i].window =
     kr_wayland_mkpath(wayland_test->wayland, &window_params);
    if (wayland_test->windows[i].window == NULL) {
      fprintf(stderr, "Could not create window %dx%d\n", width, height);
      exit(1);
    }
    snprintf(wayland_test->windows[i].name,
     sizeof(wayland_test->windows[i].name), "Window Num %d", i);
    width = width + width;
    height = height + height;
  }

  return wayland_test;
}

void wayland_test_run(int width, int height) {

  kr_wayland_test *wayland_test;

  wayland_test = wayland_test_create(width, height);
  wayland_test_loop(wayland_test);
  wayland_test_destroy(wayland_test);
}

int wmain (int argc, char *argv[]) {

  int width;
  int height;

  width = 1280;
  height = 720;

  if (argc == 3) {
    wayland_test_run(atoi(argv[1]), atoi(argv[2]));
  } else {
    wayland_test_run(width, height);
  }

  return 0;
}

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

#define W 500
#define H 500

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
        if (rt == -1) { rt = y; rl = x; }z  
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

int main(int argc, char *argv[]) {
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
/*      if (steps == 742) printf("742	 %10lu: %4lu steps #[%d]\n", i, steps, log[steps]);
      if (steps == 800) printf("800 %10lu: %4lu steps #[%d]\n", i, steps, log[steps]);
      if (steps == 803) printf("803 %10lu: %4lu steps #[%d]\n", i, steps, log[steps]);            
 */     i++;
    }
    for (i = 0; i < 1000; i++) {
      if (!log[i]) continue;
    //  printf("%10lu: %3d\n", i, log[i]);
    }
  }
  return 1;
}

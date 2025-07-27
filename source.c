#include "header.h"

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


#define KR_WL_MAX_WINDOWS 4
#define KR_WL_NFRAMES 2
#define KR_WL_FIRST_FRAME_TIMEOUT_MS 2601

#include "x.h"

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

static void handle_seat_capabilities(void *data, struct wl_seat *seat,
 enum wl_seat_capability caps);
static void keyboard_handle_modifiers(void *data, struct wl_keyboard *keyboard,
 uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched,
 uint32_t mods_locked, uint32_t group);
static void keyboard_handle_keymap(void *data, struct wl_keyboard *keyboard,
 uint32_t format, int fd, uint32_t size);
static void keyboard_handle_enter(void *data, struct wl_keyboard *keyboard,
 uint32_t serial, struct wl_surface *surface, struct wl_array *keys);
static void keyboard_handle_leave(void *data, struct wl_keyboard *keyboard,
 uint32_t serial, struct wl_surface *surface);
static void keyboard_handle_key(void *data, struct wl_keyboard *keyboard,
 uint32_t serial, uint32_t time, uint32_t key, uint32_t state_w);
static void pointer_handle_enter(void *data,
 struct wl_pointer *pointer, uint32_t serial, struct wl_surface *surface,
 wl_fixed_t sx_w, wl_fixed_t sy_w);
static void pointer_handle_leave(void *data,
 struct wl_pointer *pointer, uint32_t serial, struct wl_surface *surface);
static void pointer_handle_motion(void *data,
 struct wl_pointer *pointer, uint32_t time, wl_fixed_t sx_w, wl_fixed_t sy_w);
static void pointer_handle_button(void *data,
 struct wl_pointer *pointer, uint32_t serial, uint32_t time, uint32_t button,
 uint32_t state_w);
static void pointer_handle_axis(void *data,
 struct wl_pointer *pointer, uint32_t time, uint32_t axis, wl_fixed_t value);

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


int main(int argc, char **argv) {
  if (argc > 1) printf("%d retarded arguments.\n", argc - 1);
  int R;
  int MFD;
  unsigned long SZ = 4205260800;
  unsigned char *DAT;

  if ((MFD = memfd_create("pixmap-framebuffer", 0)) < 0) return 1;
  do { R = ftruncate(MFD, SZ); } while (R < 0 && errno == EINTR);
	DAT = mmap(NULL, SZ, PROT_READ | PROT_WRITE, MAP_SHARED, MFD, 0);
	if (!DAT) return 13*13;
	printf("ya mmap DAT %p + 4205260800, nice!\n", DAT);


  mpz_t n;
  mpz_init(n);
  mpz_clear(n);
  pw_init(&argc, &argv);
  printf("Cairo: %s\n", cairo_version_string());
  printf("GMP: %s\n", gmp_version);
  printf("Pipewire %s/%s ", pw_get_headers_version(), pw_get_library_version());






  /* *C */
  return 2601;
}

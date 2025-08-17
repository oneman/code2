#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/epoll.h>
#include <linux/types.h>
#include <linux/input.h>
#include <linux/hidraw.h>

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
  memset(out, 0, sizeof(out));
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

int main(int argc, char **argv) {
  int id = 0;
  int sz = 0;
  int ret = 0;
  int keyboard = 0;
  int mouse = 0;
  struct hidraw_report_descriptor rpt;
  char *dev = "/dev/hidraw0";
  memset(&rpt, 0, sizeof(rpt));
  if (argc > 1) dev = argv[1];
  id = open(dev, O_RDONLY | O_NONBLOCK);
  if (id < 0) {
    perror("Unable to open device");
    return 1;
  }
  ret = ioctl(id, HIDIOCGRDESCSIZE, &sz);
  if (ret < 0) {
    perror("HIDIOCGRDESCSIZE");
    return 1;
  }
  if (sz < 4) {
    perror("wtf");
    return 1;
  }
  rpt.size = sz;
  ret = ioctl(id, HIDIOCGRDESC, &rpt);
  if (ret < 0) {
    perror("HIDIOCGRDESC");
    return 1;
  }
  unsigned char *s = rpt.value;
  if ((s[0] != 5) || (s[1] != 1) || (s[2] != 9)
   || ((s[3] != 2) && (s[3] != 6))) {
    perror("not the right kind of input");
    return 1;
  }
  if (s[3] == 2) mouse = 1;
  if (s[3] == 6) keyboard = 1;
  int ed;
  struct epoll_event ev;
  struct epoll_event events[1];
  memset(&ev, 0, sizeof(ev));
  ed = epoll_create1(EPOLL_CLOEXEC);
  if (ed < 0) {
    perror("Unable to create epoll");
    return 1;
  }
  ev.events = EPOLLIN;
  ev.data.fd = id;
  ret = epoll_ctl(ed, EPOLL_CTL_ADD, id, &ev);
  if (ret == -1) {
    perror("Unable to ctl epoll");
    return 1;
  }
  for (;;) {
    ret = epoll_wait(ed, events, 1, -1);
    if (ret != 1) {
      perror("epoll_wait problem");
      return 1;
    }
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
      /*
      write(1, out, 16);
      write(1, "\n", 1);
      printf("%d %d %d %d %d %d %d %d\n",
       scanx(out + 0), scanx(out + 2),
       scanx(out + 4), scanx(out + 6),
       scanx(out + 8), scanx(out + 10),
       scanx(out + 12), scanx(out + 14));
      printmod(b);
      */
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
  }
  /* yeah bye */
  close(id);
  close(ed);
  return 0;
}

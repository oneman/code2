#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/syscall.h>
#include <sys/utsname.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <err.h>
#include <errno.h>
#include <time.h>
#include <fts.h>
#include <krad/io/file2.h>
#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE ((26 * 26) + 26)
#endif
#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <locale.h>
#include <sys/uio.h>

#define HIGH 248

#define ASSERT(_e, ...) if (!(_e)) { fprintf(stderr, __VA_ARGS__); exit(1); }

#include <krad/radio/client.h>

#include "font.c"
/*#include "doc.c"*/
#include <cairo/cairo.h>
#include "pix.c"
#include "text.c"

/*
*  gcc cairo_sandbox.c -o cairo_sandbox `pkg-config --libs --cflags cairo`
*/

typedef struct {
  int x;
  int y;
} point;

typedef struct {
  point pt[8];
  int n;
} points;

#include "data.c"
#include "files.c"
#include "config.c"

int su() { return setreuid(0, 0) + setregid(0, 0); }

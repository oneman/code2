#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftmodapi.h>

  void ft_fail(int ft_errno) {
    printf("Freetype Error: %s\n", FT_Error_String(ft_errno));
    exit(1);
  }

  void *ft_alloc(FT_Memory ft_mem, long sz) {
    void *buf;
    static size_t tb = 0;
    tb += sz;
    printf("Freetype: alloc %ld bytes. [Total %ld]\n", sz, tb);
    buf = malloc(sz);
    memset(buf, 0, sz);
    return buf;
  }

  void ft_free(FT_Memory ft_mem, void *buf) {
    printf("Freetype: free\n");
    free(buf);
  }

  void *ft_realloc(FT_Memory ft_mem, long old_sz, long new_sz, void *buf) {
    void *newbuf;
    printf("Freetype: realloc %ld -> %ld bytes\n", old_sz, new_sz);
    newbuf = malloc(new_sz);
    memset(newbuf, 0, new_sz);
    memcpy(newbuf, buf, old_sz);
    free(buf);
    return newbuf;
  }

void ft_test(uint8_t *buf, size_t sz) {
  int ft_errno = 0;
  FT_Open_Args ft_args;
  FT_Library ft_lib = NULL;
  FT_Face f = NULL;
  struct FT_MemoryRec_ ft_mem;

  memset(&ft_args, 0, sizeof(ft_args));
  memset(&ft_lib, 0, sizeof(ft_lib));
  memset(&f, 0, sizeof(f));

  ft_mem.user = "demo";
  ft_mem.alloc = ft_alloc;
  ft_mem.free = ft_free;
  ft_mem.realloc = ft_realloc;

  ft_errno = FT_New_Library(&ft_mem, &ft_lib);
  if (ft_errno) ft_fail(ft_errno);
  printf("ft new lib\n");

  FT_Add_Default_Modules(ft_lib);
  printf("ft add def mod\n");
  FT_Set_Default_Properties(ft_lib);
  printf("ft set def prop\n");
  ft_args.flags = FT_OPEN_MEMORY;
  ft_args.memory_base = buf;
  ft_args.memory_size = sz;
  printf("ft pre open face: %p \n", ft_lib);
  ft_errno = FT_Open_Face(ft_lib, &ft_args, 0, &f);
  if (ft_errno) ft_fail(ft_errno);
  printf("ft open face\n");

  int pxl_w = 12 * 10;
  int pxl_h = pxl_w;
  int w = pxl_w * 64;
  int h = pxl_h * 64;
  int dpi = 720;

  ft_errno = FT_Set_Char_Size(f, w, h, dpi, dpi);
  if (ft_errno) ft_fail(ft_errno);
  printf("ft set char size\n");

  ft_errno = FT_Set_Pixel_Sizes(f, pxl_w, pxl_h);
  if (ft_errno) ft_fail(ft_errno);
  printf("ft set px size\n");

  uint32_t char_code = 'a';
  ft_errno = FT_Load_Char(f, char_code, FT_LOAD_DEFAULT);
  if (ft_errno) ft_fail(ft_errno);
  printf("ft load char\n");

  ft_errno = FT_Render_Glyph(f->glyph, FT_RENDER_MODE_NORMAL);
  if (ft_errno) ft_fail(ft_errno);
  printf("ft render glyph\n");

  FT_Bitmap pxmap;
  pxmap = f->glyph->bitmap;
  printf("ft bitmap: %u x %u stride: %d grays: %u mode:  %u\n", pxmap.width,
    pxmap.rows, pxmap.pitch, pxmap.num_grays, pxmap.pixel_mode);

  ft_errno = FT_Done_Face(f);
  if (ft_errno) ft_fail(ft_errno);
  printf("ft done face\n");

  ft_errno = FT_Done_Library(ft_lib);
  if (ft_errno) ft_fail(ft_errno);
  printf("ft done lib\n");
}



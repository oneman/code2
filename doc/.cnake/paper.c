#define letterpaper_quarter_inch_ptx 18
#define letterpaper_half_inch_ptx 36
#define letterpaper_inch_ptx 72
#define letterpaper_height_ptx 792
#define letterpaper_width_ptx 612
#define letterpaper_gov_margin 72
#define letterpaper_nonmarginal_width 468 /* perm sym */
#define letterpaper_nonmarginal_height 648 /* perm sym */
/* 648 / 468 = 1.384615 */
#define letterpaper_nonmarginal_quad_ruled_columns 26
#define letterpaper_nonmarginal_quad_ruled_rows 36
/* 36 / 26 = 1.384615 */
#define lpm letterpaper_gov_margin
#define gs_diam 26 * 18
#define gs_rad 13 * 18
#define gs_center lpm + gs_rad

#define crossword_sz "4 and 7/16in wide"

/* content area: 6.5 * 9 = 58.5 in 484704 */
/* letterpaper area: 58.5 + 35 = 93.5 in */
/* letterpaper total quanta = 93.5 * 5184 */
/* margin area: (11 * 2) + (13 * 1) = 35in */
/* margin quanta: 5184 * 35 = 181440 */
/* margin overlapping quanta: 5184 * 4 = 20736 */
/* margin nonoverlapping quanta: 160704 */
/* margin nonoverlapping wide quanta: 67392 */
/* margin nonoverlapping tall quanta: 93312 */
/* margin nonoverlapping ratio: 93312/67392 = 1.384615 */
/* dollar size: 6.2 * 2.6 */

int mom(int argc, char *argv[]) {
  cairo_surface_t *surface;
  cairo_t *cr;
  time_t seconds;
  seconds = time(NULL);
  surface = NULL;
  cr = NULL;
  char filename[128];
  snprintf(filename, sizeof(filename), "%s/mom_%ld.png", getenv("HOME"),
    seconds);
  int w = 320;
  int h = 320;
  surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h);
  cr = cairo_create(surface);
  //unsigned char *px;
  //px = cairo_image_surface_get_data(surface);
  /* paper */
  cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
  cairo_paint(cr);
  /* crossword grid */
  cairo_set_line_width(cr, 1);
  cairo_set_source_rgb(cr, 0, 0, 0);
  int i;
  for (i = 0; i < 16; i++) {
    cairo_move_to(cr, i * 20, 0);
    cairo_line_to(cr, i * 20, 320);
    cairo_stroke(cr);
    cairo_move_to(cr, 0, i * 20);
    cairo_line_to(cr, 320, i * 20);
    cairo_stroke(cr);
  }
  cairo_surface_write_to_png(surface, filename);
  cairo_destroy(cr);
  cairo_surface_destroy(surface);
  return 0;
}

int tryman(int argc, char *argv[]) {
  cairo_surface_t *surface;
  cairo_t *cr;
  time_t seconds;
  seconds = time(NULL);
  surface = NULL;
  cr = NULL;
  char filename[128];
  snprintf(filename, sizeof(filename), "%s/lush_%ld.png", getenv("HOME"),
    seconds);
  int w = 3840;
  int h = 2160;
  surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h);
  cr = cairo_create(surface);
  unsigned char *px;
  px = cairo_image_surface_get_data(surface);

  /* paper */
  cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
  cairo_paint(cr);

  cairo_set_line_width(cr, 0.5);
  cairo_set_source_rgb(cr, 0, 0, 0);
  int i;
  for (i = 0; i < 16; i++) {
    cairo_move_to(cr, i * 20, 0);
    cairo_line_to(cr, i * 20, 320);
    cairo_stroke(cr);
    cairo_move_to(cr, 0, i * 20);
    cairo_line_to(cr, 320, i * 20);
    cairo_stroke(cr);
  }

  // quad rule
  cairo_set_source_rgba(cr, 0.26, 0.26, 0.99, 1);
  int c = 1;
  int r = 1;
  for (c = 1; c < 34; c++) {
    cairo_move_to(cr, c * 18, 0);
    cairo_line_to(cr, c * 18, 792);
    cairo_stroke(cr);
  }
  for (r = 1; r < 44; r++) {
    cairo_move_to(cr, 0, r * 18);
    cairo_line_to(cr, 612, r * 18);
    cairo_stroke(cr);
  }

  /* golden margin */
  cairo_set_line_width(cr, 2);
  cairo_set_source_rgba(cr, 1.0, 0.83, 0, 1);
  cairo_move_to(cr, 72, 72);
  cairo_line_to(cr, 540, 72);
  cairo_line_to(cr, 540, 720);
  cairo_line_to(cr, 72, 720);
  cairo_close_path(cr);
  cairo_stroke(cr);

  cairo_new_path(cr);
  cairo_set_source_rgba(cr, 1.0, 0.84, 0.13, 0.71);
  cairo_rectangle(cr, 72, 72, 18 * 26, 18 * 26);
  cairo_fill(cr);

  cairo_new_path(cr);
  cairo_set_source_rgba(cr, 1.0, 0.13, 0.66, 0.2);
  cairo_rectangle(cr, 72, 18 * 30, 18 * 16, 18 * 10);
  cairo_fill(cr);

  cairo_new_path(cr);
  cairo_set_source_rgba(cr, 1.0, 0.53, 0.66, 0.4);
  cairo_rectangle(cr, (4 * 18) + (13 * 18), (4 * 18),
          (13 * 18), (5 * 18));
  cairo_fill(cr);

  /*
  cairo_new_path(cr);
  cairo_set_line_width (cr, 2.6);
  cairo_set_source_rgba(cr, 0.1, 0.3, 0.2, 1);
  cairo_arc (cr, gs_center, gs_center, gs_rad, 0, (3.1 / 180.0) * 360.0);
  cairo_stroke (cr);
  cairo_fill(cr);


  cairo_new_path(cr);
  cairo_set_line_width (cr, 2.6);
  cairo_set_source_rgba(cr, 1, 0.3, 0.2, 1);
  cairo_arc (cr, gs_center, gs_center, gs_rad, (3.1415926 / 180.0) * 90, (3.1415926 / 180.0) * 150.0);
  double x, y = 0;
  cairo_get_current_point(cr, &x, &y);
  printf("x,y: %f,%f\n", x, y);
  cairo_stroke (cr);
  cairo_fill(cr);

  cairo_new_path(cr);
  cairo_set_source_rgba(cr, 1, 0.1, 0.1, 1);
  cairo_arc(cr, x, y, 9, 0, 3.1415926 / 180.0 * 360);
  cairo_fill(cr);


  cairo_new_path(cr);
  cairo_set_line_width (cr, 1.6);
  cairo_set_source_rgba(cr, 0.3, 0.2, 0.1, 1);
  cairo_move_to(cr, x, y);
  cairo_line_to(cr, letterpaper_width_ptx - x, y);
  cairo_line_to(cr, letterpaper_width_ptx/2, letterpaper_gov_margin);
  cairo_close_path(cr);
  cairo_stroke (cr);


  cairo_new_path(cr);
  cairo_set_line_width (cr, 1.6);
  cairo_set_source_rgba(cr, 0.3, 0.6, 0.6, 1);
  cairo_move_to(cr, x, y);
  cairo_line_to(cr, letterpaper_width_ptx/2, letterpaper_gov_margin + (26 * 18));
  cairo_line_to(cr, letterpaper_width_ptx -x, y);


  cairo_close_path(cr);
  cairo_stroke (cr);


  cairo_new_path(cr);
  cairo_set_line_width (cr, 2.6);
  cairo_set_source_rgba(cr, 0.3, 0.5, 0.4, 1);
  cairo_arc (cr, gs_center, gs_center + gs_rad, gs_rad,
   0,
   (3.1415926 / 180.0) * 270.0);
  cairo_stroke (cr);
  cairo_fill(cr);

  cairo_new_path(cr);
  cairo_set_line_width (cr, 2.6);
  cairo_set_source_rgba(cr, 0.7, 0.2, 0.6, 1);
  cairo_arc (cr, gs_center, gs_center + gs_rad, gs_rad,
   (3.1415926 / 30.0) * 70.0, 0);
  cairo_stroke (cr);
  cairo_fill(cr);
  */

  cairo_surface_write_to_png(surface, filename);

  cairo_destroy(cr);
  cairo_surface_destroy(surface);
  return 0;
}

int color_block(int argc, char *argv[]) {
  cairo_surface_t *surface;
  cairo_t *cr;
  time_t seconds;
  seconds = time(NULL);
  surface = NULL;
  cr = NULL;
  char filename[128];
  snprintf(filename, sizeof(filename), "%s/lush_%ld.png", getenv("HOME"),
    seconds);
  int w = 4096;
  int h = 4096;
  surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, w, h);
  cr = cairo_create(surface);
  unsigned char *px;
  px = cairo_image_surface_get_data(surface);

  cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
  cairo_paint(cr);

  cairo_new_path(cr);
  cairo_set_source_rgb(cr, 1, 0, 0);
  cairo_rectangle(cr, 26, 26, 100, 100);
  cairo_fill(cr);

  cairo_surface_flush(surface);
  u8 a = 0;
  u8 b = 0;
  u8 c = 0;
  for (int i = 0; i < (w * h); i++) {
    int n = i;
    char *aa = (char *)&n;
    px[0] = aa[0];
    px[1] = aa[1];
    px[2] = aa[2];
    /*a++;
    if (a == 255) {
      b++;
      a = 0;
    }
    if (b == 255) {
      c++;
      b = 0;
    }*/
    px = px + 4;
  }
  cairo_surface_mark_dirty(surface);
  cairo_new_path(cr);
  cairo_set_source_rgb(cr, 0.53, 0.66, 0.4);
  cairo_rectangle(cr, (14 * 18) + (26 * 18), (4 * 18),
      (18 * 18), (15 * 18));
  cairo_fill(cr);

  cairo_new_path(cr);
  cairo_set_source_rgb(cr, 0, 1, 0);
  cairo_rectangle(cr, 260, 260, 100, 100);
  cairo_fill(cr);

  cairo_surface_flush(surface);
  px = cairo_image_surface_get_data(surface);
  for (int i = 0; i < (w * h); i++) {
    //px[(i * 4) + 2] = 128;
    int n = i;
    //px[1] = n >> 8;
    //px[0] = 55;
    //px[3] = n >> 8;
    px += 4;
  }
  cairo_surface_mark_dirty(surface);

  cairo_surface_write_to_png(surface, filename);
  cairo_destroy(cr);
  cairo_surface_destroy(surface);
  return 0;
}

int paper(int argc, char *argv[]) {
  return color_block(argc, argv);
  cairo_surface_t *surface;
  cairo_t *cr;
  time_t seconds;
  seconds = time(NULL);
  surface = NULL;
  cr = NULL;
  char filename[128];
  snprintf(filename, sizeof(filename), "%s/lush_%ld.png", getenv("HOME"),
    seconds);
  int w = 612;
  int h = 792;
  surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h);
  cr = cairo_create(surface);
  cairo_set_line_width(cr, 0.5);
  unsigned char *px;
  px = cairo_image_surface_get_data(surface);
  cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
  cairo_paint(cr);

  /* has paper */




  cairo_surface_write_to_png(surface, filename);
  cairo_destroy(cr);
  cairo_surface_destroy(surface);
  return 0;
}

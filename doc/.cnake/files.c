#include <krad/io/dir.h>

int file_scan(char *path, size_t sz) {
  uint8_t *data;
  kr_file2 *file;
  kr_file2_info info;
  kr_fs_setup setup;
  kr_file_set *fs;
  sz = strlen(path);
  if (!kr_file2_exists(path, sz)) {
    printf("Doesn't exist %s\n", path);
    return 1;
  }
  setup.nfiles = 1;
  setup.user = NULL;
  setup.event_cb = NULL;
  fs = kr_file_set_create(&setup);
  if (!fs) {
    printf("this fs didn't create\n");
    return 1;
  }
  file = kr_file2_open(fs, path, sz);
  if (!file) {
    printf("Can't open %s\n", path);
    return kr_file_set_destroy(fs);
  }
  data = kr_file2_get_data(file);
  if (data && !kr_file2_get_info(file, &info)) {
    printf("%s\n", path);
    data_scan(data, info.sz);
  }
  printf("\n\n");
  return kr_file_set_destroy(fs);
}

int path_scan(char *path, size_t sz) {
  int ret;
  FTS *fs;
  FTSENT *n;
  char *paths[] = { path, NULL };
  fs = fts_open(paths, FTS_PHYSICAL, NULL);
  if (!fs) return 1;
  for (;;) {
    n = fts_read(fs);
    if (!n) break;
    if (n->fts_info == FTS_F) file_scan(n->fts_path, strlen(n->fts_path));
  }
  ret = fts_close(fs);
  return ret;
}

int home_scan() {
  kr_dir_create("/home/demo/1");
  char *h = "/home/demo/selected";
  if (!kr_dir_exists(h)) h = "/home";
  return path_scan(h, strlen(h));
}

int cwd_scan() {
  static char cwd_path[4096];
  char *cwd = getcwd(cwd_path, sizeof(cwd_path));
  if (!cwd) exit(1);
  return path_scan(cwd, strlen(cwd));
}

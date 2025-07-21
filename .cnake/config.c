#include <krad/io/dir.h>

typedef struct {
  u8 *data;
  u64 sz;
  kr_file_set *fs;
} program_config;

static program_config global_program_config;
static program_config *C = &global_program_config;

void dump_config() {
  printf("%.*s", C->sz, C->data);
}

void reconfigure() {
  char config_path[256];
  memset(config_path, 0, sizeof(config_path));
  char *homedir = getenv("HOME");
  if (!homedir) {
    printf("No HOME in ENV Wtf?!\n");
    exit(1);
  }
  u64 sz = snprintf(config_path, sizeof(config_path) - 1, "%s/config", homedir);
  u8 *path = config_path;
  u8 *data;
  kr_file2 *file;
  kr_file2_info info;
  kr_fs_setup setup;
  sz = strlen(path);
  if (!kr_file2_exists(path, sz)) {
    printf("Doesn't exist %s\n", path);
    exit(1);
  }
  setup.nfiles = 1;
  setup.user = NULL;
  setup.event_cb = NULL;
  C->fs = kr_file_set_create(&setup);
  if (!C->fs) {
    printf("this fs didn't create\n");
    exit(1);
  }
  file = kr_file2_open(C->fs, path, sz);
  if (!file) {
    printf("Can't open %s\n", path);
    kr_file_set_destroy(C->fs);
    exit(1);
  }
  data = kr_file2_get_data(file);
  if (data && !kr_file2_get_info(file, &info)) {
    C->data = data;
    C->sz = info.sz;
  } else {
    printf("Config failure!\n");
    exit(1);
  }
}

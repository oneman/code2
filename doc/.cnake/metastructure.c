#include "stdio.h"
#include "inttypes.h"
#include "string.h"

#define FILENAME_MAX_LEN 64
#define NAME_MAX_LEN 48
#define MEMBER_MAX 48
#define DEF_MAX_LEN 4096

typedef float f32;
typedef double f64;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef size_t usize;
typedef ssize_t isize;

typedef struct struct_info struct_info;
typedef struct struct_data struct_data;
typedef struct member_info member_info;

typedef struct char_member_info char_member_info;
typedef struct int8_member_info int8_member_info;
typedef struct int16_member_info int16_member_info;
typedef struct int32_member_info int32_member_info;
typedef struct int64_member_info int64_member_info;
typedef struct uint8_member_info uint8_member_info;
typedef struct uint16_member_info uint16_member_info;
typedef struct uint32_member_info uint32_member_info;
typedef struct uint64_member_info uint64_member_info;
typedef struct float_member_info float_member_info;
typedef struct double_member_info double_member_info;
typedef struct struct_member_info struct_member_info;
typedef struct anon_member_info anon_member_info;

struct char_member_info {
  int32_t max;
  int notnull;
};

struct int8_member_info {
  int8_t init;
  int8_t min;
  int8_t max;
};

struct int16_member_info {
  int16_t init;
  int16_t min;
  int16_t max;
};

struct int32_member_info {
  int32_t init;
  int32_t min;
  int32_t max;
};

struct int64_member_info {
  int64_t init;
  int64_t min;
  int64_t max;
};

struct uint8_member_info {
  uint8_t init;
  uint8_t min;
  uint8_t max;
};

struct uint16_member_info {
  uint16_t init;
  uint16_t min;
  uint16_t max;
};

struct uint32_member_info {
  uint32_t init;
  uint32_t min;
  uint32_t max;
};

struct uint64_member_info {
  uint64_t init;
  uint64_t min;
  uint64_t max;
};

struct float_member_info {
  float init;
  float min;
  float max;
};

struct double_member_info {
  double init;
  double min;
  double max;
};

struct struct_member_info {
  char type_name[NAME_MAX_LEN];
  char enum_init[NAME_MAX_LEN];
  int idx;
};

struct anon_member_info {
  int depth;
  struct_info *info;
  int idx;
};

typedef enum {
  T_CHAR = 1,
  T_INT8,
  T_INT16,
  T_INT32,
  T_INT64,
  T_UINT8,
  T_UINT16,
  T_UINT32,
  T_UINT64,
  T_FLOAT,
  T_DOUBLE,
  T_STRUCT,
  T_UNION,
  T_ENUM,
  T_ASTRUCT,
  T_AUNION,
  T_ANON
} member_type;

typedef enum {
  ST_ENUM = 1,
  ST_STRUCT,
  ST_UNION,
  ST_ASTRUCT,
  ST_AUNION
} struct_type;

typedef union {
  char_member_info char_info;
  int8_member_info int8_info;
  int16_member_info int16_info;
  int32_member_info int32_info;
  int64_member_info int64_info;
  uint8_member_info uint8_info;
  uint16_member_info uint16_info;
  uint32_member_info uint32_info;
  uint64_member_info uint64_info;
  float_member_info float_info;
  double_member_info double_info;
  struct_member_info substruct_info;
  struct_member_info subunion_info;
  struct_member_info subenum_info;
  anon_member_info anon_info;
} member_type_info;

struct member_info {
  char name[NAME_MAX_LEN];
  int arr;
  int arr_notnull;
  char len_def[32];
  int ptr;
  size_t off;
  size_t sz;
  int opt;
  member_type type;
  member_type_info type_info;
};

struct struct_info {
  struct_type type;
  char name[NAME_MAX_LEN];
  int member_count;
  member_info members[MEMBER_MAX];
  union {
    size_t sz;
    int root_idx;
  };
  int is_typedef;
};

struct struct_data {
  char filename[FILENAME_MAX_LEN];
  int line_number;
  char definition[DEF_MAX_LEN];
  struct_info info;
};

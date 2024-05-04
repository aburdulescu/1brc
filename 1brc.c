#include <assert.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

typedef struct {
  char* ptr;
  size_t len;
} String;

typedef struct {
  int16_t mul;
  int16_t val;
} TempTableEntry;

static const TempTableEntry temp_table[256] = {
    {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},
    {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},
    {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},
    {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},
    {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},
    {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},
    {10, 0}, {10, 1}, {10, 2}, {10, 3}, {10, 4}, {10, 5}, {10, 6}, {10, 7},
    {10, 8}, {10, 9}, {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},
    {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},
    {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},
    {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},
    {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},
    {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},
    {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},
    {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},
    {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},
    {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},
    {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},
    {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},
    {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},
    {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},
    {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},
    {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},
    {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},
    {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},
    {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},
    {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},
    {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},
    {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},
    {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},
    {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},
    {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},  {1, 0},
};

static int16_t tempToInt(String temp) {
  int16_t result = 0;
  for (size_t i = 0; i < temp.len; ++i) {
    const uint8_t c = temp.ptr[i];
    result *= temp_table[c].mul;
    result += temp_table[c].val;
  }
  const bool is_negative = temp.ptr[0] == '-';
  return is_negative ? (-1 * result) : result;
}

static char* printableCity(String city) {
  static char s[101];
  memcpy(s, city.ptr, city.len);
  s[city.len] = 0;
  return s;
}

#define MAX_CITIES 10000

typedef struct {
  String cities[MAX_CITIES];
  int64_t sums[MAX_CITIES];
  int16_t mins[MAX_CITIES];
  int16_t maxs[MAX_CITIES];
} Database;

void updateDatabase(Database* db, String city, uint32_t city_hash,
                    int16_t temp) {
  const size_t pos = city_hash % MAX_CITIES;
  if (db->cities[pos].len == 0) db->cities[pos] = city;
  if (temp > db->maxs[pos]) db->maxs[pos] = temp;
  if (temp < db->mins[pos]) db->mins[pos] = temp;
  db->sums[pos] += temp;
}

static bool parseCity(String* l, String* city, uint32_t* city_hash) {
  // FNV-1a 32 bit
  static const uint32_t fnv1a_init_32 = 2166136261;
  static const uint32_t fnv1a_prime_32 = 16777619;

  // find separator and also hash the city
  char* sep = NULL;
  uint32_t h = fnv1a_init_32;
  for (size_t i = 0; i < l->len; ++i) {
    if (l->ptr[i] == ';') {
      sep = l->ptr + i;
      break;
    }
    h ^= l->ptr[i];
    h *= fnv1a_prime_32;
  }
  if (sep == NULL) {
    return false;
  }

  // set city
  city->ptr = l->ptr;
  city->len = sep - l->ptr;

  // set hash
  *city_hash = h;

  // skip
  l->ptr = sep + 1;
  l->len -= city->len + 1;

  return true;
}

static bool parseTemp(String* l, int16_t* temp) {
  // find newline
  char* nl = memchr(l->ptr, '\n', l->len);
  if (nl == NULL) {
    return false;
  }

  String temp_str = {
      .ptr = l->ptr,
      .len = nl - l->ptr,
  };

  *temp = tempToInt(temp_str);

  // skip
  l->ptr = nl + 1;
  l->len -= temp_str.len + 1;

  return true;
}

static bool parseLine(String* l, Database* db) {
  String city = {};
  uint32_t city_hash = 0;
  if (!parseCity(l, &city, &city_hash)) return false;

  int16_t temp = 0;
  if (!parseTemp(l, &temp)) return false;

  updateDatabase(db, city, city_hash, temp);

  return true;
}

static bool mmapFile(const char* path, String* out) {
  struct stat st = {};
  if (stat(path, &st) == -1) {
    perror("stat");
    return false;
  }

  int fd = open(path, O_RDONLY);
  if (fd == -1) {
    perror("open");
    return NULL;
  }

  void* addr = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
  if (addr == MAP_FAILED) {
    perror("mmap");
    goto fd_close;
  }

  out->ptr = addr;
  out->len = st.st_size;

fd_close:
  close(fd);

  return true;
}

static void test_parseTemp() {
  typedef struct {
    char* input;
    int16_t expected;
  } Test;

  Test tests[] = {
      {"0.0", 0},    {"1.1", 11},   {"-1.1", -11},
      {"12.3", 123}, {"99.9", 999}, {"-99.9", -999},
  };

  for (size_t i = 0; i < sizeof(tests) / sizeof(Test); i++) {
    const int16_t v = tempToInt((String){
        .ptr = tests[i].input,
        .len = strlen(tests[i].input),
    });
    printf("input=%s, want=%d, have=%d\n", tests[i].input, tests[i].expected,
           v);
    assert(v == tests[i].expected);
  }
}

static void run(String file) {
  Database db = {};
  for (String line = file; parseLine(&line, &db);) {
  }

  for (size_t i = 0; i < MAX_CITIES; i++) {
    if (db.cities[i].len == 0) continue;
    printf("%s %f %f %f\n", printableCity(db.cities[i]), db.mins[i] / 10.0,
           db.maxs[i] / 10.0, db.sums[i] / 10.0);
  }
  fflush(stdout);
}

static void test_NextLine() {
  char s[] =
      "aaaaa;23.2\n"
      "bbbbbbbbbb;-42.3\n";

  String file = {.ptr = s, .len = strlen(s)};

  run(file);
}

static void run_tests() {
  test_NextLine();
  test_parseTemp();
}

int main(int argc, char** argv) {
  if (argc > 1 && strcmp(argv[1], "-t") == 0) {
    run_tests();
    return 0;
  }

  const char* file_path = "measurements.txt";
  if (argc > 1) {
    file_path = argv[1];
  }

  String file = {};
  if (!mmapFile(file_path, &file)) return 1;

  run(file);

  munmap(file.ptr, file.len);

  return 0;
}

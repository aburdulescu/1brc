#include <assert.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
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

static const TempTableEntry tempTable[256] = {
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

static int16_t tempToInt(const char* temp) {
  const bool isNegative = temp[0] == '-';
  int16_t result = 0;
  for (; *temp != '\0'; ++temp) {
    const size_t i = *temp;
    result *= tempTable[i].mul;
    result += tempTable[i].val;
  }
  return isNegative ? (-1 * result) : result;
}

typedef struct {
  char ptr[101];
  size_t len;
} City;

static void SetCity(City* c, String s) {
  c->len = s.len;
  memcpy(c->ptr, s.ptr, c->len);
  c->ptr[c->len] = 0;
}

#define MAX_CITIES 10000

typedef struct {
  int64_t sum;
  int16_t max;
  int16_t min;
} TempStats;

typedef struct {
  City cities[MAX_CITIES];
  TempStats stats[MAX_CITIES];
} Database;

static bool parseCity(String* l, String* city, uint32_t* city_hash) {
  // FNV-1a 32 bit
  static const uint32_t fnv1aInit32 = 2166136261;
  static const uint32_t fnv1aPrime32 = 16777619;

  // find separator and also hash the city
  char* sep = NULL;
  uint32_t h = fnv1aInit32;
  for (size_t i = 0; i < l->len; ++i) {
    if (l->ptr[i] == ';') {
      sep = l->ptr + i;
      break;
    }
    h ^= l->ptr[i];
    h *= fnv1aPrime32;
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

  // temperature string, range [-99.9, 99.9]
  char temp_str[6];
  const size_t temp_len = nl - l->ptr;  // -1 for ;
  memcpy(temp_str, l->ptr, temp_len);
  temp_str[temp_len] = 0;

  *temp = tempToInt(temp_str);

  // skip
  l->ptr = nl + 1;
  l->len -= temp_len + 1;

  return true;
}

static bool parseLine(String* l, Database* db) {
  String city = {};
  uint32_t city_hash = 0;
  if (!parseCity(l, &city, &city_hash)) return false;

  int16_t temp = 0;
  if (!parseTemp(l, &temp)) return false;

  // update database
  const size_t pos = city_hash % MAX_CITIES;
  if (db->cities[pos].len == 0) SetCity(&db->cities[pos], city);
  if (temp > db->stats[pos].max) db->stats[pos].max = temp;
  if (temp < db->stats[pos].min) db->stats[pos].min = temp;
  db->stats[pos].sum += temp;

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
    const char* input;
    int16_t expected;
  } Test;

  Test tests[] = {
      {"0.0", 0},    {"1.1", 11},   {"-1.1", -11},
      {"12.3", 123}, {"99.9", 999}, {"-99.9", -999},
  };

  for (size_t i = 0; i < sizeof(tests) / sizeof(Test); i++) {
    int16_t v = tempToInt(tests[i].input);
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
    printf("%s %f %f %f\n", db.cities[i].ptr, db.stats[i].min / 10.0,
           db.stats[i].max / 10.0, db.stats[i].sum / 10.0);
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

  const char* filePath = "measurements.txt";
  if (argc > 1) {
    filePath = argv[1];
  }

  String file = {};
  if (!mmapFile(filePath, &file)) return 1;

  run(file);

  munmap(file.ptr, file.len);

  return 0;
}

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

static int16_t parseTemp(const char* s) {
  const bool isNegative = s[0] == '-';
  int16_t result = 0;
  for (; *s != '\0'; ++s) {
    const size_t i = *s;
    result *= tempTable[i].mul;
    result += tempTable[i].val;
  }
  return isNegative ? (-1 * result) : result;
}

typedef struct {
  char ptr[101];
  size_t len;
} City;

bool CityEquals(City* l, City* r) {
  if (l->len != r->len) return false;
  return memcmp(l->ptr, r->ptr, l->len) == 0;
}

void CitySet(City* c, const char* ptr, size_t len) {
  c->len = len;
  memcpy(c->ptr, ptr, c->len);
  c->ptr[c->len] = 0;
}

typedef struct {
  char* ptr;
  size_t len;
} String;

// TODO: this is useless, do everything in SplitLine
static bool NextLine(String l, String* out) {
  char* p = memchr(l.ptr, '\n', l.len);
  if (p == NULL) {
    return false;
  }
  out->ptr = l.ptr;
  out->len = p - l.ptr;
  return true;
}

static String AdvanceLine(String l, size_t n) {
  return (String){.ptr = l.ptr + n, .len = l.len - n};
}

typedef struct {
  City city;    // max 100
  double temp;  // [-99.9, 99.9]
} Data;

// FNV-1a 32 bit
static const uint32_t fnv1aInit32 = 2166136261;
static const uint32_t fnv1aPrime32 = 16777619;

static bool SplitLine(String l, Data* d, uint32_t* cityHash) {
  // find separator and also hash the city

  char* sep = NULL;
  uint32_t h = fnv1aInit32;

  for (size_t i = 0; i < l.len; ++i) {
    if (l.ptr[i] == ';') {
      sep = l.ptr + i;
      break;
    }
    h ^= l.ptr[i];
    h *= fnv1aPrime32;
  }
  if (sep == NULL) {
    return false;
  }

  *cityHash = h;

  CitySet(&d->city, l.ptr, sep - l.ptr);

  // range [-99.9, 99.9]
  char temp[6];
  const size_t temp_len = l.len - d->city.len - 1;  // -1 for ;
  memcpy(temp, sep + 1, temp_len);
  temp[temp_len] = 0;

  d->temp = parseTemp(temp);

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

void DatabaseAdd(Database* db, City* city, uint32_t cityHash, float temp) {
  const size_t pos = cityHash % MAX_CITIES;
  if (db->cities[pos].len == 0) CitySet(&db->cities[pos], city->ptr, city->len);
  if (temp > db->stats[pos].max) db->stats[pos].max = temp;
  if (temp < db->stats[pos].min) db->stats[pos].min = temp;
  db->stats[pos].sum += temp;
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
    int16_t v = parseTemp(tests[i].input);
    printf("input=%s, want=%d, have=%d\n", tests[i].input, tests[i].expected,
           v);
    assert(v == tests[i].expected);
  }
}

static void test_NextLine() {
  char s[] =
      "aaaaa;23.2\n"
      "bbbbbbbbbb;-42.3\n";

  String base = {.ptr = s, .len = strlen(s)};
  for (String next = {}; NextLine(base, &next);
       base = AdvanceLine(base, next.len + 1)) {
    printf("base: %p %zu %c\n", base.ptr, base.len, base.ptr[0]);
    printf("next: %p %zu %c\n", base.ptr, next.len, base.ptr[0]);

    Data d = {};
    uint32_t cityHash = 0;
    if (!SplitLine(next, &d, &cityHash)) continue;

    printf("city=%s\n", d.city.ptr);
    printf("temp=%f\n", d.temp);
  }
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

  Database db = {};

  for (String base = file, next = {}; NextLine(base, &next);
       base = AdvanceLine(base, next.len + 1)) {
    Data d = {};
    uint32_t cityHash = 0;
    if (!SplitLine(next, &d, &cityHash)) continue;
    DatabaseAdd(&db, &d.city, cityHash, d.temp);
  }

  for (size_t i = 0; i < MAX_CITIES; i++) {
    if (db.cities[i].len == 0) continue;
    printf("%s %f %f %f\n", db.cities[i].ptr, db.stats[i].min / 10.0,
           db.stats[i].max / 10.0, db.stats[i].sum / 10.0);
  }
  fflush(stdout);

  munmap(file.ptr, file.len);

  return 0;
}

#include <assert.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

static float parseTemp(const char* s) {
  const bool isNegative = s[0] == '-';

  float v = 0;

  if (isNegative) s++;

  // add integer part
  while (*s != '.') {
    v = v * 10 + (*s - '0');
    s++;
  }

  // skip .
  s++;

  // add decimal part
  v = v * 10 + (*s - '0');
  v /= 10;

  return isNegative ? (-1 * v) : v;
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

static bool NextLine(String l, String* out) {
  char* p = strchr(l.ptr, '\n');
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

static bool SplitLine(String l, Data* d) {
  char* p = strchr(l.ptr, ';');
  if (p == NULL) {
    return false;
  }

  CitySet(&d->city, l.ptr, p - l.ptr);

  // range [-99.9, 99.9]
  char temp[6];
  const size_t temp_len = l.len - d->city.len - 1;  // -1 for ;
  memcpy(temp, p + 1, temp_len);
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
  float max;
  float min;
  float sum;
} TempStats;

typedef struct {
  City cities[MAX_CITIES];
  TempStats stats[MAX_CITIES];
  size_t len;
} Database;

void DatabaseAdd(Database* db, City* city, float temp) {
  int pos = -1;
  for (size_t i = 0; i < db->len; i++) {
    if (CityEquals(&db->cities[i], city)) {
      pos = i;
      break;
    }
  }
  if (pos == -1) {
    pos = db->len;
    ++db->len;
    CitySet(&db->cities[pos], city->ptr, city->len);
    db->stats[pos].max = temp;
    db->stats[pos].min = temp;
    db->stats[pos].sum = temp;
  } else {
    if (temp > db->stats[pos].max) db->stats[pos].max = temp;
    if (temp < db->stats[pos].min) db->stats[pos].min = temp;
    db->stats[pos].sum += temp;
  }
}

static void test_parseTemp() {
  typedef struct {
    const char* input;
    float expected;
  } Test;

  Test tests[] = {
      {"0.0", 0.0},   {"1.1", 1.1},   {"-1.1", -1.1},
      {"12.3", 12.3}, {"99.9", 99.9}, {"-99.9", -99.9},
  };

  for (size_t i = 0; i < sizeof(tests) / sizeof(Test); i++) {
    float v = parseTemp(tests[i].input);
    printf("input=%s, want=%f, have=%f\n", tests[i].input, tests[i].expected,
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
    if (!SplitLine(next, &d)) continue;

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
    if (!SplitLine(next, &d)) continue;
    DatabaseAdd(&db, &d.city, d.temp);
  }

  for (size_t i = 0; i < db.len; i++) {
    printf("%s %f %f %f\n", db.cities[i].ptr, db.stats[i].min, db.stats[i].max,
           db.stats[i].sum);
  }
  fflush(stdout);

  munmap(file.ptr, file.len);

  return 0;
}

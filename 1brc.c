#include <assert.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <threads.h>
#include <unistd.h>

typedef struct {
  char* ptr;
  size_t len;
} String;

char* StringPrintable(String s) {
  char* c = malloc(s.len + 1);
  memcpy(c, s.ptr, s.len);
  c[s.len] = 0;
  return c;
}

static char* StringFindChar(String s, char c) {
  return memchr(s.ptr, c, s.len);
}

static char* printableCity(String city) {
  static char s[101];
  memcpy(s, city.ptr, city.len);
  s[city.len] = 0;
  return s;
}

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

#define MAX_CITIES 10000

typedef struct {
  String city;
  int64_t sum;
  uint16_t count;
  int16_t min;
  int16_t max;
} DatabaseEntry;

typedef struct {
  DatabaseEntry entries[MAX_CITIES];
  DatabaseEntry* list[MAX_CITIES];
  size_t list_len;
} Database;

void updateDatabase(Database* db, String city, uint32_t city_hash,
                    int16_t temp) {
  DatabaseEntry* e = &db->entries[city_hash % MAX_CITIES];
  if (e->city.len == 0) {
    e->city = city;
    db->list[db->list_len] = e;
    ++db->list_len;
  }
  if (temp < e->min) e->min = temp;
  if (temp > e->max) e->max = temp;
  e->sum += temp;
  ++e->count;
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

static bool parseLine(String* l, Database* db) {
  String city = {};
  uint32_t city_hash = 0;
  if (!parseCity(l, &city, &city_hash)) return false;

  const int16_t temp = tempToInt(*l);

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

typedef struct {
  String base;
  String iter;
  mtx_t mtx;
} File;

static bool initFile(File* f, String file) {
  if (mtx_init(&f->mtx, mtx_plain) != thrd_success) {
    return false;
  }
  f->base = file;
  f->iter = f->base;
  return true;
}

static bool nextLine(File* f, String* line) {
  if (mtx_lock(&f->mtx) != thrd_success) {
    return false;
  }

  char* p = StringFindChar(f->iter, '\n');
  if (p == NULL) {
    mtx_unlock(&f->mtx);
    return false;
  }

  line->ptr = f->iter.ptr;
  line->len = p - f->iter.ptr;

  f->iter.ptr = p + 1;
  f->iter.len -= line->len + 1;

  return mtx_unlock(&f->mtx) == thrd_success;
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

/* static int citySorter(const void* a, const void* b) { */
/*   const DatabaseEntry* l = *(const DatabaseEntry**)a; */
/*   const DatabaseEntry* r = *(const DatabaseEntry**)b; */

/*   const size_t len = (l->city.len < r->city.len) ? l->city.len : r->city.len;
 */

/*   const int result = memcmp(l->city.ptr, r->city.ptr, len); */

/*   // if common part is the same, put first the smaller one */
/*   if (result == 0) return l->city.len - r->city.len; */

/*   return result; */
/* } */

/* static void run(String file) { */
/*   Database db = {}; */
/*   for (String line = file; parseLine(&line, &db);) { */
/*   } */

/*   qsort(db.list, db.list_len, sizeof(DatabaseEntry*), citySorter); */

/*   for (size_t i = 0; i < db.list_len; ++i) { */
/*     const DatabaseEntry* e = db.list[i]; */
/*     if (e->city.len == 0) continue; */
/*     printf("%s = %f / %f / %f\n", printableCity(e->city), e->min / 10.0, */
/*            (e->sum / 10.0) / e->count, e->max / 10.0); */
/*   } */

/*   fflush(stdout); */
/* } */

typedef struct {
  File* f;
  Database* db;
} WorkerData;

static int worker_entrypoint(void* arg) {
  WorkerData* wd = (WorkerData*)arg;

  for (String line = {}; nextLine(wd->f, &line);) {
    parseLine(&line, wd->db);
  }

  for (size_t i = 0; i < wd->db->list_len; ++i) {
    printf("%s\n", printableCity(wd->db->list[i]->city));
  }

  printf("db len: %zu\n", wd->db->list_len);

  return 0;
}

/* static void test_parseLine() { */
/*   char s[] = */
/*       "aaaaaxxx;23.2\n" */
/*       "aaaaa;23.2\n" */
/*       "bbbbbbbbbb;-42.3\n"; */

/*   String file = {.ptr = s, .len = strlen(s)}; */

/*   run(file); */
/* } */

static void test_worker() {
  char s[] =
      "aaaaaxxx;23.2\n"
      "aaaaa;23.2\n"
      "bbbbbbbbbb;-42.3\n";

  String file = {.ptr = s, .len = strlen(s)};

  File f = {};
  initFile(&f, file);

  Database db = {};

  WorkerData wd = {
      .f = &f,
      .db = &db,
  };

  worker_entrypoint(&wd);
}

static void run_tests() {
  test_parseTemp();
  //  test_parseLine();
  test_worker();
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

  File f = {};
  if (!initFile(&f, file)) return 1;

  const int num_workers = sysconf(_SC_NPROCESSORS_ONLN);

  Database* databases = malloc(sizeof(Database) * num_workers);

  thrd_t* workers = malloc(sizeof(thrd_t) * num_workers);
  for (int i = 0; i < num_workers; ++i) {
    WorkerData* wd = malloc(sizeof(WorkerData));
    wd->f = &f;
    wd->db = &databases[i];

    thrd_create(&workers[i], worker_entrypoint, wd);
  }

  for (int i = 0; i < num_workers; ++i) {
    int res = 0;
    thrd_join(workers[i], &res);
  }

  return 0;
}

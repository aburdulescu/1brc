#include <assert.h>
#include <fcntl.h>
#include <stdatomic.h>
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

static bool StringEmpty(String s) { return s.len == 0 || s.ptr == NULL; }

static String StringFromCstr(char* c) {
  String s = {.ptr = c, .len = strlen(c)};
  return s;
}

static String StringSlice(String s, size_t begin, size_t end) {
  String r = {.ptr = s.ptr + begin, .len = end};
  return r;
}

char* StringToCstr(String s) {
  char* c = malloc(s.len + 1);
  memcpy(c, s.ptr, s.len);
  c[s.len] = 0;
  return c;
}

static char* StringFind(String s, char c) { return memchr(s.ptr, c, s.len); }

static char* StringRfind(String s, char c) {
  for (int i = s.len - 1; i >= 0; --i) {
    if (s.ptr[i] == c) return s.ptr + i;
  }
  return NULL;
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

// FNV-1a 32 bit
static const uint32_t fnv1a_offset_32 = 2166136261;
static const uint32_t fnv1a_prime_32 = 16777619;

typedef uint32_t Hash;

static uint32_t fnv1a(String s) {
  uint32_t h = fnv1a_offset_32;
  for (size_t i = 0; i < s.len; ++i) {
    h ^= s.ptr[i];
    h *= fnv1a_prime_32;
  }
  return h;
}

// max is 10000 but we need a power of 2
#define MAX_CITIES (1 << 14)

typedef struct {
  String city;
  int64_t sum;
  uint64_t count;
  int16_t min;
  int16_t max;
} DatabaseEntry;

typedef struct {
  DatabaseEntry entries[MAX_CITIES];
  DatabaseEntry* list[MAX_CITIES];
  size_t list_len;
} Database;

static bool StringEquals(String s, String other) {
  if (s.len != other.len) return false;
  return memcmp(s.ptr, other.ptr, s.len) == 0;
}

static Hash DatabaseFindSlot(Database* db, const DatabaseEntry* v,
                             Hash city_hash) {
  Hash slot = city_hash & (MAX_CITIES - 1);

  for (; slot < MAX_CITIES; ++slot) {
    const DatabaseEntry* e = &db->entries[slot];

    // found an empty slot
    if (StringEmpty(e->city)) break;

    // found the same city
    if (StringEquals(e->city, v->city)) break;
  }

  assert(slot < MAX_CITIES);

  return slot;
}

static void DatabaseUpdateEntry(Database* db, const DatabaseEntry* v,
                                Hash city_hash) {
  const Hash slot = DatabaseFindSlot(db, v, city_hash);
  DatabaseEntry* e = &db->entries[slot];
  if (StringEmpty(e->city)) {
    // if slot is empty, add the city
    e->city = v->city;
    db->list[db->list_len] = e;
    ++db->list_len;
  }
  if (v->min < e->min) e->min = v->min;
  if (v->max > e->max) e->max = v->max;
  e->sum += v->sum;
  e->count += v->count;
}

static void DatabaseUpdate(Database* db, String city, Hash city_hash,
                           int16_t temp) {
  const DatabaseEntry v = {
      .city = city,
      .sum = temp,
      .count = 1,
      .min = temp,
      .max = temp,
  };
  DatabaseUpdateEntry(db, &v, city_hash);
}

static void DatabaseMerge(Database* db, Database* other) {
  for (size_t i = 0; i < other->list_len; ++i) {
    const Hash city_hash = fnv1a(other->list[i]->city);
    DatabaseUpdateEntry(db, other->list[i], city_hash);
  }
}

static void parseCity(String* l, String* city, Hash* city_hash) {
  // find separator and also hash the city
  char* sep = NULL;
  Hash h = fnv1a_offset_32;
  for (size_t i = 0; i < l->len; ++i) {
    if (l->ptr[i] == ';') {
      sep = l->ptr + i;
      break;
    }
    h ^= l->ptr[i];
    h *= fnv1a_prime_32;
  }

  // set city
  city->ptr = l->ptr;
  city->len = sep - l->ptr;

  // set hash
  *city_hash = h;

  // skip
  l->ptr = sep + 1;
  l->len -= city->len + 1;
}

static void parseTemp(String* l, int16_t* temp) {
  // find newline
  char* nl = StringFind(*l, '\n');

  String temp_str = {
      .ptr = l->ptr,
      .len = nl - l->ptr,
  };

  *temp = tempToInt(temp_str);

  // skip
  l->ptr = nl + 1;
  l->len -= temp_str.len + 1;
}

static void parseLine(String* l, Database* db) {
  String city = {};
  Hash city_hash = 0;
  parseCity(l, &city, &city_hash);

  int16_t temp = 0;
  parseTemp(l, &temp);

  DatabaseUpdate(db, city, city_hash, temp);
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
  String iter;
  size_t chunk_size;
} File;

static bool FileNextChunk(File* f, String* chunk) {
  if (StringEmpty(f->iter)) {
    return false;
  }

  if (f->iter.len < f->chunk_size) {
    *chunk = f->iter;
    f->iter = (String){};
    return true;
  }

  char* p = StringRfind(StringSlice(f->iter, 0, f->chunk_size), '\n');
  if (p == NULL) {
    return false;
  }

  chunk->ptr = f->iter.ptr;
  chunk->len = p - f->iter.ptr + 1;

  f->iter.ptr = p + 1;
  f->iter.len -= chunk->len;

  return true;
}

static int citySorter(const void* a, const void* b) {
  const DatabaseEntry* l = *(const DatabaseEntry**)a;
  const DatabaseEntry* r = *(const DatabaseEntry**)b;

  const size_t len = (l->city.len < r->city.len) ? l->city.len : r->city.len;

  const int result = memcmp(l->city.ptr, r->city.ptr, len);

  // if common part is the same, put first the smaller one
  if (result == 0) return l->city.len - r->city.len;

  return result;
}

static void printDatabaseEntry(const DatabaseEntry* e) {
  printf("%s=%.1f/%.1f/%.1f", printableCity(e->city), e->min / 10.0,
         (e->sum / 10.0) / e->count, e->max / 10.0);
}

static void processDatabase(Database* db) {
  qsort(db->list, db->list_len, sizeof(DatabaseEntry*), citySorter);

  printf("{");

  // print all but last
  for (size_t i = 0; i < db->list_len - 1; ++i) {
    const DatabaseEntry* e = db->list[i];
    printDatabaseEntry(e);
    printf(", ");
  }

  // print last one
  printDatabaseEntry(db->list[db->list_len - 1]);

  printf("}\n");

  printf("len=%zu\n", db->list_len);

  fflush(stdout);
}

typedef struct {
  String* ptr;
  size_t len;
  atomic_uint i;
} Chunks;

static void ChunksInit(Chunks* c, size_t cap) {
  c->ptr = malloc(cap * sizeof(String));
  c->len = 0;
  atomic_init(&c->i, 0);
}

static void ChunksAdd(Chunks* c, String chunk) {
  c->ptr[c->len] = chunk;
  ++c->len;
}

static bool ChunksGet(Chunks* c, String* chunk) {
  unsigned int i = atomic_load(&c->i);
  do {
    if (i == c->len) {
      return false;
    }
    *chunk = c->ptr[i];
  } while (!atomic_compare_exchange_weak(&c->i, &i, i + 1));
  return true;
}

typedef struct {
  Chunks* chunks;
  Database* db;
} WorkerData;

static int worker(void* arg) {
  WorkerData* wd = (WorkerData*)arg;

  for (String chunk = {}; ChunksGet(wd->chunks, &chunk);) {
    while (!StringEmpty(chunk)) parseLine(&chunk, wd->db);
  }

  return 0;
}

static bool run(String file, int num_workers, size_t chunk_size) {
  File f = {.iter = file, .chunk_size = chunk_size};

  Chunks chunks = {};
  ChunksInit(&chunks, (file.len / chunk_size) + 1);

  for (String chunk = {}; FileNextChunk(&f, &chunk);) {
    ChunksAdd(&chunks, chunk);
  }

  Database* databases = malloc(sizeof(Database) * num_workers);

  WorkerData* worker_data = malloc(sizeof(WorkerData) * num_workers);
  for (int i = 0; i < num_workers; ++i) {
    worker_data[i].chunks = &chunks;
    worker_data[i].db = &databases[i];
  }

  thrd_t* workers = malloc(sizeof(thrd_t) * num_workers);
  for (int i = 0; i < num_workers; ++i) {
    thrd_create(&workers[i], worker, &worker_data[i]);
  }

  for (int i = 0; i < num_workers; ++i) {
    int res = 0;
    thrd_join(workers[i], &res);
  }

  Database* db = &databases[0];

  for (int i = 1; i < num_workers; ++i) {
    DatabaseMerge(db, &databases[i]);
  }

  processDatabase(db);

  return true;
}

static void test_StringRfind() {
  {
    char* p = StringRfind(StringFromCstr("abcXdef"), 'X');
    assert(p != NULL);
  }
  {
    char* p = StringRfind(StringFromCstr("abcdefX"), 'X');
    assert(p != NULL);
  }
  {
    char* p = StringRfind(StringFromCstr("abcdef"), 'X');
    assert(p == NULL);
  }
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
    const int16_t v = tempToInt(StringFromCstr(tests[i].input));
    printf("input=%s, want=%d, have=%d\n", tests[i].input, tests[i].expected,
           v);
    assert(v == tests[i].expected);
  }
}

static void test_worker() {
  String file = StringFromCstr(
      "aaaaaxxx;23.2\n"
      "aaaaa;23.2\n"
      "bbbbbbbbbb;-42.3\n"
      "aaaaaxxx;23.2\n"
      "bbbbbbbbbb;-42.3\n"
      "aaaaa;23.2\n"
      "bbbbbbbbbb;-42.3\n"
      "aaaaaxxx;23.2\n"
      "bbbbbbbbbb;-42.3\n"
      "aaaaaxxx;23.2\n");

  assert(run(file, 2, 32));
}

static void run_tests() {
  test_StringRfind();
  test_parseTemp();
  test_worker();
}

int main(int argc, char** argv) {
  if (argc > 1 && strcmp(argv[1], "test") == 0) {
    run_tests();
    return 0;
  }

  const char* file_path = "1brc/measurements.txt";
  if (argc > 1) {
    file_path = argv[1];
  }

  String file = {};
  if (!mmapFile(file_path, &file)) return 1;

  const int num_workers = sysconf(_SC_NPROCESSORS_ONLN);
  const size_t chunk_size = 32 * 1024 * 1024;

  if (!run(file, num_workers, chunk_size)) return 1;

  return 0;
}

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
  char city[101];  // max 100
  char temp[6];    // [-99.9, 99.9]
} Data;

static bool SplitLine(String l, Data* d) {
  char* p = strchr(l.ptr, ';');
  if (p == NULL) {
    return false;
  }

  const size_t city_len = p - l.ptr;
  memcpy(d->city, l.ptr, city_len);
  d->city[city_len] = 0;

  const size_t temp_len = l.len - city_len - 1;  // -1 for ;
  memcpy(d->temp, p + 1, temp_len);
  d->temp[temp_len] = 0;

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

    printf("city=%s\n", d.city);
    printf("temp=%s\n", d.temp);
  }
}

static void run_tests() { test_NextLine(); }

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

  for (String base = file, next = {}; NextLine(base, &next);
       base = AdvanceLine(base, next.len + 1)) {
    Data d = {};
    if (!SplitLine(next, &d)) continue;
  }

  munmap(file.ptr, file.len);

  return 0;
}

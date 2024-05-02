#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef struct {
  char* ptr;
  size_t len;
} Line;

bool NextLine(Line l, Line* out) {
  char* p = strchr(l.ptr, '\n');
  if (p == NULL) {
    return false;
  }
  out->ptr = l.ptr;
  out->len = p - l.ptr;
  return true;
}

Line AdvanceLine(Line l, size_t n) {
  return (Line){.ptr = l.ptr + n, .len = l.len - n};
}

typedef struct {
  char city[100];  // max 100
  char temp[4];    // [-99.9, 99.9]
} Data;

bool SplitLine(Line l, Data* d) {
  char* p = strchr(l.ptr, ';');
  if (p == NULL) {
    return false;
  }

  const size_t city_len = p - l.ptr;
  memcpy(d->city, l.ptr, city_len);
  d->city[city_len] = 0;

  const size_t temp_len = l.len - city_len;
  memcpy(d->temp, p + 1, temp_len);
  d->temp[temp_len] = 0;

  return true;
}

int main() {
  FILE* f = fopen("measurements.txt", "r");

  char buf[8192] = {};

  uint64_t count = 1;

  const uint64_t max = 1000000000;

  for (;;) {
    size_t nread = fread(buf, 1, sizeof(buf), f);
    if (ferror(f)) {
      fprintf(stderr, "error: fread failed\n");
      break;
    }
    if (feof(f)) {
      break;
    }

    buf[nread] = 0;

    Line next = {.ptr = buf, .len = nread};

    for (;;) {
      Line line = {};
      if (!NextLine(next, &line)) {
        break;
      }

      printf("\r%f%% ", ((float)count / (float)max) * 100);
      fflush(stdout);
      count++;

      next = AdvanceLine(next, line.len);
    }
  }

  fclose(f);

  return 0;
}

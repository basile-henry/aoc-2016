#ifndef BAZ_HEADER
#define BAZ_HEADER

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#define private __attribute__((unused)) static

#define panic(...)                                                             \
  do {                                                                         \
    fprintf(stderr, __VA_ARGS__);                                              \
    exit(1);                                                                   \
  } while (0)

#define assert_msg(cond, ...)                                                  \
  do {                                                                         \
    if (!(cond)) {                                                                \
      fprintf(stderr, __VA_ARGS__);                                            \
      exit(1);                                                                 \
    }                                                                          \
  } while (0)

///////////////////////////////////////////////////////////////////////////////
// Int types

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t i8;
typedef int16_t iu16;
typedef int32_t iu32;
typedef int64_t iu64;
typedef size_t usize;

private
inline u8 u64_to_u8(u64 x) {
  assert_msg(x <= UINT8_MAX, "%zd is greater than %d (max u8)\n", x,
             UINT8_MAX);

  return (u8)x;
}

private
inline u16 u64_to_u16(u64 x) {
  assert_msg(x <= UINT16_MAX, "%zd is greater than %d (max u16)\n", x,
             UINT16_MAX);

  return (u16)x;
}

///////////////////////////////////////////////////////////////////////////////
// Hash

// FxHash using this reference:
// https://github.com/rust-lang/rustc-hash/blob/5e09ea0a1c7ab7e4f9e27771f5a0e5a36c58d1bb/src/lib.rs#L79
typedef usize FxHasher;
typedef usize Hash;

#if UINTPTR_MAX == 0xFFFFFFFF
#define K 0x9e3779b9
#elif UINTPTR_MAX == 0xFFFFFFFFFFFFFFFFu
#define K 0x517cc1b727220a95
#else
#error Only 32 and 64 bits archs supported
#endif

private
inline void FxHasher_add(FxHasher *hasher, usize x) {
  *hasher = (*hasher << 5) | (*hasher >> (8 * sizeof(usize) - 5));
  *hasher ^= x;
  *hasher *= K;
}

private
Hash usize_hash(const void *x) {
  FxHasher hasher = {0};
  FxHasher_add(&hasher, *(usize *)x);
  return hasher;
}

////////////////////////////////////////////////////////////////////////////////
// Tuples

#define T2(X, Y)                                                               \
  struct {                                                                     \
    X fst;                                                                     \
    Y snd;                                                                     \
  }

////////////////////////////////////////////////////////////////////////////////
// Optional

#define Option(T)                                                              \
  struct {                                                                     \
    T dat;                                                                     \
    bool valid;                                                                \
  }

////////////////////////////////////////////////////////////////////////////////
// Array

/*
Define a capacity-bounded, but dynamically growing array with element type T
and capacity N.

For example `define_array(WordsArray, Span, 32);` defines the new type
`WordsArray` along with various methods such as `WordsArray_push`,
`WordsArray_pop`, ...
*/
#define define_array(A_NAME, T, N)                                             \
  typedef struct {                                                             \
    usize len;                                                                 \
    T dat[N];                                                                  \
  } A_NAME;                                                                    \
                                                                               \
private                                                                        \
  usize A_NAME##_capacity = N;                                                 \
                                                                               \
private                                                                        \
  T *A_NAME##_push(A_NAME *array, T x) {                                       \
    assert(array->len < N);                                                    \
    T *slot = &array->dat[array->len];                                         \
    *slot = x;                                                                 \
    array->len += 1;                                                           \
    return slot;                                                               \
  }                                                                            \
                                                                               \
  typedef Option(T) A_NAME##Pop;                                               \
private                                                                        \
  A_NAME##Pop A_NAME##_pop(A_NAME *array) {                                    \
    /* Initialise as not valid */                                              \
    A_NAME##Pop ret = {                                                        \
        .valid = false,                                                        \
    };                                                                         \
                                                                               \
    if (array->len == 0) {                                                     \
      return ret;                                                              \
    }                                                                          \
                                                                               \
    array->len -= 1;                                                           \
                                                                               \
    ret.valid = true;                                                          \
    ret.dat = array->dat[array->len];                                          \
    return ret;                                                                \
  }                                                                            \
                                                                               \
  typedef A_NAME A_NAME##_unused_trailing_semicolon_hack

////////////////////////////////////////////////////////////////////////////////
// Span

typedef struct {
  const u8 *dat;
  usize len;
} Span;

private
Span Span_from_str(const char *str) {
  Span ret = {
      .dat = (u8 *)str,
      .len = strlen(str),
  };
  return ret;
}

// Load a file and get a Span to its content
//
// Note: This function leaks a file descriptor and doesn't munmap for you
private
Span Span_from_file(const char *path) {
  int fd = open(path, O_RDONLY);
  assert(fd != -1);

  off_t len = lseek(fd, 0, SEEK_END);
  assert(len >= 0);

  u8 *dat = (u8 *)mmap(0, (size_t)len, PROT_READ, MAP_PRIVATE, fd, 0);
  assert(dat != (void *)-1);

  Span ret = {
      .dat = dat,
      .len = (usize)len,
  };
  return ret;
}

private
void Span_print(Span span) { fwrite(span.dat, 1, span.len, stdout); }

private
Hash Span_hash(const void *span_ptr) {
  Span span = *((Span *)span_ptr);
  FxHasher hasher = {0};

  for (usize i = 0; i < span.len; i++) {
    FxHasher_add(&hasher, (usize)span.dat[i]);
  }

  return hasher;
}

private
bool Span_eq(const void *a_ptr, const void *b_ptr) {
  Span *a = (Span *)a_ptr;
  Span *b = (Span *)b_ptr;

  return a->len == b->len && memcmp(a->dat, b->dat, a->len) == 0;
}

// Slice x[from..to] which is inclusive from and exclusive to
private
Span Span_slice(Span x, usize from, usize to) {
  assert(from <= to);
  assert(to <= x.len);

  if (from == to) {
    Span ret = {0};
    return ret;
  }

  Span ret = {
      .dat = x.dat + from,
      .len = to - from,
  };
  return ret;
}

typedef Option(T2(u64, Span)) SpanParseU64;
private
SpanParseU64 Span_parse_u64(Span x, int base) {
  errno = 0;
  char *end;
  const u64 res = strtoul((char *)x.dat, &end, base);
  assert(errno == 0);

  if (x.dat == (u8 *)end) {
    SpanParseU64 ret = {
        .valid = false,
    };
    return ret;
  }

  SpanParseU64 ret = {
      .dat =
          {
              .fst = res,
              .snd = Span_slice(x, (usize)end - (usize)x.dat, x.len),
          },
      .valid = true,
  };
  return ret;
}

private
inline bool Span_starts_with(Span x, Span start) {
  return start.len <= x.len &&
         strncmp((char *)x.dat, (char *)start.dat, start.len) == 0;
}

private
inline Span Span_trim_start_whitespace(Span x) {
  usize offset = 0;

  while (offset < x.len && (x.dat[offset] == ' ' || x.dat[offset] == '\n' ||
                            x.dat[offset] == '\t')) {
    offset++;
  }

  return Span_slice(x, offset, x.len);
}

private
inline Span Span_trim_start(Span x, Span start) {
  if (Span_starts_with(x, start)) {
    return Span_slice(x, start.len, x.len);
  }

  return x;
}

typedef Option(T2(Span, Span)) SpanSplitOn;
private
SpanSplitOn Span_split_on(u8 byte, Span x) {
  u8 *match = (u8 *)memchr((void *)x.dat, (unsigned char)byte, x.len);
  usize match_ix = (usize)(match - x.dat);

  if (match == NULL) {
    SpanSplitOn ret = {
        .valid = false,
    };
    return ret;
  } else {
    Span fst = Span_slice(x, 0, match_ix);
    Span snd = Span_slice(x, match_ix + 1, x.len);
    SpanSplitOn ret = {
        .dat =
            {
                .fst = fst,
                .snd = snd,
            },
        .valid = true,
    };
    return ret;
  }
}

typedef struct {
  Span rest;
  u8 sep;
} SpanSplitIterator;

typedef Option(Span) SpanSplitIteratorNext;
private
SpanSplitIteratorNext SpanSplitIterator_next(SpanSplitIterator *it) {
  if (it->rest.len == 0) {
    SpanSplitIteratorNext ret = {
        .valid = false,
    };
    return ret;
  }

  SpanSplitOn split = Span_split_on(it->sep, it->rest);

  if (split.valid) {
    SpanSplitIteratorNext ret = {
        .dat = split.dat.fst,
        .valid = true,
    };
    it->rest = split.dat.snd;
    return ret;
  } else {
    SpanSplitIteratorNext ret = {
        .dat = it->rest,
        .valid = true,
    };
    it->rest.len = 0;
    return ret;
  }
}

////////////////////////////////////////////////////////////////////////////////
// HashMap

typedef struct {
  const void *key;
  void *value;
} HashEntry;

private
inline bool HashEntry_occupied(HashEntry entry) { return entry.key != NULL; }

typedef Hash (*HashFunction)(const void *key);
typedef bool (*EqFunction)(const void *a, const void *b);

private
bool usize_eq(const void *a_ptr, const void *b_ptr) {
  usize a = *(usize *)a_ptr;
  usize b = *(usize *)b_ptr;
  return a == b;
}

typedef struct {
  HashFunction get_hash;
  EqFunction get_eq;
  u32 capacity;
  u32 count;
  HashEntry *dat;
} HashMap;

private
inline HashMap HashMap_alloc(HashFunction hash, EqFunction eq, usize capacity) {
  HashEntry *dat = (HashEntry *)calloc(capacity, sizeof(HashEntry));
  HashMap hm = {.get_hash = hash,
                .get_eq = eq,
                .capacity = (u32)capacity,
                .count = 0,
                .dat = dat};
  return hm;
}

private
inline void HashMap_free(HashMap hm) { free(hm.dat); }

private
usize HashMap_entry_ix(HashMap hm, const void *key) {
  Hash hash = hm.get_hash(key);

  usize start_ix = hash % hm.capacity;

  usize ix = start_ix;
  while (HashEntry_occupied(hm.dat[ix]) && !hm.get_eq(hm.dat[ix].key, key)) {
    ix = (ix + 1) % hm.capacity;

    assert(ix != start_ix); // Ran out of space
  }

  return ix;
}

private
inline HashEntry *HashMap_entry(HashMap hm, const void *key) {
  usize ix = HashMap_entry_ix(hm, key);
  return &hm.dat[ix];
}

private
bool HashMap_contains(HashMap hm, const void *key) {
  return HashEntry_occupied(*HashMap_entry(hm, key));
}

private
void *HashMap_lookup(HashMap hm, const void *key) {
  HashEntry *entry = HashMap_entry(hm, key);
  if (HashEntry_occupied(*entry)) {
    return entry->value;
  } else {
    return NULL;
  }
}

// Return if it is overwriting a previous entry
private
bool HashMap_insert(HashMap *hm, const void *key, void *value) {
  HashEntry *entry = HashMap_entry(*hm, key);
  bool was_occupied = HashEntry_occupied(*entry);

  entry->key = key;
  entry->value = value;

  if (!was_occupied) {
    hm->count += 1;
  }

  return was_occupied;
}

// Implentation based on https://en.wikipedia.org/wiki/Open_addressing
private
HashEntry HashMap_remove(HashMap *hm, const void *key) {
  usize i = HashMap_entry_ix(*hm, key);
  HashEntry ret = hm->dat[i];

  if (!HashEntry_occupied(ret)) {
    return ret;
  }

  hm->count -= 1;
  hm->dat[i].key = NULL;

  usize j = i;
  while (true) {
    j = (j + 1) % hm->capacity;

    if (!HashEntry_occupied(hm->dat[j])) {
      return ret;
    }

    usize k = hm->get_hash(hm->dat[j].key) % hm->capacity;

    // determine if k lies cyclically in (i,j]
    // i ≤ j: |    i..k..j    |
    // i > j: |.k..j     i....| or |....j     i..k.|
    if (i <= j) {
      if ((i < k) && (k <= j)) {
        continue;
      }
    } else {
      if ((i < k) || (k <= j)) {
        continue;
      }
    }

    hm->dat[i] = hm->dat[j];
    hm->dat[j].key = NULL;
    i = j;
  }
}

#endif // BAZ_HEADER

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
    if (!(cond)) {                                                             \
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
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef size_t usize;

private
inline u8 u64_to_u8(u64 x) {
  assert_msg(x <= UINT8_MAX, "%zd is greater than %d (max u8)\n", x, UINT8_MAX);

  return (u8)x;
}

private
inline u16 u64_to_u16(u64 x) {
  assert_msg(x <= UINT16_MAX, "%zd is greater than %d (max u16)\n", x,
             UINT16_MAX);

  return (u16)x;
}

#define ABS_DIFF(T, X, Y) ({ T x = X; T y = Y; x > y ? x - y : y - x; })

///////////////////////////////////////////////////////////////////////////////
// Mem utils

private
void swap(void *__restrict a, void *__restrict b, usize bytes) {
  u8 temp[bytes]; // VLA
  memcpy(temp, a, bytes);
  memcpy(a, b, bytes);
  memcpy(b, temp, bytes);
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
#define K 0x517cc1b727220a95u
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
Hash usize_hash(const usize *x) {
  FxHasher hasher = {0};
  FxHasher_add(&hasher, *x);
  return hasher;
}

private
bool usize_eq(const usize *a, const usize *b) { return *a == *b; }

private
int usize_cmp(const usize *a, const usize *b) {
  return *a < *b ? -1 : *a == *b ? 0 : 1;
}

private
bool u8_eq(const u8 *a, const u8 *b) { return *a == *b; }

private
int u8_cmp(const u8 *a, const u8 *b) { return *a < *b ? -1 : *a == *b ? 0 : 1; }

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

#define UNWRAP(E) ({ __auto_type res = E; assert(res.valid);  res.dat; })

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
  void REQUIRE_SEMICOLON()

////////////////////////////////////////////////////////////////////////////////
// Binary Heap

// A Max Heap
// COMP_FUN of the shape: int cmp(const T *a, const T *b);
// comparison function which returns​a negative integer value if the first
// argument is less than the second, a positive integer value if the first
// argument is greater than the second and zero if the arguments are equivalent.
#define define_binary_heap(B_NAME, T, N, COMP_FUN)                             \
  typedef struct {                                                             \
    usize len;                                                                 \
    T dat[N];                                                                  \
  } B_NAME;                                                                    \
                                                                               \
  /* Implementation from https://en.wikipedia.org/wiki/Binary_heap#Insert */   \
private                                                                        \
  void B_NAME##_insert(B_NAME *binary_heap, T x) {                             \
    /* Add the element to the bottom level of the heap at the leftmost open    \
     * space. */                                                               \
    assert(binary_heap->len < N);                                              \
    usize ix = binary_heap->len;                                               \
    binary_heap->dat[ix] = x;                                                  \
    binary_heap->len += 1;                                                     \
                                                                               \
    if (binary_heap->len == 1) {                                               \
      return;                                                                  \
    }                                                                          \
                                                                               \
    /* Compare the added element with its parent; if they are in the correct   \
     * order, stop. */                                                         \
    usize parent = (ix - 1) / 2;                                               \
                                                                               \
    while (true) {                                                             \
      int cmp = COMP_FUN(&binary_heap->dat[parent], &binary_heap->dat[ix]);    \
                                                                               \
      if (cmp >= 0) {                                                          \
        return;                                                                \
      }                                                                        \
                                                                               \
      /* If not, swap the element with its parent and return to the previous   \
       * step. */                                                              \
      swap(&binary_heap->dat[ix], &binary_heap->dat[parent], sizeof(T));       \
                                                                               \
      ix = parent;                                                             \
      if (ix == 0) {                                                           \
        return;                                                                \
      }                                                                        \
                                                                               \
      parent = (ix - 1) / 2;                                                   \
    }                                                                          \
  }                                                                            \
                                                                               \
  typedef Option(T) B_NAME##Extract;                                           \
  /* Implementation from https://en.wikipedia.org/wiki/Binary_heap#Extract */  \
private                                                                        \
  B_NAME##Extract B_NAME##_extract(B_NAME *binary_heap) {                      \
    /* Initialise as not valid */                                              \
    B_NAME##Extract ret = {                                                    \
        .valid = false,                                                        \
    };                                                                         \
                                                                               \
    if (binary_heap->len == 0) {                                               \
      return ret;                                                              \
    }                                                                          \
    ret.valid = true;                                                          \
    ret.dat = binary_heap->dat[0];                                             \
                                                                               \
    binary_heap->len--;                                                        \
    if (binary_heap->len == 0) {                                               \
      return ret;                                                              \
    }                                                                          \
                                                                               \
    /* Replace the root of the heap with the last element on the last level.   \
     */                                                                        \
    binary_heap->dat[0] = binary_heap->dat[binary_heap->len];                  \
                                                                               \
    usize ix = 0;                                                              \
    while (true) {                                                             \
      /* No left child, we're done */                                          \
      if (ix * 2 + 1 >= binary_heap->len) {                                    \
        return ret;                                                            \
      }                                                                        \
                                                                               \
      int cmp_l =                                                              \
          COMP_FUN(&binary_heap->dat[ix], &binary_heap->dat[ix * 2 + 1]);      \
                                                                               \
      /* No right child, check with left if valid state, then we're done */    \
      if (ix * 2 + 2 >= binary_heap->len) {                                    \
        if (cmp_l < 0) {                                                       \
          swap(&binary_heap->dat[ix], &binary_heap->dat[ix * 2 + 1],           \
               sizeof(T));                                                     \
        }                                                                      \
        return ret;                                                            \
      }                                                                        \
                                                                               \
      int cmp_r =                                                              \
          COMP_FUN(&binary_heap->dat[ix], &binary_heap->dat[ix * 2 + 2]);      \
                                                                               \
      /* Compare the new root with its children; if they are in the correct    \
       * order, stop. */                                                       \
      if (cmp_l >= 0 && cmp_r >= 0) {                                          \
        return ret;                                                            \
      }                                                                        \
                                                                               \
      /* If not, swap the element with one of its children and return to the   \
       * previous step. */                                                     \
      int cmp_c = COMP_FUN(&binary_heap->dat[ix * 2 + 1],                      \
                           &binary_heap->dat[ix * 2 + 2]);                     \
                                                                               \
      if (cmp_c < 0) {                                                         \
        /* Right is greater */                                                 \
        swap(&binary_heap->dat[ix], &binary_heap->dat[ix * 2 + 2], sizeof(T)); \
        ix = ix * 2 + 2;                                                       \
                                                                               \
      } else {                                                                 \
        /* Left is greater */                                                  \
        swap(&binary_heap->dat[ix], &binary_heap->dat[ix * 2 + 1], sizeof(T)); \
        ix = ix * 2 + 1;                                                       \
      }                                                                        \
    }                                                                          \
  }                                                                            \
                                                                               \
  void REQUIRE_SEMICOLON()

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
Hash Span_hash(const Span *span) {
  FxHasher hasher = {0};

  for (usize i = 0; i < span->len; i++) {
    FxHasher_add(&hasher, (usize)span->dat[i]);
  }

  return hasher;
}

// Intended for generic equality
private
bool Span_eq(const Span *a, const Span *b) {
  return a->len == b->len && memcmp(a->dat, b->dat, a->len) == 0;
}

private bool Span_match(const Span *x, const char *ref) {
  Span y = Span_from_str(ref);
  return Span_eq(x, &y);
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

typedef Option(T2(i64, Span)) SpanParseI64;
private
SpanParseI64 Span_parse_i64(Span x, int base) {
  errno = 0;
  char *end;
  const i64 res = strtol((char *)x.dat, &end, base);
  assert(errno == 0);

  if (x.dat == (u8 *)end) {
    SpanParseI64 ret = {
        .valid = false,
    };
    return ret;
  }

  SpanParseI64 ret = {
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

private SpanSplitIterator Span_split_lines(Span x) {
  SpanSplitIterator s = {
    .rest = x,
    .sep = (u8) '\n',
  };
  return s;
}

private SpanSplitIterator Span_split_words(Span x) {
  SpanSplitIterator s = {
    .rest = x,
    .sep = (u8) ' ',
  };
  return s;
}

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

// Define a hash_map with N elements, keys K, values V
// K_HASH is a function: Hash func(const K *key)
// K_EQ is a function: bool func(const K *a, const K *b)
#define define_hash_map(H_NAME, K, V, N, K_HASH, K_EQ)                         \
  typedef struct {                                                             \
    usize count;                                                               \
    bool occupied[N];                                                          \
    K keys[N];                                                                 \
    V values[N];                                                               \
  } H_NAME;                                                                    \
                                                                               \
private                                                                        \
  usize H_NAME##_entry_ix(const H_NAME *hm, const K *key) {                    \
    Hash hash = K_HASH(key);                                                   \
                                                                               \
    usize start_ix = hash % N;                                                 \
                                                                               \
    usize ix = start_ix;                                                       \
    while (hm->occupied[ix] && !K_EQ(&hm->keys[ix], key)) {                    \
      ix = (ix + 1) % N;                                                       \
                                                                               \
      assert(ix != start_ix); /* Ran out of space */                           \
    }                                                                          \
                                                                               \
    return ix;                                                                 \
  }                                                                            \
                                                                               \
private                                                                        \
  inline bool H_NAME##_contains(const H_NAME *hm, const K *key) {              \
    usize ix = H_NAME##_entry_ix(hm, key);                                     \
    return hm->occupied[ix];                                                   \
  }                                                                            \
                                                                               \
  typedef Option(V *) H_NAME##Lookup;                                          \
private                                                                        \
  H_NAME##Lookup H_NAME##_lookup(H_NAME *hm, const K *key) {                   \
    usize ix = H_NAME##_entry_ix(hm, key);                                     \
    if (hm->occupied[ix]) {                                                    \
      H_NAME##Lookup ret = {                                                   \
          .dat = &hm->values[ix],                                              \
          .valid = true,                                                       \
      };                                                                       \
      return ret;                                                              \
    } else {                                                                   \
      H_NAME##Lookup ret = {                                                   \
          .valid = false,                                                      \
      };                                                                       \
      return ret;                                                              \
    }                                                                          \
  }                                                                            \
                                                                               \
  /* Return if it is overwriting a previous entry */                           \
private                                                                        \
  bool H_NAME##_insert(H_NAME *hm, K key, V value) {                           \
    usize ix = H_NAME##_entry_ix(hm, &key);                                    \
    bool was_occupied = hm->occupied[ix];                                      \
                                                                               \
    hm->occupied[ix] = true;                                                   \
    hm->keys[ix] = key;                                                        \
    hm->values[ix] = value;                                                    \
                                                                               \
    if (!was_occupied) {                                                       \
      hm->count += 1;                                                          \
    }                                                                          \
                                                                               \
    return was_occupied;                                                       \
  }                                                                            \
                                                                               \
private                                                                        \
  V *H_NAME##_insert_modify(H_NAME *hm, K key, V def) {                        \
    usize ix = H_NAME##_entry_ix(hm, &key);                                    \
    bool was_occupied = hm->occupied[ix];                                      \
                                                                               \
    hm->occupied[ix] = true;                                                   \
    hm->keys[ix] = key;                                                        \
                                                                               \
    if (!was_occupied) {                                                       \
      hm->count += 1;                                                          \
      hm->values[ix] = def;                                                    \
    }                                                                          \
                                                                               \
    return &hm->values[ix];                                                    \
  }                                                                            \
                                                                               \
  /* Implementation based on https://en.wikipedia.org/wiki/Open_addressing */  \
  typedef Option(T2(K, V)) H_NAME##Remove;                                     \
private                                                                        \
  H_NAME##Remove H_NAME##_remove(H_NAME *hm, const K *key) {                   \
    usize i = H_NAME##_entry_ix(hm, key);                                      \
    H_NAME##Remove ret = {                                                     \
        .valid = false,                                                        \
    };                                                                         \
                                                                               \
    if (!hm->occupied[i]) {                                                    \
      return ret;                                                              \
    }                                                                          \
                                                                               \
    ret.valid = true;                                                          \
    ret.dat.fst = hm->keys[i];                                                 \
    ret.dat.snd = hm->values[i];                                               \
                                                                               \
    hm->count -= 1;                                                            \
    hm->occupied[i] = false;                                                   \
                                                                               \
    usize j = i;                                                               \
    while (true) {                                                             \
      j = (j + 1) % N;                                                         \
                                                                               \
      if (!hm->occupied[j]) {                                                  \
        return ret;                                                            \
      }                                                                        \
                                                                               \
      usize k = K_HASH(&hm->keys[j]) % N;                                      \
                                                                               \
      /* determine if k lies cyclically in (i,j]                               \
         i ≤ j: |    i..k..j    |                                            \
         i > j: |.k..j     i....| or |....j     i..k.| */                      \
      if (i <= j) {                                                            \
        if ((i < k) && (k <= j)) {                                             \
          continue;                                                            \
        }                                                                      \
      } else {                                                                 \
        if ((i < k) || (k <= j)) {                                             \
          continue;                                                            \
        }                                                                      \
      }                                                                        \
                                                                               \
      hm->keys[i] = hm->keys[j];                                               \
      hm->values[i] = hm->values[j];                                           \
      hm->occupied[j] = false;                                                 \
      i = j;                                                                   \
    }                                                                          \
  }                                                                            \
                                                                               \
  void REQUIRE_SEMICOLON()

////////////////////////////////////////////////////////////////////////////////
// BitSet

#define define_bit_set(B_NAME, T, N)                                           \
  typedef struct {                                                             \
    T dat[N];                                                                  \
  } B_NAME;                                                                    \
                                                                               \
private                                                                        \
  usize T##_bits = 8 * sizeof(T);                                              \
                                                                               \
private                                                                        \
  usize B_NAME##_size = N * 8 * sizeof(T);                                     \
                                                                               \
private                                                                        \
  inline void B_NAME##_insert(B_NAME *bs, usize x) {                           \
    assert(x / T##_bits < N);                                                  \
    bs->dat[x / T##_bits] |= ((T)1) << (x % T##_bits);                         \
  }                                                                            \
                                                                               \
private                                                                        \
  inline bool B_NAME##_contains(B_NAME bs, usize x) {                          \
    assert(x / T##_bits < N);                                                  \
    return (bool)((bs.dat[x / T##_bits] >> (x % T##_bits)) & 1);               \
  }                                                                            \
                                                                               \
private                                                                        \
  inline void B_NAME##_remove(B_NAME *bs, usize x) {                           \
    assert(x / T##_bits < N);                                                  \
    bs->dat[x / T##_bits] &= ~(((T)1) << (x % T##_bits));                      \
  }                                                                            \
                                                                               \
private                                                                        \
  bool B_NAME##_eq(B_NAME a, B_NAME b) {                                       \
    for (usize i = 0; i < N; i++) {                                            \
      if (a.dat[i] != b.dat[i]) {                                              \
        return false;                                                          \
      }                                                                        \
    }                                                                          \
    return true;                                                               \
  }                                                                            \
                                                                               \
private                                                                        \
  B_NAME B_NAME##_union(B_NAME a, B_NAME b) {                                  \
    for (usize i = 0; i < N; i++) {                                            \
      a.dat[i] |= b.dat[i];                                                    \
    }                                                                          \
    return a;                                                                  \
  }                                                                            \
                                                                               \
private                                                                        \
  B_NAME B_NAME##_intersection(B_NAME a, B_NAME b) {                           \
    for (usize i = 0; i < N; i++) {                                            \
      a.dat[i] &= b.dat[i];                                                    \
    }                                                                          \
    return a;                                                                  \
  }                                                                            \
                                                                               \
  /* Is "a" a subset of "b" */                                                 \
private                                                                        \
  inline bool B_NAME##_is_subset(B_NAME a, B_NAME b) {                         \
    return B_NAME##_eq(a, B_NAME##_intersection(a, b));                        \
  }                                                                            \
                                                                               \
private                                                                        \
  inline usize B_NAME##_count(B_NAME a) {                                      \
    usize x = 0;                                                               \
    for (usize i = 0; i < N; i++) {                                            \
      x += (usize)__builtin_popcount(a.dat[i]);                                \
    }                                                                          \
    return x;                                                                  \
  }                                                                            \
                                                                               \
  void REQUIRE_SEMICOLON()

#endif // BAZ_HEADER

#include "baz.h"

typedef struct {
  Span seed;
  i32 i;
  usize k;
  bool mirror;
} ModifiedDragonCurveIterator;

static inline u8 dragon_curve(usize k) {
  return (u8)((k >> (__builtin_ctzl(k) + 1)) & 1);
}

static u8 ModifiedDragonCurveIterator_next(ModifiedDragonCurveIterator *it) {
  u8 ret;
  if (it->mirror) {
    // Moving backwards
    if (it->i < 0) {
      it->mirror = false;
      it->i++;
      it->k++;
      return dragon_curve(it->k);
    }

    ret = 1 - (it->seed.dat[it->i] - '0');
    it->i--;
  } else {
    // Moving forward
    if (it->i == (i32)it->seed.len) {
      it->mirror = true;
      it->i--;
      it->k++;
      return dragon_curve(it->k);
    }
    ret = it->seed.dat[it->i] - '0';
    it->i++;
  }

  return ret;
}

define_array(String, u8, 32);

typedef struct {
  u8 level;
  u8 val;
} Partial;

define_array(PartialStack, Partial, 32);

typedef struct {
  String checksum;
  PartialStack partials;
  u8 levels;
} ChecksumBuilder;

static inline void ChecksumBuilder_push(ChecksumBuilder *cb, u8 val) {
  Partial current = {
      .level = cb->levels,
      .val = val,
  };

  PartialStackPeek top = PartialStack_peek(&cb->partials);

  while (top.valid && top.dat.level == current.level) {
    current.val = (u8)(current.val == top.dat.val);
    current.level--;

    cb->partials.len--; // pop

    if (current.level == 0) {
      String_push(&cb->checksum, current.val + '0');
      return;
    }

    top = PartialStack_peek(&cb->partials);
  }

  PartialStack_push(&cb->partials, current);
  return;
}

private
void solve(Span input, usize disk_len) {
  ModifiedDragonCurveIterator dragon = {
      .seed = input,
      .i = 0,
      .mirror = false,
  };

  u8 levels = 0;
  usize len = disk_len;
  while ((len & 1) == 0) {
    levels++;
    len >>= 1;
  }

  ChecksumBuilder cb = {
      .levels = levels,
  };

  for (usize i = 0; i < disk_len; i++) {
    ChecksumBuilder_push(&cb, ModifiedDragonCurveIterator_next(&dragon));
  }

  Span checksum = {
      .dat = cb.checksum.dat,
      .len = cb.checksum.len,
  };
  Span_print(checksum);
  printf("\n");
}

int main(void) {
  // Example
  solve(Span_from_str("110010110100"), 12);
  solve(Span_from_str("10000"), 20);

  solve(Span_from_str("11100010111110100"), 272);
  solve(Span_from_str("11100010111110100"), 35651584);

  return 0;
}

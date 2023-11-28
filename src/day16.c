#include "baz.h"

static inline u8 dragon_curve(usize k) {
  return (u8)((k >> (__builtin_ctzl(k) + 1)) & 1);
}

static u8 modified_dragon_curve(const u8 *seed, const u8 *mirror, usize len,
                                usize i) {
  usize k = i / (len + 1);
  usize o = i % (len + 1);

  if (o < len) {
    const u8 *buf = k % 2 == 0 ? seed : mirror;
    return buf[o];
  } else {
    return dragon_curve(k + 1);
  }
}

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
  assert(input.len <= 32);
  u8 seed[32] = {0};
  u8 mirror[32] = {0};

  for (usize i = 0; i < input.len; i++) {
    seed[i] = input.dat[i] - '0';
    mirror[input.len - 1 - i] = 1 - seed[i];
  }

  u8 levels = (u8)__builtin_ctzl(disk_len);

  ChecksumBuilder cb = {
      .levels = levels,
  };

  usize block_size = 2 * (input.len + 1);
  usize blocks = disk_len / block_size;

  for (usize j = 0; j < blocks; j++) {
    for (usize i = 0; i < input.len; i++) {
      ChecksumBuilder_push(&cb, seed[i]);
    }

    ChecksumBuilder_push(&cb, dragon_curve(2 * j + 1));

    for (usize i = 0; i < input.len; i++) {
      ChecksumBuilder_push(&cb, mirror[i]);
    }

    ChecksumBuilder_push(&cb, dragon_curve(2 * j + 2));
  }

  for (usize i = 0; i < disk_len % block_size; i++) {
    ChecksumBuilder_push(&cb, modified_dragon_curve(seed, mirror, input.len,
                                                    blocks * block_size + i));
  }

  Span checksum = {
      .dat = cb.checksum.dat,
      .len = cb.checksum.len,
  };
  String out = {0};
  String_push_span(&out, checksum);
  String_println(&out);
}

int main(void) {
  // Example
  solve(Span_from_str("110010110100"), 12);
  solve(Span_from_str("10000"), 20);

  solve(Span_from_str("11100010111110100"), 272);
  solve(Span_from_str("11100010111110100"), 35651584);

  return 0;
}

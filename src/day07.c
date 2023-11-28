#include "baz.h"

static bool contains_abba(Span x) {
  if (x.len < 4) {
    return false;
  }

  for (usize i = 0; i < x.len - 3; i++) {
    if (x.dat[i] == x.dat[i + 3] && x.dat[i + 1] == x.dat[i + 2] &&
        x.dat[i] != x.dat[i + 1]) {
      return true;
    }
  }

  return false;
}

static bool ip_supports_tls(Span ip) {
  SpanSplitIterator square_it = {
      .rest = ip,
      .sep = (u8)'[',
  };

  bool supports_tls = false;
  bool in_hypernet = false;

  SpanSplitIteratorNext section = SpanSplitIterator_next(&square_it);
  while (section.valid) {
    bool is_abba = contains_abba(section.dat);

    if (!in_hypernet && is_abba) {
      supports_tls = true;
    }

    if (in_hypernet && is_abba) {
      supports_tls = false;
      break;
    }

    // Change separator
    in_hypernet = !in_hypernet;
    square_it.sep = in_hypernet ? (u8)']' : (u8)'[';
    section = SpanSplitIterator_next(&square_it);
  }

  return supports_tls;
}

define_array(SpanArray, Span, 32);
define_hash_map(SpanHashSet, Span, u8, 32, Span_hash, Span_eq);

static bool ip_supports_ssl(Span ip) {
  SpanSplitIterator square_it = {
      .rest = ip,
      .sep = (u8)'[',
  };

  bool in_hypernet = false;

  SpanArray in_abas = {0};
  SpanHashSet out_abas_set = {0};

  SpanSplitIteratorNext section = SpanSplitIterator_next(&square_it);
  while (section.valid) {
    if (section.dat.len >= 3) {
      for (usize i = 0; i < section.dat.len - 2; i++) {
        if (section.dat.dat[i] == section.dat.dat[i + 2] &&
            section.dat.dat[i] != section.dat.dat[i + 1]) {
          // It is an ABA
          Span aba = Span_slice(section.dat, i, i + 3);

          if (in_hypernet) {
            SpanArray_push(&in_abas, aba);
          } else {
            SpanHashSet_insert(&out_abas_set, aba, 0);
          }
        }
      }
    }

    // Change separator
    in_hypernet = !in_hypernet;
    square_it.sep = in_hypernet ? (u8)']' : (u8)'[';
    section = SpanSplitIterator_next(&square_it);
  }

  for (usize i = 0; i < in_abas.len; i++) {
    Span aba = in_abas.dat[i];

    u8 bab_array[3] = {aba.dat[1], aba.dat[0], aba.dat[1]};
    Span bab = {
        .dat = bab_array,
        .len = 3,
    };

    if (SpanHashSet_contains(&out_abas_set, &bab)) {
      return true;
    }
  }

  return false;
}

static void solve(Span input) {
  usize part1 = 0;
  usize part2 = 0;
  SpanSplitIterator line_it = {
      .rest = input,
      .sep = (u8)'\n',
  };

  SpanSplitIteratorNext line = SpanSplitIterator_next(&line_it);
  while (line.valid) {
    if (ip_supports_tls(line.dat)) {
      part1++;
    }

    if (ip_supports_ssl(line.dat)) {
      part2++;
    }

    line = SpanSplitIterator_next(&line_it);
  }

  String out = {0};
  String_push_str(&out, "part1: ");
  String_push_u64(&out, part1, 10);
  String_println(&out);

  String_clear(&out);
  String_push_str(&out, "part2: ");
  String_push_u64(&out, part2, 10);
  String_println(&out);
}

int main(void) {
  Span input = Span_from_file("inputs/day07.txt");

  solve(input);

  return 0;
}

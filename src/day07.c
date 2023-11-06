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

static bool ip_supports_ssl(Span ip) {
  SpanSplitIterator square_it = {
      .rest = ip,
      .sep = (u8)'[',
  };

  bool in_hypernet = false;

  SpanArray in_abas = {.len = 0};
  SpanArray out_abas = {.len = 0};

  // Span keys, NULL values
  HashEntry out_aba_entries[32] = {0};
  HashMap out_abas_set = {
      .get_hash = Span_hash,
      .get_eq = Span_eq,
      .capacity = 32,
      .count = 0,
      .dat = out_aba_entries,
  };

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
            HashMap_insert(&out_abas_set, SpanArray_push(&out_abas, aba), NULL);
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

    if (HashMap_contains(out_abas_set, &bab)) {
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

  printf("part1: %zd\n", part1);
  printf("part2: %zd\n", part2);
}

int main(void) {
  printf("%d\n", ip_supports_tls(Span_from_str("abba[mnop]qrst")));
  printf("%d\n", ip_supports_tls(Span_from_str("abcd[bddb]xyyx")));
  printf("%d\n", ip_supports_tls(Span_from_str("aaaa[qwer]tyui")));
  printf("%d\n", ip_supports_tls(Span_from_str("ioxxoj[asdfgh]zxcvbn")));

  printf("%d\n", ip_supports_ssl(Span_from_str("aba[bab]xyz")));
  printf("%d\n", ip_supports_ssl(Span_from_str("xyx[xyx]xyx")));
  printf("%d\n", ip_supports_ssl(Span_from_str("aaa[kek]eke")));
  printf("%d\n", ip_supports_ssl(Span_from_str("zazbz[bzb]cdb")));

  Span input = Span_from_file("inputs/day07.txt");

  solve(input);

  return 0;
}

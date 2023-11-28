#include "baz.h"

typedef struct {
  u16 dat[26];
} CharFreqMap;

static inline void CharFreqMap_insert(CharFreqMap *fm, u8 c) {
  assert(c >= (u8)'a' && c <= (u8)'z');

  fm->dat[c - (u8)'a'] += 1;
}

static u8 CharFreqMap_most_frequent(CharFreqMap fm) {
  u16 max_val = 0;
  u8 max_ix = 0;

  for (u8 i = 0; i < 26; i++) {
    if (fm.dat[i] >= max_val) {
      max_val = fm.dat[i];
      max_ix = i;
    }
  }

  return max_ix + (u8)'a';
}

// Note: least but none-zero
static u8 CharFreqMap_least_frequent(CharFreqMap fm) {
  u16 min_val = UINT16_MAX;
  u8 min_ix = 0;

  for (u8 i = 0; i < 26; i++) {
    if (fm.dat[i] > 0 && fm.dat[i] <= min_val) {
      min_val = fm.dat[i];
      min_ix = i;
    }
  }

  return min_ix + (u8)'a';
}

define_array(FreqMaps, CharFreqMap, 16);

static void solve(Span input) {

  FreqMaps freq_maps = {0};
  SpanSplitIterator line_it = {
      .rest = input,
      .sep = (u8)'\n',
  };

  bool first_iter = true;
  SpanSplitIteratorNext line = SpanSplitIterator_next(&line_it);
  while (line.valid) {
    if (first_iter) {
      first_iter = false;

      assert(line.dat.len <= 16);

      // This is valid because freq_map items are 0 initialized
      freq_maps.len = line.dat.len;
    }

    // Every line needs to be the same length
    assert(line.dat.len == freq_maps.len);

    for (usize i = 0; i < freq_maps.len; i++) {
      CharFreqMap_insert(&freq_maps.dat[i], line.dat.dat[i]);
    }

    line = SpanSplitIterator_next(&line_it);
  }

  String out = {0};
  String_push_str(&out, "Most frequent: ");
  for (usize i = 0; i < freq_maps.len; i++) {
    String_push(&out, CharFreqMap_most_frequent(freq_maps.dat[i]));
  }
  String_push(&out, '\n');
  String_print(&out);

  String_clear(&out);
  String_push_str(&out, "Least frequent: ");
  for (usize i = 0; i < freq_maps.len; i++) {
    String_push(&out, CharFreqMap_least_frequent(freq_maps.dat[i]));
  }
  String_push(&out, '\n');
  String_print(&out);
}

int main(void) {
  Span example = Span_from_str("eedadn\n"
                               "drvtee\n"
                               "eandsr\n"
                               "raavrd\n"
                               "atevrs\n"
                               "tsrnev\n"
                               "sdttsa\n"
                               "rasrtv\n"
                               "nssdts\n"
                               "ntnada\n"
                               "svetve\n"
                               "tesnvt\n"
                               "vntsnd\n"
                               "vrdear\n"
                               "dvrsen\n"
                               "enarar\n");

  solve(example);

  Span input = Span_from_file("inputs/day06.txt");
  solve(input);

  return 0;
}

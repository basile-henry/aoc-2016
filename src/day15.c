#include "baz.h"

typedef struct {
  u8 ix;
  u8 positions;
  u8 start;
} Disc;

static Disc Disc_parse(Span line) {
  // Disc #1 has 5 positions; at time=0, it is at position 4.

  SpanSplitIterator word_it = Span_split_words(line);
  SpanSplitIterator_skip(&word_it, 1);                      // Disc
  Span ix_span = UNWRAP(SpanSplitIterator_next(&word_it));  // #<N>
  SpanSplitIterator_skip(&word_it, 1);                      // has
  Span pos_span = UNWRAP(SpanSplitIterator_next(&word_it)); // <p>
  SpanSplitIterator_skip(&word_it, 7); // position; at time=3 it is at position
  Span start_span = UNWRAP(SpanSplitIterator_next(&word_it)); // <s>.

  Disc disc = {
      .ix = (u8)UNWRAP(Span_parse_u64(Span_slice(ix_span, 1, ix_span.len), 10))
                .fst,
      .positions = (u8)UNWRAP(Span_parse_u64(pos_span, 10)).fst,
      .start = (u8)UNWRAP(Span_parse_u64(start_span, 10)).fst,
  };

  return disc;
}

define_array(Discs, Disc, 8);

static bool fall_through(const Discs *discs, usize time) {
  for (usize i = 0; i < discs->len; i++) {
    Disc disc = discs->dat[i];
    if ((time + (usize)disc.start + i + 1) % disc.positions != 0) {
      return false;
    }
  }

  return true;
}

void solve(Span input) {
  SpanSplitIterator line_it = Span_split_lines(input);

  Discs discs = {0};

  SpanSplitIteratorNext line = SpanSplitIterator_next(&line_it);
  while (line.valid) {
    Disc disc = Disc_parse(line.dat);
    Discs_push(&discs, disc);

    line = SpanSplitIterator_next(&line_it);
  }

  {
    usize time = 0;
    while (!fall_through(&discs, time)) {
      time++;
    }

    printf("%zd\n", time);
  }

  Disc new_disc = {
      .ix = (u8)discs.len,
      .positions = 11,
      .start = 0,

  };
  Discs_push(&discs, new_disc);

  {
    usize time = 0;
    while (!fall_through(&discs, time)) {
      time++;
    }

    printf("%zd\n", time);
  }
}

int main(void) {
  Span example = Span_from_str(
      "Disc #1 has 5 positions; at time=0, it is at position 4.\n"
      "Disc #2 has 2 positions; at time=0, it is at position 1.\n");
  solve(example);

  Span input = Span_from_file("inputs/day15.txt");
  solve(input);

  return 0;
}

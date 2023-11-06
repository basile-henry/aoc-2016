#include "baz.h"
#include <assert.h>
#include <stdio.h>

typedef struct {
  u64 rows[6];
} Screen;

private
void Screen_print(const Screen *screen) {
  for (u64 row_ix = 0; row_ix < 6; row_ix++) {
    for (u64 col_ix = 0; col_ix < 50; col_ix++) {
      bool on = ((screen->rows[row_ix] >> col_ix) & 1ull) == 1ull;
      putc(on ? '#' : '.', stdout);
    }
    putc('\n', stdout);
  }
}

static int Screen_count(const Screen *screen) {
  int count = 0;

  for (u64 row_ix = 0; row_ix < 6; row_ix++) {
    count += __builtin_popcountl(screen->rows[row_ix]);
  }

  return count;
}

static void Screen_rect(Screen *screen, u64 a, u64 b) {
  assert(a < 50);
  assert(b < 6);

  for (u64 row_ix = 0; row_ix < b; row_ix++) {
    screen->rows[row_ix] |= (1ull << a) - 1ull;
  }
}

static void Screen_rotate_row(Screen *screen, u64 y, u64 n) {
  assert(y < 6);

  u64 row = screen->rows[y];
  u64 rot = n % 50;

  u64 row_mask = (1ull << 50) - 1ull;
  u64 bottom_mask = (1ull << rot) - 1ull;
  u64 top_mask = row_mask & ~bottom_mask;

  u64 top = (row << rot) & top_mask;
  u64 bottom = (row >> (50 - rot)) & bottom_mask;

  screen->rows[y] = top | bottom;
}

static void Screen_rotate_column(Screen *screen, u64 x, u64 n) {
  assert(x < 50);
  u64 col = 0;

  for (u64 row_ix = 0; row_ix < 6; row_ix++) {
    col |= ((screen->rows[row_ix] >> x) & 1) << row_ix;
  }

  for (int row_ix = 0; row_ix < 6; row_ix++) {
    int rot_row_ix = ( 6 + row_ix - (int) n) % 6;

    if (((col >> rot_row_ix) & 1) == 1) {
      screen->rows[row_ix] |= 1ull << x;
    } else {
      screen->rows[row_ix] &= ~(1ull << x);
    }
  }
}

static void solve(Span input) {
  Screen screen = {0};

  SpanSplitIterator line_it = {
      .rest = input,
      .sep = (u8)'\n',
  };

  SpanSplitIteratorNext line = SpanSplitIterator_next(&line_it);
  while (line.valid) {

    Span rect = Span_from_str("rect ");
    Span rotate_row = Span_from_str("rotate row y=");
    Span rotate_col = Span_from_str("rotate column x=");

    if (Span_starts_with(line.dat, rect)) {
      Span rest = Span_slice(line.dat, rect.len, line.dat.len);
      SpanParseU64 a_parse = Span_parse_u64(rest, 10);
      assert(a_parse.valid);

      Span rest2 = Span_slice(a_parse.dat.snd, 1, a_parse.dat.snd.len);
      SpanParseU64 b_parse = Span_parse_u64(rest2, 10);
      assert(b_parse.valid);

      usize a = a_parse.dat.fst;
      usize b = b_parse.dat.fst;
      Screen_rect(&screen, a, b);

    } else if (Span_starts_with(line.dat, rotate_row)) {
      Span rest = Span_slice(line.dat, rotate_row.len, line.dat.len);
      SpanParseU64 y_parse = Span_parse_u64(rest, 10);
      assert(y_parse.valid);

      Span rest2 = Span_slice(y_parse.dat.snd, 4, y_parse.dat.snd.len);
      SpanParseU64 n_parse = Span_parse_u64(rest2, 10);
      assert(n_parse.valid);

      usize y = y_parse.dat.fst;
      usize n = n_parse.dat.fst;

      Screen_rotate_row(&screen, y, n);

    } else if (Span_starts_with(line.dat, rotate_col)) {
      Span rest = Span_slice(line.dat, rotate_col.len, line.dat.len);
      SpanParseU64 x_parse = Span_parse_u64(rest, 10);
      assert(x_parse.valid);

      Span rest2 = Span_slice(x_parse.dat.snd, 4, x_parse.dat.snd.len);
      SpanParseU64 n_parse = Span_parse_u64(rest2, 10);
      assert(n_parse.valid);

      usize x = x_parse.dat.fst;
      usize n = n_parse.dat.fst;

      Screen_rotate_column(&screen, x, n);
    } else {
      panic("unexpected");
    }


    line = SpanSplitIterator_next(&line_it);
  }

  Screen_print(&screen);
  printf("%d\n", Screen_count(&screen));
}

int main(void) {
  Span input = Span_from_file("inputs/day08.txt");
  solve(input);

  return 0;
}

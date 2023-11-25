#include "baz.h"

define_bit_set(Row, u64, 2);

static Row next_row(Row x, usize width) {
  Row next = {0};
  if (Row_contains(x, 1)) {
    Row_insert(&next, 0);
  }

  for (usize i = 1; i < width - 1; i++) {
    if (Row_contains(x, i - 1) ^ Row_contains(x, i + 1)) {
      Row_insert(&next, i);
    }
  }

  if (Row_contains(x, width - 2)) {
    Row_insert(&next, width - 1);
  }

  return next;
}

static void solve(Span input, usize num_rows) {
  Span first_row_span = Span_trim_end_whitespace(input);
  usize width = first_row_span.len;
  Row first_row = {0};

  for (usize i = 0; i < first_row_span.len; i++) {
    if (first_row_span.dat[i] == '^') {
      Row_insert(&first_row, i);
    }
  }

  usize traps = 0;

  traps += Row_count(first_row);

  Row prev_row = first_row;
  for (usize i = 1; i < num_rows; i++) {
    Row row = next_row(prev_row, width);
    traps += Row_count(row);
    prev_row = row;
  }

  printf("%zd\n", width * num_rows - traps);
}

int main(void) {
  Span example = Span_from_str(".^^.^.^^^^\n");
  solve(example, 10);

  Span input = Span_from_file("inputs/day18.txt");
  solve(input, 40);
  solve(input, 400000);

  return 0;
}

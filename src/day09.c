#include "baz.h"
#include <stdio.h>

typedef struct {
  u16 num_char;
  u16 repeat;
} Marker;

// Return the Span after the marker's closing ')'
// Assume the initial '(' is already consumed
Span parse_marker(Span input, Marker *out) {
  SpanParseU64 res_num_char = Span_parse_u64(input, 10);
  assert(res_num_char.valid);
  out->num_char = u64_to_u16(res_num_char.dat.fst);

  input = Span_trim_start(res_num_char.dat.snd, Span_from_str("x"));

  SpanParseU64 res_repeat = Span_parse_u64(input, 10);
  assert(res_repeat.valid);
  out->repeat = u64_to_u8(res_repeat.dat.fst);

  input = Span_trim_start(res_repeat.dat.snd, Span_from_str(")"));

  return input;
}

u64 solve(Span input, bool recurse) {
  u64 count = 0;

  while (input.len > 0) {
    // Whitespace is ignored
    input = Span_trim_start_whitespace(input);

    SpanSplitOn to_marker = Span_split_on('(', input);
    SpanSplitOn to_newline = Span_split_on('\n', input);

    if (to_marker.valid &&
        (!to_newline.valid || to_marker.dat.fst.len < to_newline.dat.fst.len)) {
      // Dealing with marker
      u64 advance = (u64)to_marker.dat.fst.len;
      count += advance;

      Marker m;
      input = parse_marker(to_marker.dat.snd, &m);

      Span recursive = Span_slice(input, 0, m.num_char);
      input = Span_slice(input, m.num_char, input.len);

      if (recurse) {
        count += solve(recursive, true) * (u64)m.repeat;
      } else {
        count += (u64)m.num_char * (u64)m.repeat;
      }
    } else if (to_newline.valid) {
      // Dealing with text
      u64 advance = (u64)to_newline.dat.fst.len;
      count += advance;
      input = to_newline.dat.snd;
    } else {
      count += input.len;
      break;
    }
  }

  return count;
}

int main(void) {
  Span input = Span_from_file("inputs/day09.txt");
  printf("%zd\n", solve(input, false));
  printf("%zd\n", solve(input, true));

  return 0;
}

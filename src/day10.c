#include "baz.h"

define_array(Chips, u8, 8);
define_array(GivingBots, u8, 128);

typedef struct {
  u8 low;
  u8 high;
  bool low_output;  // as opposed to a bot
  bool high_output; // as opposed to a bot
} Instr;

void solve(Span input) {
  GivingBots giving_bots = {0};
  Chips output_chips[256] = {0};
  Chips bot_chips[256] = {0};
  Instr bot_instrs[256] = {0};

  SpanSplitIterator line_it = {
      .rest = input,
      .sep = (u8)'\n',
  };

  SpanSplitIteratorNext line = SpanSplitIterator_next(&line_it);
  while (line.valid) {
    Span value_span = Span_from_str("value ");
    Span bot_span = Span_from_str("bot ");
    Span output_span = Span_from_str("output ");

    if (Span_starts_with(line.dat, value_span)) {
      Span instr = Span_slice(line.dat, value_span.len, line.dat.len);

      SpanParseU64 res_value = Span_parse_u64(instr, 10);
      assert(res_value.valid);
      u8 value = u64_to_u8(res_value.dat.fst);

      instr =
          Span_trim_start(res_value.dat.snd, Span_from_str(" goes to bot "));

      SpanParseU64 res_bot = Span_parse_u64(instr, 10);
      assert(res_bot.valid);
      u8 bot = u64_to_u8(res_bot.dat.fst);

      Chips_push(&bot_chips[bot], value);
      if (bot_chips[bot].len == 2) {
        GivingBots_push(&giving_bots, bot);
      }

    } else if (Span_starts_with(line.dat, bot_span)) {
      Span instr = Span_slice(line.dat, bot_span.len, line.dat.len);

      SpanParseU64 res_bot = Span_parse_u64(instr, 10);
      assert(res_bot.valid);
      u8 bot = u64_to_u8(res_bot.dat.fst);

      instr = Span_trim_start(res_bot.dat.snd, Span_from_str(" gives low to "));

      bool low_output;

      if (Span_starts_with(instr, bot_span)) {
        instr = Span_slice(instr, bot_span.len, instr.len);
        low_output = false;
      } else if (Span_starts_with(instr, output_span)) {
        instr = Span_slice(instr, output_span.len, instr.len);
        low_output = true;
      } else {
        panic("Unexpected\n");
      }

      SpanParseU64 res_low = Span_parse_u64(instr, 10);
      assert(res_low.valid);
      u8 low = u64_to_u8(res_low.dat.fst);

      instr = Span_trim_start(res_low.dat.snd, Span_from_str(" and high to "));

      bool high_output;

      if (Span_starts_with(instr, bot_span)) {
        instr = Span_slice(instr, bot_span.len, instr.len);
        high_output = false;
      } else if (Span_starts_with(instr, output_span)) {
        instr = Span_slice(instr, output_span.len, instr.len);
        high_output = true;
      } else {
        panic("Unexpected\n");
      }

      SpanParseU64 res_high = Span_parse_u64(instr, 10);
      assert(res_high.valid);
      u8 high = u64_to_u8(res_high.dat.fst);

      Instr bot_instr = {
          .low = low,
          .high = high,
          .low_output = low_output,
          .high_output = high_output,
      };

      bot_instrs[bot] = bot_instr;
    } else {
      panic("Unexpected\n");
    }

    line = SpanSplitIterator_next(&line_it);
  }

  while (giving_bots.len > 0) {
    GivingBotsPop next = GivingBots_pop(&giving_bots);
    assert(next.valid);

    u8 bot = next.dat;
    Instr instr = bot_instrs[bot];

    ChipsPop a = Chips_pop(&bot_chips[bot]);
    assert(a.valid);

    ChipsPop b = Chips_pop(&bot_chips[bot]);
    assert(b.valid);

    if (bot_chips[bot].len >= 2) {
      // Add it back into the queue
      GivingBots_push(&giving_bots, bot);
    }

    u8 low = a.dat < b.dat ? a.dat : b.dat;
    u8 high = b.dat < a.dat ? a.dat : b.dat;

    if (high == 61 && low == 17) {
      String out = {0};
      String_push_str(&out, "Bot [");
      String_push_u64(&out, bot, 10);
      String_push_str(&out, "]: Low: ");
      String_push_u64(&out, low, 10);
      String_push_str(&out, " | High: ");
      String_push_u64(&out, high, 10);
      String_println(&out);
    }

    if (instr.low_output) {
      Chips_push(&output_chips[instr.low], low);
    } else {
      Chips_push(&bot_chips[instr.low], low);

      if (bot_chips[instr.low].len == 2) {
        GivingBots_push(&giving_bots, instr.low);
      }
    }

    if (instr.high_output) {
      Chips_push(&output_chips[instr.high], high);
    } else {
      Chips_push(&bot_chips[instr.high], high);

      if (bot_chips[instr.high].len == 2) {
        GivingBots_push(&giving_bots, instr.high);
      }
    }
  }

  String out = {0};
  String_push_u64(&out,
                  (usize)output_chips[0].dat[0] *
                      (usize)output_chips[1].dat[0] *
                      (usize)output_chips[2].dat[0],
                  10);
  String_println(&out);
}

int main(void) {
  Span input = Span_from_file("inputs/day10.txt");
  solve(input);

  return 0;
}

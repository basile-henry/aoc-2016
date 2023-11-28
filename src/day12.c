#include "baz.h"

typedef struct {
  enum { Int, Reg } tag;
  union {
    i32 i;
    u8 r;
  } dat;
} IntOrReg;

typedef struct {
  enum {
    Cpy,
    Inc,
    Dec,
    Jnz,
  } tag;
  IntOrReg x;
  IntOrReg y;
} Instr;

static IntOrReg IntOrReg_parse(Span x) {
  SpanParseI64 res = Span_parse_i64(x, 10);
  IntOrReg ret;
  if (res.valid) {
    ret.tag = Int;
    ret.dat.i = (i32)res.dat.fst;
  } else {
    assert(x.len == 1);
    assert(x.dat[0] >= 'a');
    assert(x.dat[0] <= 'd');
    ret.tag = Reg;
    ret.dat.r = (u8)(x.dat[0] - 'a');
  }

  return ret;
}

static Instr Instr_parse(Span line) {
  SpanSplitIterator word_it = Span_split_words(line);

  Span tag_span = UNWRAP(SpanSplitIterator_next(&word_it));
  Span x_span = UNWRAP(SpanSplitIterator_next(&word_it));

  Instr instr = {0};

  if (Span_match(&tag_span, "cpy")) {
    instr.tag = Cpy;
    instr.x = IntOrReg_parse(x_span);

    Span y_span = UNWRAP(SpanSplitIterator_next(&word_it));
    instr.y = IntOrReg_parse(y_span);
    assert(instr.y.tag == Reg);
  } else if (Span_match(&tag_span, "inc")) {
    instr.tag = Inc;
    instr.x = IntOrReg_parse(x_span);
    assert(instr.x.tag == Reg);
  } else if (Span_match(&tag_span, "dec")) {
    instr.tag = Dec;
    instr.x = IntOrReg_parse(x_span);
    assert(instr.x.tag == Reg);
  } else if (Span_match(&tag_span, "jnz")) {
    instr.tag = Jnz;
    instr.x = IntOrReg_parse(x_span);

    Span y_span = UNWRAP(SpanSplitIterator_next(&word_it));
    instr.y = IntOrReg_parse(y_span);
    assert(instr.y.tag == Int);
  } else {
    String out = {0};
    String_push_span(&out, tag_span);
    String_println(&out);
    panic("Unexpected\n");
  }

  return instr;
}

private
void Instr_print(Instr i) {
  String out = {0};
  switch (i.tag) {
  case Cpy: {
    if (i.x.tag == Reg) {
      String_push_str(&out, "Cpy { .x = ");
      String_push(&out, i.x.dat.r + 'a');
      String_push_str(&out, ", .y = ");
      String_push(&out, i.y.dat.r + 'a');
    } else {
      String_push_str(&out, "Cpy { .x = ");
      String_push_i64(&out, i.x.dat.i, 10);
      String_push_str(&out, ", .y = ");
      String_push(&out, i.y.dat.r + 'a');
    }
  } break;
  case Inc: {
    String_push_str(&out, "Inc { .x = ");
    String_push(&out, i.x.dat.r + 'a');
    String_push_str(&out, " }");
  } break;
  case Dec: {
    String_push_str(&out, "Dec { .x = ");
    String_push(&out, i.x.dat.r + 'a');
    String_push_str(&out, " }");
  } break;
  case Jnz:
    if (i.x.tag == Reg) {
      String_push_str(&out, "Jnz { .x = ");
      String_push(&out, i.x.dat.r + 'a');
      String_push_str(&out, ", .y = ");
      String_push_i64(&out, i.y.dat.i, 10);
    } else {
      String_push_str(&out, "Jnz { .x = ");
      String_push_i64(&out, i.x.dat.i, 10);
      String_push_str(&out, ", .y = ");
      String_push_i64(&out, i.y.dat.i, 10);
    }
    break;
  }
  String_println(&out);
}

define_array(Program, Instr, 32);

typedef struct {
  i32 pc;
  i32 regs[4];
} VM;

static void VM_print(const VM *vm) {
  String out = {0};
  String_push_str(&out, "{ .pc = ");
  String_push_i64(&out, vm->pc, 10);
  String_push_str(&out, ", .regs = { ");
  String_push_i64(&out, vm->regs[0], 10);
  String_push_str(&out, ", ");
  String_push_i64(&out, vm->regs[1], 10);
  String_push_str(&out, ", ");
  String_push_i64(&out, vm->regs[2], 10);
  String_push_str(&out, ", ");
  String_push_i64(&out, vm->regs[3], 10);
  String_push_str(&out, ", }, }\n");
  String_print(&out);
}

static void VM_eval(VM *vm, const Program *program) {
  while (vm->pc >= 0 && vm->pc < (i32)program->len) {
    Instr instr = program->dat[vm->pc];

    switch (instr.tag) {
    case Cpy:
      if (instr.x.tag == Reg) {
        vm->regs[instr.y.dat.r] = vm->regs[instr.x.dat.r];
      } else {
        vm->regs[instr.y.dat.r] = instr.x.dat.i;
      }
      break;
    case Inc:
      vm->regs[instr.x.dat.r]++;
      break;
    case Dec:
      vm->regs[instr.x.dat.r]--;
      break;
    case Jnz: {
      int cond = instr.x.tag == Reg ? vm->regs[instr.x.dat.r] : instr.x.dat.i;
      if (cond != 0) {
        vm->pc += instr.y.dat.i - 1;
      }
    } break;
    }

    vm->pc++;
  }
}

static void solve(Span input) {
  SpanSplitIterator line_it = Span_split_lines(input);

  Program program = {0};

  SpanSplitIteratorNext line = SpanSplitIterator_next(&line_it);
  while (line.valid) {
    Instr instr = Instr_parse(line.dat);
    Program_push(&program, instr);

    line = SpanSplitIterator_next(&line_it);
  }

  {
    VM vm = {0};
    VM_eval(&vm, &program);
    VM_print(&vm);
  }

  {
    VM vm = {0};
    vm.regs[2] = 1;
    VM_eval(&vm, &program);
    VM_print(&vm);
  }
}

int main(void) {
  Span example = Span_from_str("cpy 41 a\n"
                               "inc a\n"
                               "inc a\n"
                               "dec a\n"
                               "jnz a 2\n"
                               "dec a\n");
  solve(example);

  Span input = Span_from_file("inputs/day12.txt");
  solve(input);

  return 0;
}

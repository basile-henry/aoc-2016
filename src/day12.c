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
    Span_print(tag_span);
    printf("\n");
    panic("Unexpected\n");
  }

  return instr;
}

private
void Instr_print(Instr i) {
  switch (i.tag) {
  case Cpy: {
    char y[2] = {(char)i.y.dat.r + 'a', 0};
    if (i.x.tag == Reg) {
      char x[2] = {(char)i.x.dat.r + 'a', 0};
      printf("Cpy { .x = %s, .y = %s }\n", x, y);
    } else {
      printf("Cpy { .x = %d, .y = %s }\n", i.x.dat.i, y);
    }
  } break;
  case Inc: {
    char x[2] = {(char)i.x.dat.r + 'a', 0};
    printf("Inc { .x = %s }\n", x);
  } break;
  case Dec: {
    char x[2] = {(char)i.x.dat.r + 'a', 0};
    printf("Dec { .x = %s }\n", x);
  } break;
  case Jnz:
    if (i.x.tag == Reg) {
      char x[2] = {(char)i.x.dat.r + 'a', 0};
      printf("Jnz { .x = %s, .y = %d }\n", x, i.y.dat.i);
    } else {
      printf("Jnz { .x = %d, .y = %d }\n", i.x.dat.i, i.y.dat.i);
    }
    break;
  }
}

define_array(Program, Instr, 32);

typedef struct {
  i32 pc;
  i32 regs[4];
} VM;

static void VM_print(const VM *vm) {
  printf("{ .pc = %d, .regs = { %d, %d, %d, %d, }, }\n", vm->pc, vm->regs[0],
         vm->regs[1], vm->regs[2], vm->regs[3]);
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

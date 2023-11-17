#include "baz.h"
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>

typedef u8 Item; // More like a u4

static inline Item Item_mk(u8 id, bool generator) {
  assert(id < 8);
  return (Item)(id | ((u8)generator << 3));
}

static inline bool Item_generator(Item x) { return (bool)((x >> 3) & 1); }

static inline u8 Item_id(Item x) { return x & 7; }

static void Item_print(Item item) {
  char id[2] = {0};
  id[0] = (char)toupper(Item_id(item) + 'a');

  printf("%s%s", id, Item_generator(item) ? "G" : "M");
}

static inline bool Item_eq(Item a, Item b) {
  return Item_generator(a) == Item_generator(b) && Item_id(a) == Item_id(b);
}

static bool Item_mix(Item a, Item b) {
  return Item_generator(a) != Item_generator(b) && Item_id(a) != Item_id(b);
}

define_bit_set(FloorState, u16, 1);

typedef struct {
  FloorState floors[4];
  u8 elevator;
} State;

static Hash State_hash(const State *s) {
  FxHasher hasher = {0};
  FxHasher_add(&hasher, s->elevator);
  for (usize i = 0; i < 4; i++) {
    FxHasher_add(&hasher, s->floors[i].dat[0]);
  }
  return hasher;
}

static inline bool State_eq(const State *a, const State *b) {
  return a->elevator == b->elevator &&
         a->floors[0].dat[0] == b->floors[0].dat[0] &&
         a->floors[1].dat[0] == b->floors[1].dat[0] &&
         a->floors[2].dat[0] == b->floors[2].dat[0] &&
         a->floors[3].dat[0] == b->floors[3].dat[0];
}

static int State_cmp(const State *a, const State *b) {
  for (int i = 3; i >= 0; i--) {
    usize a_count = FloorState_count(a->floors[i]);
    usize b_count = FloorState_count(b->floors[i]);
    int cmp = usize_cmp(&a_count, &b_count);

    if (cmp != 0) {
      return cmp;
    }
  }

  return 0;
}

static void State_print(const State *state) {
  for (int i = 3; i >= 0; i--) {
    printf("F%d %s ", i + 1, state->elevator == i ? "E" : ".");

    FloorState floor = state->floors[i];
    for (usize j = 0; j < FloorState_size; j++) {
      if (!FloorState_contains(floor, j)) {
        continue;
      }

      Item_print((Item)j);
      printf(" ");
    }
    printf("\n");
  }
}

static u8 ids[26] = {0};
static u8 next_id = 1;

static u8 get_id(u8 c) {
  u8 id = c - 'a';

  if (ids[id] == 0) {
    ids[id] = next_id;
    next_id++;
  }

  return ids[id];
}

static State State_parse(Span input) {
  State state = {0};

  SpanSplitIterator line_it = {
      .rest = input,
      .sep = (u8)'\n',
  };

  usize floor_ix = 0;
  SpanSplitIteratorNext line = SpanSplitIterator_next(&line_it);
  while (line.valid) {
    SpanSplitIterator word_it = {
        .rest = line.dat,
        .sep = (u8)' ',
    };
    SpanSplitIterator_next(&word_it); // The
    SpanSplitIterator_next(&word_it); // <number>
    SpanSplitIterator_next(&word_it); // floor
    SpanSplitIterator_next(&word_it); // contains

    SpanSplitIteratorNext word = SpanSplitIterator_next(&word_it);
    while (word.valid) {
      Span and = Span_from_str("and");
      if (Span_eq(&word.dat, &and)) {
        word = SpanSplitIterator_next(&word_it);
      }

      Span nothing = Span_from_str("nothing");
      if (Span_eq(&word.dat, &nothing)) {
        break;
      }

      Span a = Span_from_str("a");
      if (!Span_eq(&word.dat, &a)) {
        panic("Unexpected\n");
      }
      SpanSplitIteratorNext type = SpanSplitIterator_next(&word_it);
      SpanSplitIteratorNext gen = SpanSplitIterator_next(&word_it);

      u8 id = get_id(type.dat.dat[0]);

      Item item = Item_mk(id, gen.dat.dat[0] == 'g');
      FloorState_insert(&state.floors[floor_ix], item);

      word = SpanSplitIterator_next(&word_it);
    }

    floor_ix += 1;
    line = SpanSplitIterator_next(&line_it);
  }

  return state;
}

static bool State_is_goal(const State *state) {
  for (u8 i = 0; i < 3; i++) {
    if (FloorState_count(state->floors[i]) > 0) {
      return false;
    }
  }

  return true;
}

define_array(MoveItems, Item, 2);

typedef struct {
  MoveItems items;
  bool up;
} Move;

static bool valid_floor(FloorState floor) {
  // generator use the top 8 bits of u16
  FloorState generators = floor;
  generators.dat[0] >>= 8;

  FloorState microchips = floor;
  microchips.dat[0] &= 0xFF;

  return FloorState_is_subset(generators, microchips) ||
         FloorState_is_subset(microchips, generators);
}

static bool try_move(const State *state, State *out, Move move) {
  memset(out, 0, sizeof(State));

  // Needs at least one item in the elevator
  if (move.items.len == 0) {
    return false;
  }

  if (move.items.len == 2) {
    // Can't mix generators and microchips of different types in elevator
    if (Item_mix(move.items.dat[0], move.items.dat[1])) {
      return false;
    }

    // Can't move twice the same item
    if (Item_eq(move.items.dat[0], move.items.dat[1])) {
      return false;
    }
  }

  // Can't move up from floor 4 (ix 3) or down from floor 1 (ix 0)
  if ((state->elevator == 3 && move.up) || (state->elevator == 0 && !move.up)) {
    return false;
  }

  out->elevator = move.up ? state->elevator + 1 : state->elevator - 1;

  // Copy state with item move
  for (usize i = 0; i < 4; i++) {
    out->floors[i] = state->floors[i];

    if (i == state->elevator) {
      FloorState_remove(&out->floors[i], move.items.dat[0]);
      if (move.items.len == 2) {
        FloorState_remove(&out->floors[i], move.items.dat[1]);
      }
    }

    if (i == out->elevator) {
      FloorState_insert(&out->floors[i], move.items.dat[0]);
      if (move.items.len == 2) {
        FloorState_insert(&out->floors[i], move.items.dat[1]);
      }
    }
  }

  // Can't mix generators and microchips of different types on new floor or old
  // floor
  if (!valid_floor(out->floors[out->elevator]) ||
      !valid_floor(out->floors[state->elevator])) {
    return false;
  };

  return true;
}

typedef struct {
  usize moves;
  usize from_ix; // ix in hashmap (which is append only, so the ix is stable)
  Move move;
} Step;

typedef struct {
  usize moves;
  State state;
} MoveState;

static int MoveState_cmp(const MoveState *a, const MoveState *b) {
  // usize a_moves = a->moves;
  // usize b_moves = b->moves;
  // int cmp = usize_cmp(&a_moves, &b_moves);

  // if (cmp != 0) {
  //   return -cmp;
  // }

  return State_cmp(&a->state, &b->state);
}

#define STATE_COUNT (100 * 1024 * 1024)
define_binary_heap(PQ, MoveState, STATE_COUNT, MoveState_cmp);
define_hash_map(BestMoves, State, Step, STATE_COUNT, State_hash, State_eq);

static void print_steps(const BestMoves *bm, usize ix) {

  Step step = bm->values[ix];
  if (step.moves > 1) {
    print_steps(bm, step.from_ix);
  }

  printf("[%zd]: { .up = %d, .items = { ", step.moves, step.move.up);
  for (usize i = 0; i < step.move.items.len; i++) {
    Item_print(step.move.items.dat[i]);
    printf(", ");
  }
  printf("} }\n");

  State_print(&bm->keys[ix]);
  printf("\n");
}

static void solve(State input) {
  PQ *q = (PQ *)calloc(1, sizeof(PQ));
  BestMoves *bm = (BestMoves *)calloc(1, sizeof(BestMoves));

  MoveState m_input = {
      .moves = 0,
      .state = input,
  };
  PQ_insert(q, m_input);
  Step origin = {
      .moves = 0,
  };
  BestMoves_insert(bm, input, origin);

  while (q->len > 0) {
    PQExtract current = PQ_extract(q);
    assert(current.valid);

    usize ix = BestMoves_entry_ix(bm, &current.dat.state);
    assert(bm->occupied[ix]);
    usize moves = bm->values[ix].moves;

    if (State_is_goal(&current.dat.state)) {
      print_steps(bm, ix);

      free(q);
      free(bm);
      return;
    }

    FloorState current_floor =
        current.dat.state.floors[current.dat.state.elevator];
    for (usize i = 0; i < FloorState_size; i++) {
      if (!FloorState_contains(current_floor, i)) {
        continue;
      }

      for (usize j = 0; j < FloorState_size; j++) {
        if (!FloorState_contains(current_floor, j)) {
          continue;
        }
        for (usize k = 0; k < 2; k++) {
          MoveItems items = {0};
          MoveItems_push(&items, (Item)i);
          if (i != j) {
            MoveItems_push(&items, (Item)j);
          }

          Move move = {
              .items = items,
              .up = k == 1,
          };

          State next;
          if (try_move(&current.dat.state, &next, move)) {
            Step step = {
                .moves = 0,
            };
            Step *next_step = BestMoves_insert_modify(bm, next, step);

            if (next_step->moves == 0 || moves + 1 < next_step->moves) {
              next_step->moves = moves + 1;
              next_step->from_ix = ix;
              next_step->move = move;
              MoveState m_next = {
                  .moves = moves + 1,
                  .state = next,
              };
              PQ_insert(q, m_next);
            }
          }
        }
      }
    }
  }
}

int main(void) {
  // State example = State_parse(
  //     Span_from_str("The first floor contains a hydrogen-compatible microchip "
  //                   "and a lithium-compatible microchip.\n"
  //                   "The second floor contains a hydrogen generator.\n"
  //                   "The third floor contains a lithium generator.\n"
  //                   "The fourth floor contains nothing relevant.\n"));

  // printf("Example:\n");
  // State_print(&example);
  // printf("\n");
  // solve(example);

  State input = State_parse(Span_from_file("inputs/day11.txt"));
  printf("Input:\n");
  State_print(&input);
  printf("\n");
  solve(input);

  FloorState_insert(&input.floors[0], Item_mk(get_id('e'), true));
  FloorState_insert(&input.floors[0], Item_mk(get_id('e'), false));
  FloorState_insert(&input.floors[0], Item_mk(get_id('d'), true));
  FloorState_insert(&input.floors[0], Item_mk(get_id('d'), false));

  printf("\n\nModified:\n");
  State_print(&input);
  printf("\n");
  solve(input);

  return 0;
}

#include "baz.h"

typedef struct {
  u16 x;
  u16 y;
} Pos;

static Hash Pos_hash(const Pos *p) {
  FxHasher hasher = {0};
  FxHasher_add(&hasher, p->x);
  FxHasher_add(&hasher, p->y);
  return hasher;
}

static inline bool Pos_eq(const Pos *a, const Pos *b) {
  return a->x == b->x && a->y == b->y;
}

static inline bool is_wall(u16 seed, Pos p) {
  u16 x = p.x;
  u16 y = p.y;
  u16 t = (u16) (x * x + 3u * x + 2u * x * y + y + y * y + seed);
  return __builtin_popcount(t) % 2 == 1;
}

typedef struct {
  usize moves;
  Pos pos;
  Pos goal;
} State;

private inline void State_print(State s) {
  printf("{ .moves = %zd, .pos = { %d, %d }, .goal = { %d, %d }, }\n", s.moves,
         s.pos.x, s.pos.y, s.goal.x, s.goal.y);
}

static inline usize State_manahattan(const State *s) {
  return ABS_DIFF(usize, s->pos.x, s->goal.x) +
         ABS_DIFF(usize, s->pos.y, s->goal.y);
}

static int State_cmp(const State *a, const State *b) {
  int cmp = usize_cmp(&a->moves, &b->moves);
  if (cmp != 0) {
    return -cmp;
  }

  usize manhattan_a = State_manahattan(a);
  usize manhattan_b = State_manahattan(b);
  return -usize_cmp(&manhattan_a, &manhattan_b);
}

#define STATE_COUNT (1024)
define_binary_heap(PriorityQueue, State, STATE_COUNT, State_cmp);
define_hash_map(Cache, Pos, usize, STATE_COUNT, Pos_hash, Pos_eq);

usize solve(u16 seed, Pos goal) {
  PriorityQueue *pq = (PriorityQueue *)calloc(1, sizeof(PriorityQueue));
  Cache *c = (Cache *)calloc(1, sizeof(Cache));

  Pos start = {
      .x = 1,
      .y = 1,
  };
  State initial = {
      .moves = 0,
      .pos = start,
      .goal = goal,
  };

  PriorityQueue_insert(pq, initial);
  Cache_insert(c, start, 0);

  while (pq->len > 0) {
    State current = UNWRAP(PriorityQueue_extract(pq));

    if (current.moves == 50) {
      printf("After 50 moves, visited: %zd\n", c->count);
    }

    if (Pos_eq(&current.pos, &current.goal)) {
      free(pq);
      free(c);
      return current.moves;
    }

    for (int dx = -1; dx <= 1; dx++) {
      if (current.pos.x == 0 && dx == -1) {
        continue;
      }
      u16 x = (u16)((int)current.pos.x + dx);

      for (int dy = -1; dy <= 1; dy += 2) {
        if (current.pos.y == 0 && dy == -1) {
          continue;
        }
        u16 y = dx == 0 ? (u16)((int)current.pos.y + dy) : current.pos.y;
        Pos next = {
            .x = x,
            .y = y,
        };

        if (!is_wall(seed, next)) {
          usize *next_moves = Cache_insert_modify(c, next, 0);

          if (*next_moves == 0 || current.moves + 1 < *next_moves) {
            *next_moves = current.moves + 1;
            State next_state = {
                .moves = *next_moves, .pos = next, .goal = current.goal};
            PriorityQueue_insert(pq, next_state);
          }
        }
      }
    }
  }

  panic("Unexpected\n");
}

int main(void) {
  Pos example = {
      .x = 7,
      .y = 4,
  };
  printf("%zd\n", solve(10, example));

  Pos input = {
      .x = 31,
      .y = 39,
  };
  printf("%zd\n", solve(1352, input));
  return 0;
}

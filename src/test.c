#include "baz.h"

Hash DumbHash(const void *x) { return *(Hash *)x % 2; }

define_array(HMKeys, usize, 4);
define_array(HMValues, u16, 4);

static void test_hash_map(void) {
  // Test Span keys
  {
    HashMap hm = HashMap_alloc(Span_hash, Span_eq, 32);

    Span x_key = Span_from_str("x");
    usize x_val = 42;

    Span y_key = Span_from_str("y");
    usize y_val = 32;

    Span z_key = Span_from_str("z");
    usize z_val = 27;

    HashMap_insert(&hm, &x_key, &x_val);
    assert(*((usize *)HashMap_lookup(hm, &x_key)) == 42);

    HashMap_insert(&hm, &y_key, &y_val);
    assert(*((usize *)HashMap_lookup(hm, &y_key)) == 32);

    HashMap_insert(&hm, &z_key, &z_val);
    assert(*((usize *)HashMap_lookup(hm, &z_key)) == 27);

    HashMap_free(hm);
  }

  {
    HashMap hm = HashMap_alloc(DumbHash, usize_eq, 8);

    usize x = 3;
    HashMap_insert(&hm, &x, &x);

    usize y = 5;
    HashMap_insert(&hm, &y, &y);

    usize z = 7;
    HashMap_insert(&hm, &z, &z);

    usize a = 4;
    HashMap_insert(&hm, &a, &a);

    usize b = 6;
    HashMap_insert(&hm, &b, &b);

    usize c = 8;
    HashMap_insert(&hm, &c, &c);

    usize d = 9;
    HashMap_insert(&hm, &d, &d);

    assert(*(usize *)HashMap_remove(&hm, &a).value == a);

    HashMap_free(hm);
  }

  // Statically allocated HashMap
  {
    HashEntry hm_entries[4] = {0};
    HashMap hm = {
        .get_hash = usize_hash,
        .get_eq = usize_eq,
        .capacity = 4,
        .count = 0,
        .dat = hm_entries,
    };

    HMKeys keys = {.len = 0};
    HMValues values = {.len = 0};

    HashMap_insert(&hm, HMKeys_push(&keys, 12345), HMValues_push(&values, 37));
    HashMap_insert(&hm, HMKeys_push(&keys, 8398), HMValues_push(&values, 12));
    HashMap_insert(&hm, HMKeys_push(&keys, 5432), HMValues_push(&values, 1));
    HashMap_insert(&hm, HMKeys_push(&keys, 6839), HMValues_push(&values, 2));
    HashMap_insert(&hm, &keys.dat[0], &values.dat[1]); // Overwriting key 0
  }
}

int u16_comp(const u16 *a, const u16 *b) {
  return *a < *b ? -1 : *a == *b ? 0 : 1;
}

define_binary_heap(PriorityQueue, u16, 8, u16_comp);

static void test_binary_heap(void) {
  PriorityQueue q = {0};
  PriorityQueue_insert(&q, 42);
  PriorityQueue_insert(&q, 17);
  PriorityQueue_insert(&q, 42);
  PriorityQueue_insert(&q, 83);
  PriorityQueue_insert(&q, 15);
  PriorityQueue_insert(&q, 23);

  {
    PriorityQueueExtract x = PriorityQueue_extract(&q);
    assert(x.valid);
    assert(x.dat == 83);
  }
  {
    PriorityQueueExtract x = PriorityQueue_extract(&q);
    assert(x.valid);
    assert(x.dat == 42);
  }
  {
    PriorityQueueExtract x = PriorityQueue_extract(&q);
    assert(x.valid);
    assert(x.dat == 42);
  }
  {
    PriorityQueueExtract x = PriorityQueue_extract(&q);
    assert(x.valid);
    assert(x.dat == 23);
  }
  {
    PriorityQueueExtract x = PriorityQueue_extract(&q);
    assert(x.valid);
    assert(x.dat == 17);
  }
  {
    PriorityQueueExtract x = PriorityQueue_extract(&q);
    assert(x.valid);
    assert(x.dat == 15);
  }
  {
    PriorityQueueExtract x = PriorityQueue_extract(&q);
    assert(!x.valid);
  }
}

int main(void) {
  test_binary_heap();
  test_hash_map();

  return 0;
}

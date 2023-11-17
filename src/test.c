#include "baz.h"

Hash dumb_hash(const usize *x) { return *x % 2; }

define_hash_map(SpanHashMap, Span, usize, 32, Span_hash, Span_eq);
define_hash_map(DumbHashMap, usize, usize, 8, dumb_hash, usize_eq);

static void test_hash_map(void) {
  // Test Span keys
  {
    SpanHashMap hm = {0};

    Span x_key = Span_from_str("x");
    usize x_val = 42;

    Span y_key = Span_from_str("y");
    usize y_val = 32;

    Span z_key = Span_from_str("z");
    usize z_val = 27;

    {
      SpanHashMap_insert(&hm, x_key, x_val);
      SpanHashMapLookup res = SpanHashMap_lookup(&hm, &x_key);
      assert(res.valid);
      assert(*res.dat == 42);
    }

    {
      SpanHashMap_insert(&hm, y_key, y_val);
      SpanHashMapLookup res = SpanHashMap_lookup(&hm, &y_key);
      assert(res.valid);
      assert(*res.dat == 32);
    }

    {
      SpanHashMap_insert(&hm, z_key, z_val);
      SpanHashMapLookup res = SpanHashMap_lookup(&hm, &z_key);
      assert(res.valid);
      assert(*res.dat == 27);
    }
  }

  {
    // Test heap allocating
    DumbHashMap *hm = (DumbHashMap *)calloc(1, sizeof(DumbHashMap));

    usize x = 3;
    DumbHashMap_insert(hm, x, x);

    usize y = 5;
    DumbHashMap_insert(hm, y, y);

    usize z = 7;
    DumbHashMap_insert(hm, z, z);

    usize a = 4;
    assert(!DumbHashMap_insert(hm, a, a));
    assert(DumbHashMap_insert(hm, a, a - 1)); // Overwriting

    usize b = 6;
    DumbHashMap_insert(hm, b, b);

    usize c = 8;
    DumbHashMap_insert(hm, c, c);

    usize d = 9;
    DumbHashMap_insert(hm, d, d);

    {
      DumbHashMapRemove res = DumbHashMap_remove(hm, &a);
      assert(res.valid);
      assert(res.dat.fst == a);
      assert(res.dat.snd == a - 1);
    }

    {
      DumbHashMapRemove res = DumbHashMap_remove(hm, &a);
      assert(!res.valid);
    }
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

define_bit_set(BitSet, u16, 3);

static void test_bit_set(void) {
  BitSet s = {0};

  BitSet_insert(&s, 42);
  BitSet_insert(&s, 42);
  BitSet_insert(&s, 36);
  assert(BitSet_contains(s, 36));
  assert(BitSet_contains(s, 42));
  assert(!BitSet_contains(s, 27));

  BitSet t = {0};
  assert(BitSet_is_subset(t, s));
  assert(!BitSet_is_subset(s, t));

  BitSet_insert(&t, 36);
  assert(BitSet_is_subset(t, s));

  BitSet_insert(&t, 42);
  assert(BitSet_is_subset(t, s));
  assert(BitSet_is_subset(s, t));
}

int main(void) {
  test_binary_heap();
  test_hash_map();
  test_bit_set();

  return 0;
}

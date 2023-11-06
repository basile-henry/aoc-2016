#include "baz.h"

typedef struct {
  usize count;
  u8 byte;
} ByteCount;

define_array(RoomWords, Span, 8);

typedef struct {
  RoomWords room_words;
  usize sector_id;
  bool is_real;
} Room;

static void Room_decypher(Room room) {
  for (usize i = 0; i < room.room_words.len; i++) {
    Span word = room.room_words.dat[i];

    for (usize j = 0; j < word.len; j++) {
      u8 c = (u8)(((usize) word.dat[j] - (usize) 'a' + room.sector_id) % 26) + 'a';
      putc(c, stdout);
    }

    putc(' ', stdout);
  }

  printf(" %zd\n", room.sector_id);
}

static Room Room_parse(Span room_span) {
  Room room = {0};
  usize char_count[26] = {0};

  while (true) {
    SpanSplitOn section_split = Span_split_on((u8)'-', room_span);

    if (section_split.valid) {
      Span section = section_split.dat.fst;
      RoomWords_push(&room.room_words, section);

      room_span = section_split.dat.snd;

      for (usize i = 0; i < section.len; i++) {
        u8 c = section.dat[i];
        assert(c >= (u8)'a');
        assert(c - 'a' < 26);

        char_count[c - 'a'] += 1;
      }
    } else {
      // Not a section anymore
      Span sector_id_span = {
          .dat = room_span.dat,
          .len = room_span.len - 7,
      };
      Span checksum = {
          .dat = room_span.dat + (room_span.len - 6),
          .len = 5,
      };

      SpanParseU64 sector_id_parse = Span_parse_u64(sector_id_span, 10);
      assert(sector_id_parse.valid);

      usize sector_id = sector_id_parse.dat.fst;

      ByteCount top[5] = {0};

      for (int i = 25; i >= 0; i--) {
        u8 c = 'a' + (u8) i;
        usize n = char_count[i];

        for (int j = 0; j < 5; j++) {
          if (n >= top[j].count) {
            {
              u8 t = top[j].byte;
              top[j].byte = c;
              c = t;
            }

            {
              usize t = top[j].count;
              top[j].count = n;
              n = t;
            }
          }
        }
      }

      assert(checksum.len == 5);

      bool checksum_matches = true;
      for (int j = 0; j < 5; j++) {
        if (top[j].byte != checksum.dat[j]) {
          checksum_matches = false;
          break;
        }
      }

      room.sector_id = sector_id;
      room.is_real = checksum_matches;

      if (checksum_matches) {
        Room_decypher(room);
      }

      return room;
    }
  }
}

void solve(Span data) {
  usize part1 = 0;
  SpanSplitIterator line_it = {
    .rest = data,
    .sep = (u8) '\n',
  };

  SpanSplitIteratorNext line = SpanSplitIterator_next(&line_it);
  while (line.valid) {
    Room room = Room_parse(line.dat);

    if (room.is_real) {
      part1 += room.sector_id;
    }

    line = SpanSplitIterator_next(&line_it);
  }

  printf("%zd\n", part1);
}

int main(void) {
  // Test example
  {
    Span example_span = Span_from_str("aaaaa-bbb-z-y-x-123[abxyz]");
    Room example_room = Room_parse(example_span);
    printf("{ .sector_id = %zd, .is_real = %d, }\n", example_room.sector_id,
           example_room.is_real);
  }

  {
    Span example_span = Span_from_str("a-b-c-d-e-f-g-h-987[abcde]");
    Room example_room = Room_parse(example_span);
    printf("{ .sector_id = %zd, .is_real = %d, }\n", example_room.sector_id,
           example_room.is_real);
  }

  {
    Span example_span = Span_from_str("not-a-real-room-404[oarel]");
    Room example_room = Room_parse(example_span);
    printf("{ .sector_id = %zd, .is_real = %d, }\n", example_room.sector_id,
           example_room.is_real);
  }

  {
    Span example_span = Span_from_str("totally-real-room-200[decoy]");
    Room example_room = Room_parse(example_span);
    printf("{ .sector_id = %zd, .is_real = %d, }\n", example_room.sector_id,
           example_room.is_real);
  }

  Span input = Span_from_file("inputs/day04.txt");
  solve(input);

  return 0;
}

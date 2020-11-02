#include <stdint.h>

// Game paremeters
#define MAX_LEN        140 // 150 ish appears to be the practical limit on the atmega328p
#define START_LEN      4
#define MOV_DELAY      120 // This is effectively the game speed (lower is faster)

// Game area paremeters
#define BLOCK_LEN      3   // x,y sizes must be clampped to this
#define X_GAME_SIZE    90  // Max is 126 based on a BLOCK_LEN of 3
#define Y_GAME_SIZE    90  // Max is 93 based on a BLOCK_LEN of 3
#define X_LOWER_BOUND  18  // Defines the start of game area, as well as the actual coords for physical border
#define Y_LOWER_BOUND  0   // Ditto

// The following are derived  and should not be changed
#define X_OFFSET       (X_LOWER_BOUND + 1)
#define Y_OFFSET       (Y_LOWER_BOUND + 1)
#define X_UPPER_BOUND  (X_OFFSET + X_GAME_SIZE)
#define Y_UPPER_BOUND  (Y_OFFSET + Y_GAME_SIZE)
#define X_GRID_SZE     (X_GAME_SIZE / BLOCK_LEN)
#define Y_GRID_SZE     (Y_GAME_SIZE / BLOCK_LEN)
#define TOT_GRID_SZE   (X_GRID_SZE * Y_GRID_SZE)
#define TAIL_STRT_X    ((X_GRID_SZE - 2) * BLOCK_LEN + Y_OFFSET)
#define TAIL_STRT_Y    (Y_GRID_SZE / 2 * BLOCK_LEN + Y_OFFSET)

// Input related definitions
#define JOY_X_PIN      A0  // Analog X axis
#define JOY_Y_PIN      A1  // Analog Y axis
#define ANALOG_LOW     300 // Lower threshold
#define ANALOG_HIGH    724 // Upper threshold
#define INPUT_SAMPLES  8   // Number of times to check for input per iteration

// Simple macro functions
#define draw_seg(s)    tv.draw_rect(s.x, s.y, (BLOCK_LEN - 1), (BLOCK_LEN - 1), WHITE, WHITE)
#define undraw_seg(s)  tv.draw_rect(s.x, s.y, (BLOCK_LEN - 1), (BLOCK_LEN - 1), BLACK, BLACK)
#define valid_dir(d)   (d && d != snake.dir && (d+5) % 4 + 1 != snake.dir)
#define brdr_col(X, Y) (X <= X_LOWER_BOUND || X >= X_UPPER_BOUND || Y <= Y_LOWER_BOUND || Y >= Y_UPPER_BOUND)

typedef enum {
  NORTH = 1,
  EAST = 2,
  SOUTH = 3,
  WEST = 4,
} direction;


struct Segment {
  uint8_t x;
  uint8_t y;
};


struct Snake {
  direction dir;
  uint8_t len;
  uint8_t tail_idx;
  struct Segment body[MAX_LEN];
};

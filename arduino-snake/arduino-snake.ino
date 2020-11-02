/* Refactoring TODO:

-Replace "byte" with "uint8_t"

-Look into naming of "initialize_game". Probably should be "initialize_snake",
and place_food should be called separately.

-Replace cardinal direction constants with an enum:
typedef enum {
  NO_DIR,
  NORTH,
  EAST,
  SOUTH,
  WEST,
} direction;

*/


#include <TVout.h>

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

// Input/direction definitions
#define NORTH          1
#define EAST           2
#define SOUTH          3
#define WEST           4

// Useful macro functions
#define draw_seg(s)    tv.draw_rect(s.x, s.y, (BLOCK_LEN - 1), (BLOCK_LEN - 1), WHITE, WHITE)
#define undraw_seg(s)  tv.draw_rect(s.x, s.y, (BLOCK_LEN - 1), (BLOCK_LEN - 1), BLACK, BLACK)
#define valid_dir(d)   (d && d != snake.direction && (d+5) % 4 + 1 != snake.direction)
#define brdr_col(X, Y) (X <= X_LOWER_BOUND || X >= X_UPPER_BOUND \
                        || Y <= Y_LOWER_BOUND || Y >= Y_UPPER_BOUND)

struct Segment {
  byte x;
  byte y;
};

struct Snake {
  byte direction;
  byte len;
  byte tail_idx;
  struct Segment body[MAX_LEN];
};

TVout tv;
struct Snake snake;
struct Segment food;


void setup()  {
  randomSeed(analogRead(A5));
  tv.begin(NTSC);

  tv.clear_screen();
  tv.draw_rect(X_LOWER_BOUND, Y_LOWER_BOUND, X_GAME_SIZE+1, Y_GAME_SIZE+1, WHITE, WHITE);
}


void loop() {
  
  tv.delay(1000);
  while(!get_input())
    ;

  tv.clear_screen();
  tv.draw_rect(X_LOWER_BOUND, Y_LOWER_BOUND, X_GAME_SIZE+1, Y_GAME_SIZE+1, WHITE, BLACK);
  initialize_game();

  while(try_movement()) {
    poll_input(MOV_DELAY);
  }
}

void draw_food(struct Segment f) {
  tv.set_pixel(f.x, f.y+2, WHITE);
  tv.set_pixel(f.x+1, f.y+1, WHITE);
  tv.set_pixel(f.x+2, f.y+2, WHITE);
}


void initialize_game() {
  /* Initialize snake and place at starting location */
  snake.len = START_LEN;
  snake.direction = WEST;
  snake.tail_idx = 0;

  for(int i=0; i<START_LEN; i++) {
    snake.body[i].x = TAIL_STRT_X - (i * BLOCK_LEN);
    snake.body[i].y = TAIL_STRT_Y;
    draw_seg(snake.body[i]);
  }

  place_food();
}


void place_food() {
  unsigned int ran_pos = random(TOT_GRID_SZE);
  byte x_try, y_try;
  do {
    x_try = ran_pos % X_GRID_SZE * BLOCK_LEN + X_OFFSET;
    y_try = ran_pos / Y_GRID_SZE * BLOCK_LEN + Y_OFFSET;
    
    ran_pos = ++ran_pos % TOT_GRID_SZE;

  } while (snake_collision(x_try, y_try));

  food.x = x_try;
  food.y = y_try;

  draw_food(food);
}


byte get_input() {
  if (analogRead(JOY_Y_PIN) < ANALOG_LOW)
    return NORTH;
  if (analogRead(JOY_X_PIN) < ANALOG_LOW)
    return EAST;
  if (analogRead(JOY_Y_PIN) > ANALOG_HIGH)
    return SOUTH;
  if (analogRead(JOY_X_PIN) > ANALOG_HIGH)
    return WEST;
  return 0;
}


void poll_input(byte interval) {
    /* Check for input multiple (INPUT_SAMPLES) times per interval */

    byte sub_interval = interval / INPUT_SAMPLES;
    byte input;

    for(int i=0; i<INPUT_SAMPLES; i++) {
      input = get_input();

      if(valid_dir(input)) {
        snake.direction = input;
        tv.delay(interval);
        return;
      }
    interval -= sub_interval;
    tv.delay(sub_interval);
    }
}


void get_next_position(struct Segment *new_seg) {
  /* Sets a segment to the next logical position based on snake direction */
  byte head_idx = (snake.len + snake.tail_idx - 1) % snake.len;
  switch (snake.direction) {
  case NORTH:
    new_seg->x = snake.body[head_idx].x;
    new_seg->y = snake.body[head_idx].y - BLOCK_LEN;
    break;
  case EAST:
    new_seg->x = snake.body[head_idx].x + BLOCK_LEN;
    new_seg->y = snake.body[head_idx].y;
    break;
  case SOUTH:
    new_seg->x = snake.body[head_idx].x;
    new_seg->y = snake.body[head_idx].y + BLOCK_LEN;
    break;
  case WEST:
    new_seg->x = snake.body[head_idx].x - BLOCK_LEN;
    new_seg->y = snake.body[head_idx].y;
    break;
  }
}


bool snake_collision(byte test_x, byte test_y) {
  for(int i=0; i<snake.len; i++) {
    if(snake.body[i].x == test_x && snake.body[i].y == test_y) {
      return true;
    }
  }
  return false;
}


void move_snake(struct Segment new_head) {
  draw_seg(new_head);
  undraw_seg(snake.body[snake.tail_idx]);

  snake.body[snake.tail_idx] = new_head;
  snake.tail_idx = ++snake.tail_idx % snake.len;
}


void grow_snake(struct Segment new_head) {
  draw_seg(new_head);

  for(int i = snake.len; i > snake.tail_idx; i--) {
    snake.body[i] = snake.body[i - 1];
  }
  snake.body[snake.tail_idx++] = new_head;
  snake.len++;
}


bool try_movement() {
  /* Returns false on game over, true otherwise */
  struct Segment test_seg;
  get_next_position(&test_seg);

  // Game over
  if(brdr_col(test_seg.x, test_seg.y) || snake_collision(test_seg.x, test_seg.y)) {
    return false;
  }

  // Ate food
  if(test_seg.x == food.x && test_seg.y == food.y) {
  
    // Game won
    if(snake.len == MAX_LEN) {
      return false;
    }
    
    grow_snake(test_seg);
    place_food();
    return true;
  }

  move_snake(test_seg);
  return true;
}

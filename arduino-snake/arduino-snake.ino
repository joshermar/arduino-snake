#include <TVout.h>
#include "arduino-snake.h"


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
  
  initialize_snake();
  place_food();

  while(try_movement()) {
    poll_input(MOV_DELAY);
  }
}

void draw_food(struct Segment f) {
  tv.set_pixel(f.x, f.y+2, WHITE);
  tv.set_pixel(f.x+1, f.y+1, WHITE);
  tv.set_pixel(f.x+2, f.y+2, WHITE);
}


/* Initialize snake at starting position */
void initialize_snake() {
  snake.len = START_LEN;
  snake.dir = WEST;
  snake.tail_idx = 0;

  for(int i=0; i<START_LEN; i++) {
    snake.body[i].x = TAIL_STRT_X - (i * BLOCK_LEN);
    snake.body[i].y = TAIL_STRT_Y;
    draw_seg(snake.body[i]);
  }
}


void place_food() {
  unsigned int ran_pos = random(TOT_GRID_SZE);
  uint8_t x_try, y_try;
  do {
    x_try = ran_pos % X_GRID_SZE * BLOCK_LEN + X_OFFSET;
    y_try = ran_pos / Y_GRID_SZE * BLOCK_LEN + Y_OFFSET;
    
    ran_pos = ++ran_pos % TOT_GRID_SZE;

  } while (snake_collision(x_try, y_try));

  food.x = x_try;
  food.y = y_try;

  draw_food(food);
}


direction get_input() {
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


void poll_input(uint8_t interval) {
    /* Check for input multiple (INPUT_SAMPLES) times per interval */

    uint8_t sub_interval = interval / INPUT_SAMPLES;
    uint8_t input;

    for(int i=0; i<INPUT_SAMPLES; i++) {
      input = get_input();

      if(valid_dir(input)) {
        snake.dir = input;
        tv.delay(interval);
        return;
      }
    interval -= sub_interval;
    tv.delay(sub_interval);
    }
}


void get_next_position(struct Segment *new_seg) {
  /* Sets a segment to the next logical position based on snake direction */
  uint8_t head_idx = (snake.len + snake.tail_idx - 1) % snake.len;
  switch (snake.dir) {
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


bool snake_collision(uint8_t test_x, uint8_t test_y) {
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

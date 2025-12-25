#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<stdbool.h>
#include<unistd.h>
#include<string.h>
#include<time.h>

//#define TEST
#ifndef TEST
const uint64_t GRID_X = 100001;
const uint64_t GRID_Y = 100001;
const char *INPUT_FILE = "./input.txt";
#else
const uint64_t GRID_X = 12;
const uint64_t GRID_Y = 8;
const char *INPUT_FILE = "./input_test.txt";
#endif

const uint8_t HIT_L = 1 << 0;
const uint8_t HIT_R = 1 << 1;
const uint8_t HIT_T = 1 << 2;
const uint8_t HIT_B = 1 << 3;
const uint8_t BLOCK = 1 << 7;
const uint8_t GOOD_MASK = 0 | HIT_L | HIT_R | HIT_T | HIT_B;

const uint64_t STRIDE = GRID_X;
const uint64_t GRID_TOTAL = GRID_X * GRID_Y;

#define GRID_IDX(x, y) ((y) * (STRIDE) + (x))

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define ABS_DIFF(a, b) ((a) > (b) ? ((a) - (b)) : ((b) - (a)))

#define NANOS_PER_SEC 1000000000

static float timespec_diff_ns(struct timespec *start, struct timespec *end) {
    time_t sec_diff = end->tv_sec - start->tv_sec;

    return sec_diff * NANOS_PER_SEC + (end->tv_nsec - start->tv_nsec);
}

typedef struct {
    uint64_t x;
    uint64_t y;
} vec2;

typedef struct {
    vec2 *data;
    size_t size;
    size_t capacity;
} coords_vec;

void join_range(uint8_t *points, vec2 a, vec2 b) {
    int x1 = a.x, y1 = a.y, x2 = b.x, y2 = b.y;
    if (x1 == x2) {
        for (ssize_t i = MIN(y1, y2); i <= MAX(y1, y2); i++) {
            points[GRID_IDX(x1, i)] |= BLOCK;
        }
    }

    if (y1 == y2) {
        for (ssize_t i = MIN(x1, x2); i <= MAX(x1, x2); i++) {
            points[GRID_IDX(i, y1)] |= BLOCK;
        }
    }
}

bool is_good_square(uint8_t *points, vec2 *c1, vec2 *c2) {
    for (size_t i = MIN(c1->x, c2->x); i <= MAX(c1->x, c2->x); i++) {
        if (((points[GRID_IDX(i, c1->y)] & GOOD_MASK) != GOOD_MASK) ||
                ((points[GRID_IDX(i, c2->y)] & GOOD_MASK) != GOOD_MASK))
            return 0;
    }
    
    for (size_t i = MIN(c1->y, c2->y); i <= MAX(c1->y, c2->y); i++) {
        if (((points[GRID_IDX(c1->x, i)] & GOOD_MASK) != GOOD_MASK) ||
                ((points[GRID_IDX(c2->x, i)] & GOOD_MASK) != GOOD_MASK))
            return 0;
    }
    
    return 1;
}

void compute_pass(uint8_t *points, vec2 curr, vec2 delta, uint8_t mask) {
    bool hit_wall = 0;
    while (curr.x < GRID_X && curr.y < GRID_Y) {
        uint8_t *point = &points[GRID_IDX(curr.x, curr.y)];
        if (!hit_wall && *point & BLOCK)
            hit_wall = 1;

        if (hit_wall)
            *point |= mask;
        
        curr.x += delta.x;
        curr.y += delta.y;
    }
}

void compute_points(uint8_t *points) {
    const uint64_t rtenth = GRID_Y / 10;
    for (size_t row = 0; row < GRID_Y; row++) {
        if (row % rtenth == 0) {
            fprintf(stderr, ".");
            fflush(stdout);
        }

        vec2 start = { 0, row };
        vec2 delta = { 1, 0 };
        compute_pass(points, start, delta, HIT_L);
        start.x = GRID_X - 1;
        delta.x = -1;
        compute_pass(points, start, delta, HIT_R);
    }
   
    // this is very visibly slower than the above part
    //  an interesting example of cpu cache efficiency
    const uint64_t ctenth = GRID_X / 10; 
    for (size_t col = 0; col < GRID_X; col++) {
        if (col % ctenth == 0) {
            fprintf(stderr, ".");
            fflush(stdout);
        }

        vec2 start = { col, 0 };
        vec2 delta = { 0, 1 };
        compute_pass(points, start, delta, HIT_T);
        start.y = GRID_Y - 1;
        delta.y = -1;
        compute_pass(points, start, delta, HIT_B);
    }
}

int main() {
    fprintf(stderr, "* allocating %lu MiB\n", GRID_TOTAL >> 20);
    uint8_t *points = calloc(GRID_TOTAL, sizeof(*points));
    
    fprintf(stderr, "* reading input\n");
    FILE *file = fopen(INPUT_FILE, "r");
    coords_vec coords = {0};
    coords.capacity = 4;
    coords.data = malloc(coords.capacity * sizeof(*coords.data));
    int x = 0, y = 0;
    while (fscanf(file, "%d,%d", &x, &y) == 2) {
        if (coords.size >= coords.capacity) {
            coords.capacity *= 2;
            coords.data = realloc(coords.data, coords.capacity * sizeof(*coords.data));
        }
        coords.data[coords.size++] = (vec2) { x, y };
    }

    for (size_t i = 0; i < coords.size - 1; i++)
        join_range(points, coords.data[i], coords.data[i + 1]);
    join_range(points, coords.data[0], coords.data[coords.size - 1]);

    fprintf(stderr, "* computing points");
    struct timespec start, end;
    clock_gettime(CLOCK_REALTIME, &start);
    compute_points(points);
    clock_gettime(CLOCK_REALTIME, &end);

    float ns = timespec_diff_ns(&start, &end);
    float ms = ns / 1000000;
    float s = ms / 1000;
    fprintf(stderr, " (%.2fs, %.2fms)\n", s, ms);

    fprintf(stderr, "* finding maximum rectangle\n");
    uint64_t mx = 0;
    for (size_t i = 0; i < coords.size - 1; i++) {
        for (size_t j = i + 1; j < coords.size; j++) {
            vec2 *c1 = &coords.data[i];
            vec2 *c2 = &coords.data[j];

            if (!is_good_square(points, c1, c2)) {
                continue;
            }

            uint64_t area = (ABS_DIFF(c1->x, c2->x) + 1) * (ABS_DIFF(c1->y, c2->y) + 1);
            mx = MAX(mx, area);
        }
    }

    printf("%lu\n", mx);

    free(points);
    return 0;
}

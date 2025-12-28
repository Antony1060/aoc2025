typedef struct {
    int x;
    int y;
} svec2;

typedef struct {
    uint x;
    uint y;
} vec2;

typedef struct {
    svec2 curr;
    svec2 delta;
    uchar mask;
} task_unit;

#define GRID_IDX(x, y, stride) (((size_t) (y)) * ((size_t) (stride)) + ((size_t) (x)))

void __kernel grid_pass(__global uchar *points, __global const task_unit *tasks, const svec2 GRID) {
    size_t id = get_global_id(0);

    svec2 delta = tasks[id].delta;
    svec2 curr = tasks[id].curr;

    size_t max_iter = (delta.x) != 0 ? GRID.x : GRID.y;

    bool hit_wall = false;
    for (int i = 0; i < max_iter; i++) {
        int cx = curr.x;
        int cy = curr.y;

        size_t idx = GRID_IDX(cx, cy, GRID.x);

        if (!hit_wall && points[idx] & 0b10000000)
            hit_wall = true;

        if (hit_wall)
            points[idx] |= tasks[id].mask;

        curr.x += delta.x;
        curr.y += delta.y;
    }
}

typedef struct {
    vec2 c1;
    vec2 c2;
} rectangle;

void __kernel rectangle_pass(__global const uchar *points, __global const rectangle *rects, __global ulong *out, const svec2 GRID) {
    size_t id = get_global_id(0);

    vec2 c1 = rects[id].c1;
    vec2 c2 = rects[id].c2;

    const uchar GOOD_MASK = 0b00001111;

    for (size_t i = min(c1.x, c2.x); i <= max(c1.x, c2.x); i++) {
        if (((points[GRID_IDX(i, c1.y, GRID.x)] & GOOD_MASK) != GOOD_MASK) ||
            ((points[GRID_IDX(i, c2.y, GRID.x)] & GOOD_MASK) != GOOD_MASK)) {
            out[id] = 0;
            return;
        }
    }

    for (size_t i = min(c1.y, c2.y); i <= max(c1.y, c2.y); i++) {
        if (((points[GRID_IDX(c1.x, i, GRID.x)] & GOOD_MASK) != GOOD_MASK) ||
            ((points[GRID_IDX(c2.x, i, GRID.x)] & GOOD_MASK) != GOOD_MASK)) {
            out[id] = 0;
            return;
        }
    }

    ulong area = (ulong) (abs_diff(c1.x, c2.x) + 1) * (ulong) (abs_diff(c1.y, c2.y) + 1);
    out[id] = area;
}

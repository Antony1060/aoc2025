typedef struct {
    int x;
    int y;
} svec2;

typedef struct task_unit {
    svec2 curr;
    svec2 delta;
    uchar mask;
} task_unit;

#define GRID_IDX(x, y, stride) ((y) * (stride) + (x))

void __kernel grid_pass(__global uchar *points, __global const task_unit *tasks, const svec2 GRID) {
    size_t id = get_global_id(0);

    svec2 delta = tasks[id].delta;
    svec2 curr = tasks[id].curr;

    size_t max_iter = (delta.x) != 0 ? GRID.x : GRID.y;

    bool hit_wall = false;
    for (int i = 0; i < max_iter; i++) {
        int cx = curr.x;
        int cy = curr.y;

        size_t idx = GRID_IDX((size_t) cx, (size_t) cy, (size_t) GRID.x);

        if (!hit_wall && points[idx] & 0b10000000)
            hit_wall = true;

        if (hit_wall)
            points[idx] |= tasks[id].mask;

        curr.x += delta.x;
        curr.y += delta.y;
    }
}

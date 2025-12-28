#define _GNU_SOURCE

#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<stdbool.h>
#include<unistd.h>
#include<string.h>
#include<time.h>
#include<fcntl.h>
#include<errno.h>

#ifdef GPU
#define CL_TARGET_OPENCL_VERSION 200
#include<CL/cl.h>
#endif

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

const char *OPENCL_PROGRAM_PATH = "./program.cl";

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
#define ARR_LEN(arr) (sizeof(arr) / sizeof(*arr))

#define NANOS_PER_SEC 1000000000

static float timespec_diff_ns(struct timespec *start, struct timespec *end) {
    time_t sec_diff = end->tv_sec - start->tv_sec;

    return sec_diff * NANOS_PER_SEC + (end->tv_nsec - start->tv_nsec);
}

typedef struct {
    char *data;
    size_t size;
    size_t capacity;
} char_vec;

// no error handling
char_vec read_whole_file(const char *path) {
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "open failed");
        exit(1);
    }

    const size_t chunk = 1024;
    char_vec file = {0};
    file.capacity = chunk;
    file.data = malloc(file.capacity * sizeof(*file.data));

    int r;
    while ((r = read(fd, file.data + file.size, chunk)) > 0) {
        file.size += r;
        if (file.size + chunk >= file.capacity) {
            file.capacity *= 2;
            file.data = realloc(file.data, file.capacity * sizeof(*file.data));
        }
    }

    // the loop should have allocated enough for a whole new read to fill up the buffer
    //  at the point read returned 0, the buffer definitely has space for an extra \0
    file.data[file.size] = '\0';

    return file;
}

typedef struct {
    uint32_t x;
    uint32_t y;
} vec2;

typedef struct {
    int32_t x;
    int32_t y;
} svec2;

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
typedef struct {
    svec2 start;
    svec2 delta;
    uint8_t mask;
} task_unit;

void fill_task_units(task_unit *units) {
    size_t curr = 0;

    for (size_t row = 0; row < GRID_Y; row++) {
        svec2 start = { 0, row };
        svec2 delta = { 1, 0 };
        units[curr++] = (task_unit) { start, delta, HIT_L };
    }

    for (size_t row = 0; row < GRID_Y; row++) {
        svec2 start = { GRID_X - 1, row };
        svec2 delta = { -1, 0 };
        units[curr++] = (task_unit) { start, delta, HIT_R };
    }

    // these tasks arevery visibly slower on the cpu than the above ones
    //  an interesting example of cpu cache efficiency
    for (size_t col = 0; col < GRID_X; col++) {
        svec2 start = { col, 0 };
        svec2 delta = { 0, 1 };
        units[curr++] = (task_unit) { start, delta, HIT_T };
    }

    for (size_t col = 0; col < GRID_X; col++) {
        svec2 start = { col, GRID_Y - 1 };
        svec2 delta = { 0, -1 };
        units[curr++] = (task_unit) { start, delta, HIT_B };
    }
}


#ifndef GPU
void compute_pass(uint8_t *points, task_unit unit) {
    svec2 curr = unit.start;
    svec2 delta = unit.delta;

    bool hit_wall = 0;
    while (curr.x < (int64_t) GRID_X && curr.y < (int64_t) GRID_Y && curr.x >= 0 && curr.y >= 0) {
        uint8_t *point = &points[GRID_IDX(curr.x, curr.y)];
        if (!hit_wall && *point & BLOCK)
            hit_wall = 1;

        if (hit_wall)
            *point |= unit.mask;

        curr.x += delta.x;
        curr.y += delta.y;
    }
}

uint64_t compute_answer(coords_vec coords, uint8_t *points) {
    const size_t total_units = 2 * GRID_Y + 2 * GRID_X;
    task_unit *units = malloc(total_units * sizeof(*units));
    // would be notifibly faster if we called compute_pass without filling an array first
    //  I assume cache hits, but idc, the point of this project is more running it on a gpu than cpu
    fill_task_units(units);

    fprintf(stderr, "** computing points");
    struct timespec start, end;
    clock_gettime(CLOCK_REALTIME, &start);

    const uint64_t tenth = total_units / 10;
    for (size_t i = 0; i < total_units; i++) {
        if (i % tenth == 0) {
            fprintf(stderr, ".");
            fflush(stdout);
        }

        compute_pass(points, units[i]);
    }

    clock_gettime(CLOCK_REALTIME, &end);

    fprintf(stderr, "\n");
    free(units);

    {
        float ns = timespec_diff_ns(&start, &end);
        float ms = ns / 1000000;
        float s = ms / 1000;
        fprintf(stderr, "** timings: compute (%.2fs, %.2fms)\n", s, ms);
    }

    fprintf(stderr, "** finding maximum rectangle\n");
    clock_gettime(CLOCK_REALTIME, &start);
    uint64_t mx = 0;
    for (size_t i = 0; i < coords.size - 1; i++) {
        for (size_t j = i + 1; j < coords.size; j++) {
            vec2 *c1 = &coords.data[i];
            vec2 *c2 = &coords.data[j];

            if (!is_good_square(points, c1, c2)) {
                continue;
            }

            uint64_t area = (uint64_t) (ABS_DIFF(c1->x, c2->x) + 1) * (uint64_t) (ABS_DIFF(c1->y, c2->y) + 1);
            mx = MAX(mx, area);
        }
    }
    clock_gettime(CLOCK_REALTIME, &end);

    {
        float ns = timespec_diff_ns(&start, &end);
        float ms = ns / 1000000;
        float s = ms / 1000;
        fprintf(stderr, "** timings: lookup (%.2fs, %.2fms)\n", s, ms);
    }

    return mx;
}
#else
void cl_context_callback(const char *errinfo, const void *private_info, size_t cb, void* user_data) {
    (void) private_info; (void) cb; (void) user_data;
    fprintf(stderr, "** ! opencl error: %s\n", errinfo);
}

cl_int err;
#define CL_ERRQUIT_CHECK(detail) do { \
    if (err != CL_SUCCESS) { \
        fprintf(stderr, "** ! error: %s: %d\n", detail, err); \
        return RET; \
    } \
} while (0)

void run_kernel(const char *name, task_unit *units, size_t unit_len, cl_command_queue queue, cl_kernel kernel, cl_mem tasks_buffer, cl_event *dep, cl_event *event) {
#define RET
    fprintf(stderr, "*** queueing sub-task: %s\n", name);

    cl_event write;
    err = clEnqueueWriteBuffer(queue, tasks_buffer, CL_FALSE, 0, unit_len * sizeof(task_unit), units, (dep == NULL ? 0 : 1), dep, &write);
    CL_ERRQUIT_CHECK("clEnqueueWriteBuffer");

    cl_event run;
    err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &unit_len, NULL, 1, &write, &run);
    CL_ERRQUIT_CHECK("clEnqueueNDRangeKernel");

    *event = run;
#undef RET
}

uint64_t compute_answer(coords_vec coords, uint8_t *points) {
#define RET 1
    // this is gonna be a long function lmao

    // platform scan
    cl_uint n_platforms = 0;
    // assume max of 8
    cl_platform_id platforms[8];

    err = clGetPlatformIDs(ARR_LEN(platforms), platforms, &n_platforms);
    CL_ERRQUIT_CHECK("clGetPlatformIDs");

    // assume first platform
    if (n_platforms == 0) {
       fprintf(stderr, "** ! no platforms found");
       return 0;
    }

    if (n_platforms > 1) {
       fprintf(stderr, "** warn: multiple platforms found, choosing first");
    }

    cl_platform_id platform = platforms[0];

    char plat_name[256];
    err = clGetPlatformInfo(platform, CL_PLATFORM_NAME, ARR_LEN(plat_name), plat_name, NULL);
    CL_ERRQUIT_CHECK("clGetPlatformInfo");

    fprintf(stderr, "** using platform: %s\n", plat_name);

    // device scan
    cl_uint n_devices = 0;
    // assume max of 8
    cl_device_id devices[8];

    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, ARR_LEN(devices), devices, &n_devices);
    CL_ERRQUIT_CHECK("clGetDeviceIDs");

    // assume first device
    if (n_devices == 0) {
       fprintf(stderr, "** ! no devices found");
       return 0;
    }

    if (n_devices > 1) {
       fprintf(stderr, "** warn: multiple devices found, choosing first");
    }

    cl_device_id device = devices[0];

    char device_name[256];
    err = clGetDeviceInfo(device, CL_DEVICE_NAME, ARR_LEN(device_name), plat_name, NULL);
    CL_ERRQUIT_CHECK("clGetDeviceInfo");

    fprintf(stderr, "** using device: %s\n", plat_name);

    // context
    cl_context ctx = clCreateContext(NULL, 1, &device, cl_context_callback, NULL, &err);
    CL_ERRQUIT_CHECK("clCreateContext");

    // program loading/compiling
    char_vec file = read_whole_file(OPENCL_PROGRAM_PATH);
    fprintf(stderr, "** read file (%zu bytes)\n", file.size);

    fprintf(stderr, "** preparing and compiling kernel\n");
    cl_program program = clCreateProgramWithSource(ctx, 1, (const char **) &file.data, &file.size, &err);
    CL_ERRQUIT_CHECK("clCreateProgramWithSource");

    err = clBuildProgram(program, 1, &device, /* options */ NULL, /* notify cb */ NULL, /* user_data */NULL);
    if (err != CL_SUCCESS) {
        char build_log[10240];

        err = clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, ARR_LEN(build_log), build_log, NULL);
        CL_ERRQUIT_CHECK("clGetProgramBuildInfo");

        fprintf(stderr, "build log:\n%s\n\n", build_log);
    }

    // queue
    fprintf(stderr, "** preparing buffers\n");
    cl_command_queue queue = clCreateCommandQueueWithProperties(ctx, device, NULL, &err);
    CL_ERRQUIT_CHECK("clCreateCommandQueue");

    // compute tasks
    const size_t total_units = 2 * GRID_Y + 2 * GRID_X;

    const size_t points_sz = GRID_TOTAL * sizeof(uint8_t);
    const size_t tasks_sz = total_units * sizeof(task_unit);

    task_unit *units = malloc(tasks_sz);
    fill_task_units(units);

    // initialize buffers
    cl_mem points_buffer = clCreateBuffer(ctx, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, points_sz, points, &err);
    CL_ERRQUIT_CHECK("clCreateBuffer: points");

    cl_mem tasks_buffer = clCreateBuffer(ctx, CL_MEM_READ_ONLY, MAX(GRID_Y, GRID_X) * sizeof(task_unit), NULL, &err);
    CL_ERRQUIT_CHECK("clCreateBuffer: tasks");

    // create kernel
    fprintf(stderr, "** creating kernel\n");
    cl_kernel grid_pass_kernel = clCreateKernel(program, "grid_pass", &err);
    CL_ERRQUIT_CHECK("clCreateKernel");

    // set args
    err = clSetKernelArg(grid_pass_kernel, 0, sizeof(points_buffer), &points_buffer);
    CL_ERRQUIT_CHECK("clSetKernelArg: points_buffer");

    err = clSetKernelArg(grid_pass_kernel, 1, sizeof(tasks_buffer), &tasks_buffer);
    CL_ERRQUIT_CHECK("clSetKernelArg: tasks_buffer");

    svec2 grid = {
        .x = (int32_t) GRID_X,
        .y = (int32_t) GRID_Y,
    };
    err = clSetKernelArg(grid_pass_kernel, 2, sizeof(svec2), &grid);
    CL_ERRQUIT_CHECK("clSetKernelArg: grid_vec");

    fprintf(stderr, "*** starting tasks\n");

    // l -> r pass
    cl_event lr;
    run_kernel("left -> right", units, GRID_Y, queue, grid_pass_kernel, tasks_buffer, NULL, &lr);
    // r -> l pass
    cl_event rl;
    run_kernel("right -> left", units + GRID_Y, GRID_Y, queue, grid_pass_kernel, tasks_buffer, &lr, &rl);
    // t -> b pass
    cl_event tb;
    run_kernel("top -> bottom", units + 2 * GRID_Y, GRID_X, queue, grid_pass_kernel, tasks_buffer, &rl, &tb);
    // b -> t pass
    cl_event bt;
    run_kernel("bottom -> top", units + 2 * GRID_Y + GRID_X, GRID_X, queue, grid_pass_kernel, tasks_buffer, &tb, &bt);

    struct timespec start, end;
    clock_gettime(CLOCK_REALTIME, &start);

    err = clWaitForEvents(1, &bt);
    CL_ERRQUIT_CHECK("clWaitForEvents");

    err = clFlush(queue);
    CL_ERRQUIT_CHECK("clFlush");

    clock_gettime(CLOCK_REALTIME, &end);

    {
        float ns = timespec_diff_ns(&start, &end);
        float ms = ns / 1000000;
        float s = ms / 1000;
        fprintf(stderr, "** timings: compute (%.2fs, %.2fms)\n", s, ms);
    }

    err = clReleaseMemObject(tasks_buffer);
    CL_ERRQUIT_CHECK("clReleaseMemObject: tasks");

    free(units);

    typedef struct {
        vec2 c1;
        vec2 c2;
    } rect;
    rect *rects = malloc(coords.size * coords.size * sizeof(rect));

    size_t cnt = 0;
    for (size_t i = 0; i < coords.size - 1; i++) {
        for (size_t j = i + 1; j < coords.size; j++) {
            rects[cnt++] = (rect) {
                .c1 = coords.data[i],
                .c2 = coords.data[j],
            };
        }
    }

    size_t rects_sz = cnt * sizeof(rect);
    size_t out_sz = cnt * sizeof(uint64_t);

    cl_mem rects_buffer = clCreateBuffer(ctx, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, rects_sz, rects, &err);
    CL_ERRQUIT_CHECK("clCreateBuffer: rects");

    cl_mem areas_buffer = clCreateBuffer(ctx, CL_MEM_WRITE_ONLY, rects_sz, NULL, &err);
    CL_ERRQUIT_CHECK("clCreateBuffer: areas");

    // create kernel
    fprintf(stderr, "** creating kernel\n");
    cl_kernel rect_kernel = clCreateKernel(program, "rectangle_pass", &err);
    CL_ERRQUIT_CHECK("clCreateKernel");

    // set args
    err = clSetKernelArg(rect_kernel, 0, sizeof(points_buffer), &points_buffer);
    CL_ERRQUIT_CHECK("clSetKernelArg: points_buffer");

    err = clSetKernelArg(rect_kernel, 1, sizeof(rects_buffer), &rects_buffer);
    CL_ERRQUIT_CHECK("clSetKernelArg: rects_buffer");

    err = clSetKernelArg(rect_kernel, 2, sizeof(areas_buffer), &areas_buffer);
    CL_ERRQUIT_CHECK("clSetKernelArg: areas_buffer");

    err = clSetKernelArg(rect_kernel, 3, sizeof(svec2), &grid);
    CL_ERRQUIT_CHECK("clSetKernelArg: grid_vec");

    cl_event run;
    err = clEnqueueNDRangeKernel(queue, rect_kernel, 1, NULL, &cnt, NULL, 0, NULL, &run);
    CL_ERRQUIT_CHECK("clEnqueueNDRangeKernel");

    err = clFlush(queue);
    CL_ERRQUIT_CHECK("clFlush");

    uint64_t *outs = malloc(out_sz);

    clock_gettime(CLOCK_REALTIME, &start);

    fprintf(stderr, "** reading buffer\n");
    err = clEnqueueReadBuffer(queue, areas_buffer, CL_TRUE, 0, out_sz, outs, 1, &run, NULL);
    CL_ERRQUIT_CHECK("clEnqueueReadBuffer");

    uint64_t mx = 0;
    for (size_t i = 0; i < cnt; i++) {
        mx = MAX(mx, outs[i]);
    }

    clock_gettime(CLOCK_REALTIME, &end);

    {
        float ns = timespec_diff_ns(&start, &end);
        float ms = ns / 1000000;
        float s = ms / 1000;
        fprintf(stderr, "** timings: lookup (%.2fs, %.2fms)\n", s, ms);
    }

    err = clReleaseMemObject(points_buffer);
    CL_ERRQUIT_CHECK("clReleaseMemObject: points");

    err = clReleaseMemObject(rects_buffer);
    CL_ERRQUIT_CHECK("clReleaseMemObject: rects");

    err = clReleaseMemObject(areas_buffer);
    CL_ERRQUIT_CHECK("clReleaseMemObject: areas");

    return mx;
#undef RET
}
#undef CL_ERRQUIT_CHECK
#endif

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

    fprintf(stderr, "* computing points\n");
    struct timespec start, end;
    clock_gettime(CLOCK_REALTIME, &start);
    uint64_t mx = compute_answer(coords, points);
    clock_gettime(CLOCK_REALTIME, &end);

    printf("%lu\n", mx);

    free(points);
    return 0;
}

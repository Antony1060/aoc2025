CFLAGS=-march=native -O3 -ggdb -Wall -Wextra -Werror

.PHONY: default
default: all

all: main-cpu main-gpu

main-cpu: main.c
	gcc $(CFLAGS) $? -o $@

main-gpu: main.c
	gcc $(CFLAGS) -lOpenCL -DGPU $? -o $@

clean:
	rm main-cpu main-gpu

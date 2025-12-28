CFLAGS=-march=native -O3 -ggdb -Wall -Wextra -Werror

main-cpu: main.c
	gcc $(CFLAGS) $? -o $@

main-gpu: main.c
	gcc $(CFLAGS) -lOpenCL -DGPU $? -o $@

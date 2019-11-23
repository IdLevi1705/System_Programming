
MT: multitest_proc.c multitest_thread.c searchtest.c
	gcc -m32 -o multitest_t multitest_proc.c multitest_thread.c searchtest.c -I. -lm -ldl -pthread -DMULTITHREAD

MP: multitest_proc.c multitest_thread.c searchtest.c
	gcc -m32 -o multitest_p multitest_proc.c multitest_thread.c searchtest.c -I. -lm -ldl -pthread -DMULTIPROCESS


.PHONY: clean

clean:
	rm -f multitest_p multitest_t *.o
	
	 
all: MP MT



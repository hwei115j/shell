shell: main.c queue.c
	gcc main.c queue.c -lreadline -o shell
clean:
	rm -f *.exe* *.o *.out*


exe: main.c archke_event_loop_epoll.c archke_simple_string_reader.c
	gcc -Wall -c $^
	gcc *.o -o exe
	rm *.o

.PHONY: clean
clean:
	rm exe


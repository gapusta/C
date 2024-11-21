
exe: main.c archke_event_loop_epoll.c archke_event_handlers.c archke_socket.c archke_simple_string_reader.c
	gcc -Wall -c $^
	gcc *.o -o exe
	rm *.o

dexe: main.c archke_event_loop_epoll.c archke_event_handlers.c archke_socket.c archke_simple_string_reader.c
	gcc -Wall -c -g $^
	gcc *.o -o exed
	rm *.o

.PHONY: clean dclean
clean:
	rm exe

dclean: 
	rm exed


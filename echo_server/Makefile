
exe: main.c archke_event_loop_epoll.c archke_socket.c archke_simple_string_reader.c
	gcc -Wall -c $^
	gcc *.o -o exe
	rm *.o

exed: main.c archke_event_loop_epoll.c archke_socket.c archke_simple_string_reader.c
	gcc -Wall -c -g $^
	gcc *.o -o exed
	rm *.o

.PHONY: clean
clean:
	rm exe

cleand: 
	rm exed


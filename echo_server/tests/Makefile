.PHONY: clean cleand

exe: ../archke_simple_string_reader.c archke_simple_string_reader_tests.c
	gcc -Wall -I.. -c $^
	gcc *.o -o exe
	rm *.o

exed: ../archke_simple_string_reader.c archke_simple_string_reader_tests.c
	gcc -Wall -I.. -c -g $^
	gcc *.o -o exed
	rm *.o

clean:
	rm exe

cleand: 
	rm exed


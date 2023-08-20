
main.o: board.o bitboards.o move.o
	ld -r board.o bitboards.o move.o -o main.o

board.o: board.cpp board.h
	g++ -std=c++11 -c board.cpp

bitboards.o: bitboards.cpp bitboards.h
	g++ -std=c++11 -c bitboards.cpp

move.o: move.cpp move.h 
	g++ -std=c++11 -c move.cpp

all_headers.h:
	for i in `ls *.h`; do echo "#include \"$$i\"" >>all_headers.h; done

clean: 
	rm *.o
	rm all_headers.h
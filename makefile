CC = g++ -std=c++11

all: main-process trait user

main-process: main-process.o
	${CC} main-process.o -o main-process

trait: trait.o
	${CC} trait.o -o trait

user: user.o
	${CC} user.o -o user

main-process.o: main-process.cpp
	${CC} -c main-process.cpp

trait.o: trait.cpp
	${CC} -c trait.cpp

user.o: user.cpp
	${CC} -c user.cpp

.PHONY: clean
clean:
	rm *.o
	rm main-process
	rm trait
	rm user
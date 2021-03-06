CC=clang++
CFLAGS=-O3 -Wall -march=native -std=c++14 -I/Users/shoumikpalkar/work/weld/c -L/Users/shoumikpalkar/work/weld/target/debug -lweld

.PHONY: all llvm asm clean java

all:
	$(CC) $(CFLAGS) filters.cpp -o bench

java:
	javac Filters.java

llvm:
	$(CC) $(CFLAGS) -S -emit-llvm filters.cpp

asm:
	$(CC) $(CFLAGS) -S filters.cpp

clean:
	rm -rf bench filters.ll filters.S *.class

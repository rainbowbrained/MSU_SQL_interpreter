CC = g++
CFLAGS = -std=c++17 -O2 -ftrapv -fsanitize=undefined -Wall -Werror -Wformat-security -Wignored-qualifiers -Winit-self -Wswitch-default -Wfloat-equal -Wshadow -Wpointer-arith -Wtype-limits -Wempty-body -Wmissing-field-initializers -Wcast-qual -Wwrite-strings -Wuninitialized
LINK_COMPILER_FLAGS = -fsanitize=undefined -g -o

all: server client spm

server: server.o dbms.o 
	$(CC) $(CFLAGS) $(LINK_COMPILER_FLAGS) $@ $^ 
	
client: client.o dbms.o 
	$(CC) $(CFLAGS) $(LINK_COMPILER_FLAGS) $@ $^ 
	
spm: tmp.o dbms.o 
	$(CC) $(LINK_COMPILER_FLAGS) $@ $^ 
	
tmp.o: tmp.cpp dbms.hpp 
	$(CC) $(CFLAGS) -g -c tmp.cpp dbms.hpp 
	
interpret.o: dbms.hpp 
	$(CC) $(CFLAGS) -g -c interpret.hpp dbms.hpp 
	
dbms.o: dbms.cpp dbms.hpp
	$(CC) $(CFLAGS) -g -c dbms.cpp -o $@
	
clear:
	rm *.o *.gch client server spm

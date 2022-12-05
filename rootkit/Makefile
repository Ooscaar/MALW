all: rootkit.so

rootkit.so: rootkit.c
	gcc -Wall -fPIC -shared -o rootkit.so rootkit.c -ldl

.PHONY clean:
	rm -f rootkit.so	


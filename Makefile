macaron: macaron.c
	gcc -o macaron macaron.c -lX11 -lImlib2

clean:
	rm -f /usr/local/bin/macaron

install: macaron
	mv macaron /usr/local/bin

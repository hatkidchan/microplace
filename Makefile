
all: client server

clean: clean-client clean-server

client: src/Makefile.client
	make -C src -f Makefile.client client.elf

server: src/Makefile.server
	make -C src -f Makefile.server server.elf

clean-client: src/Makefile.client
	make -C src -f Makefile.client clean

clean-server: src/Makefile.server
	make -C src -f Makefile.server clean


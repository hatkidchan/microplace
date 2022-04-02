
all: client server

clean: clean-client clean-server

client: src/Makefile.client
	make -C src -f Makefile.client client.elf
	cp src/client.elf ./

server: src/Makefile.server
	make -C src -f Makefile.server server.elf
	cp src/server.elf ./

clean-client: src/Makefile.client
	make -C src -f Makefile.client clean
	$(RM) ./client.elf

clean-server: src/Makefile.server
	make -C src -f Makefile.server clean
	$(RM) ./server.elf


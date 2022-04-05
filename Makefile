
all: client server

clean: clean-client clean-server

client: src/Makefile.client
	make -C src -f Makefile.client all
	make -C src -f Makefile.client filename | grep -P '^> ' \
		| awk '{print"src/"$$2}' | xargs cp -t .

server: src/Makefile.server
	make -C src -f Makefile.server all
	cp src/server.elf ./

clean-client: src/Makefile.client
	make -C src -f Makefile.client clean
	$(RM) ./client.elf

clean-server: src/Makefile.server
	make -C src -f Makefile.server clean
	$(RM) ./server.elf


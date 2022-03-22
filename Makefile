CLIBS := 
LDFLAGS_CLIENT := -lraylib -lm
CFLAGS := -Wall -Wextra -fshort-enums
MGFLAGS := -DMG_ENABLE_LOG=1 -DMG_ENABLE_LINES=1
INCLUDES := -I./src
INCLUDES_CLIENT := $(INCLUDES) -I./raygui/src
PACKED_FILES := client.html client.js client.wasm

CFLAGS := $(CFLAGS) $(MGFLAGS)

OBJS_SERVER := obj/srv_world.o obj/srv_handlers.o \
							 obj/srv_pksender.o obj/utils.o \
							 obj/mongoose.o obj/filesystem.o

OBJS_CLIENT := obj/mongoose-client.o \
			   obj/cli_networking.o obj/cli_pksender.o \
			   obj/utils-client.o obj/cli_state.o obj/raygui.o \
				 obj/cli_pkhandlers.o

all: client server

clean: clean-client clean-server
	$(RM) obj/*.*
	
pack: pack.c
	$(CC) -o $@ $^

src/filesystem.c: pack Makefile
	./pack $(PACKED_FILES) > src/filesystem.c

clean-server:
	$(RM) server obj/srv*.o obj/mongoose.o obj/raygui.c

clean-client:
	$(RM) client* $(OBJS_CLIENT)
	
client: src/client.c $(OBJS_CLIENT)
	$(CC) -o client src/client.c $(OBJS_CLIENT) $(CFLAGS) $(INCLUDES_CLIENT) $(LDFLAGS_CLIENT)

server: src/server.c obj/filesystem.o $(OBJS_SERVER)
	$(CC) -o server src/server.c $(OBJS_SERVER) $(CFLAGS) $(INCLUDES) $(CLIBS)
	
obj/mongoose.o: src/mongoose.c
	$(CC) -c -o $@ $^ $(CFLAGS) $(INCLUDES) -DMG_ENABLE_PACKED_FS=1
	
obj/mongoose-client.o: src/mongoose.c
	$(CC) -c -o $@ $^ $(CFLAGS) $(INCLUDES_CLIENT)
	
obj/utils-client.o: src/utils.c
	$(CC) -c -o $@ $^ $(CFLAGS) $(INCLUDES_CLIENT)

obj/raygui.o: obj/raygui.c
	$(CC) -c -o $@ $^ $(CFLAGS) $(INCLUDES_CLIENT) -DRAYGUI_IMPLEMENTATION

obj/raygui.c: raygui/src/raygui.h
	cp $^ $@

obj/cli_%.o: src/cli_%.c
	$(CC) -c -o $@ $^ $(CFLAGS) $(INCLUDES_CLIENT)

obj/%.o: src/%.c
	$(CC) -c -o $@ $^ $(CFLAGS) $(INCLUDES)


CLIBS := 
LDFLAGS_CLIENT := -lraylib
CFLAGS := -Wall -Wextra -fshort-enums
MGFLAGS := -DMG_ENABLE_LOG=1 -DMG_ENABLE_LINES=1
INCLUDES := -I./src

CFLAGS := $(CFLAGS) $(MGFLAGS)

OBJS_SERVER := obj/srv_world.o obj/srv_handlers.o \
							 obj/srv_pksender.o obj/utils.o \
							 obj/mongoose.o

OBJS_CLIENT := obj/mongoose-client.o \
			   obj/cli_networking.o obj/cli_pksender.o \
			   obj/utils-client.o

all: client server

clean: clean-client clean-server

clean-server:
	$(RM) server obj/srv*.o obj/mongoose.o

clean-client:
	$(RM) client* obj/cli*.o obj/mongoose-client.o
	
client: $(OBJS_CLIENT)
	$(CC) -o client src/client.c $(OBJS_CLIENT) $(CFLAGS) $(INCLUDES) $(LDFLAGS_CLIENT)

server: $(OBJS_SERVER)
	$(CC) -o server src/server.c $(OBJS_SERVER) $(CFLAGS) $(INCLUDES) $(CLIBS)
	
obj/mongoose.o:
	$(CC) -c -o obj/mongoose.o src/mongoose.c $(CFLAGS) $(INCLUDES)
	
obj/mongoose-client.o:
	$(CC) -c -o obj/mongoose-client.o src/mongoose.c $(CFLAGS) $(INCLUDES)
	
obj/utils-client.o: src/utils.c
	$(CC) -c -o $@ $^ $(CFLAGS) $(INCLUDES)

obj/%.o: src/%.c
	$(CC) -c -o $@ $^ $(CFLAGS) $(INCLUDES)


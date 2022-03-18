CLIBS := 
CFLAGS := -Wall -Wextra -fshort-enums
MGFLAGS := -DMG_ARCH=MG_ARCH_UNIX -DMG_ENABLE_LOG=1 -DMG_ENABLE_LINES=1
INCLUDES := -I./src

CFLAGS := $(CFLAGS) $(MGFLAGS)

OBJS_SERVER := obj/srv_world.o obj/srv_handlers.o \
							 obj/srv_pksender.o obj/utils.o \
							 obj/mongoose.o

all: server

clean:
	$(RM) server
	$(RM) obj/*.o

server: $(OBJS_SERVER)
	$(CC) -o server src/server.c $(OBJS_SERVER) $(CFLAGS) $(INCLUDES) $(CLIBS)
	
obj/mongoose.o:
	$(CC) -c -o obj/mongoose.o src/mongoose.c $(CFLAGS) $(INCLUDES)
	
obj/%.o: src/%.c
	$(CC) -c -o $@ $^ $(CFLAGS) $(INCLUDES)

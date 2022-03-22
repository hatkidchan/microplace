set -xe

build_obj() {
  emcc -o "$2" -c "$1" -O0 -Wall -DPLATFORM_WEB -I./src \
    -I/home/hkc/projects/c/raylib-stuff/raylib \
    -fshort-enums \
    -sASSERTIONS=2 -sSAFE_HEAP=1;
}
echo '
emcc -v -o "$1.html" "$1" \
  -Wall -O0 \
  -L/home/hkc/projects/c/raylib-stuff/raylib \
  /home/hkc/projects/c/raylib-stuff/raylib/libraylib.a \
  /home/hkc/projects/c/raylib-stuff/raylib/libraygui.a \
  -s USE_GLFW=3 -s ASYNCIFY=1 -s ALLOW_MEMORY_GROWTH=1 \
  -s DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR=0 \
  -DPLATFORM_WEB --shell-file shell.html \
  -lwebsocket.js;
' > /dev/null

build_obj "src/cli_networking.c" "obj/cli_networking.o"
build_obj "src/cli_pksender.c" "obj/cli_pksender.o"
build_obj "src/utils.c" "obj/utils-client.o"
build_obj "src/cli_state.c" "obj/cli_state.o"
build_obj "src/cli_pkhandlers.c" "obj/cli_pkhandlers.o"

emcc -v -o "client.html" "src/client.c" \
  -Wall -O0 \
  obj/cli_networking.o \
  obj/cli_pksender.o \
  obj/utils-client.o \
  obj/cli_state.o \
  obj/cli_pkhandlers.o \
  -fshort-enums \
  -I/home/hkc/projects/c/raylib-stuff/raylib \
  /home/hkc/projects/c/raylib-stuff/raylib/libraylib.a \
  /home/hkc/projects/c/raylib-stuff/raylib/libraygui.a \
  -s USE_GLFW=3 -s ASYNCIFY=1 -s ALLOW_MEMORY_GROWTH=1 \
  -s DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR=0 \
  -DPLATFORM_WEB \
  --shell-file src/shell.html \
  -lwebsocket.js;

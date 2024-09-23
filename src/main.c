#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <lua5.3/lua.h>
#include <lua5.3/lualib.h>
#include <lua5.3/lauxlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <string.h>
#include <unistd.h>



typedef struct{
  int x,y; 
} IntVec2_T;

typedef struct{
  uint x,y; 
} UIntVec2_T;

Display *display;
lua_State *L;
int screen;
Window root;

void runLuaScript(const char *script);
void spawn_terminal();
void close_window(Window window);
void move_window(Window window,IntVec2_T pos);
void resize_window(Window window,UIntVec2_T size);
void toggle_floatign(Window window);
void handle_keypress(XKeyEvent *ev);
void launch_App(const char *script);


int main(int argc, char *argv[])
{
  display = XOpenDisplay(NULL);
  if(display == NULL){
    fprintf(stderr, "there was a error oping the LWM");
    return EXIT_FAILURE;
  }
  screen = DefaultScreen(display);
  root = DefaultRootWindow(display);
  runLuaScript("~/.config/LWM/config.lua");
  lua_getglobal(L,"default");
  lua_getfield(L, -1, "app_launcher");
  const char *launcher_script = lua_tostring(L, -1);
  launch_App(launcher_script);
  XSelectInput(display, root, SubstructureNotifyMask);
  XEvent ev;
  bool running = True;
  while (running){
    XNextEvent(display, &ev);
    if (ev.type == KeyPress) {
      handle_keypress(&ev.xkey); 
    }
    
  }
  XCloseDisplay(display);
  return EXIT_SUCCESS;
}


void runLuaScript(const char *script) {
    L = luaL_newstate();
    luaL_openlibs(L);

    // Expand the '~' in the script path
    char fullpath[1024];
    if (script[0] == '~') {
        const char *home = getenv("HOME");
        snprintf(fullpath, sizeof(fullpath), "%s%s", home, script + 1);
    } else {
        strncpy(fullpath, script, sizeof(fullpath));
    }

    // Run the Lua script
    if (luaL_dofile(L, fullpath)) {
        fprintf(stderr, "Error running Lua config: %s\n", lua_tostring(L, -1));
    }
}

void handle_keypress(XKeyEvent *ev){
  KeySym keysym = XLookupKeysym(ev, 0);
  char buffer[32];
  int len = XLookupString(ev, buffer, sizeof(buffer),&keysym,NULL);
  buffer[len] = '\0';
  lua_getglobal(L, "keybindings");
  lua_pushstring(L, buffer);
  lua_gettable(L, -2);
  if(!lua_isnil(L, -1)){
    const char *action = lua_tostring(L, -1);
    if (strcmp(action, "spawn_terminal")==0) {
      spawn_terminal();
    }else if (strcmp(action, "close_window")==0) {
      close_window(ev->window);
    }else if (strcmp(action, "toggle_floatign")==0) {
      toggle_floatign(ev->window);
    }
  }
  lua_pop(L, 2);
}


void spawn_terminal(){
  lua_getglobal(L, "default");
  lua_getfield(L, -1, "terminal");
  const char *terminal = lua_tostring(L, -1);
  launch_App(terminal);
}

void close_window(Window window){
  XKillClient(display, window);
}

void move_window(Window window,IntVec2_T pos){
  XMoveWindow(display, window, pos.x, pos.y);
}

void resize_window(Window window,UIntVec2_T size){
  XResizeWindow(display, window, size.x, size.y);
}
void toggle_floatign(Window window){

}
void launch_App(const char *script){
  system(script);
}


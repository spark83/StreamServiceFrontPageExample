#ifndef APPWINDOW_H
#define APPWINDOW_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _SDLAppWindow {
	u16 width;
	u16 height;
	u8 fullscreen;
	u8 app_running;
	SDL_Window* window;
	SDL_GLContext context;
	void* user_data;

	s8 (*on_update_callback)(struct _SDLAppWindow*, void*);
	void (*RunAppWindow) (struct _SDLAppWindow*);
	void (*SwapWindow) (struct _SDLAppWindow*);
} SDLAppWindow;

typedef s8 (*OnUpdate)(SDLAppWindow* window, void* user);

s8 InitWindow(SDLAppWindow* window, u16 width, u16 height, u8 fullscreen,
	const s8* name, void* user, OnUpdate callback);
void ReleaseWindow(SDLAppWindow* window);

#ifdef __cplusplus
}
#endif

#endif
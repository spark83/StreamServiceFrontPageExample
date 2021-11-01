/*
 *! \brief Creates an application window, and provides update callback with
 *		   system key input events.
 *! \author Sang Park
 *! \date Oct 2021
 */
#include <gl\glew.h>
#include <gl\gl.h>
#include <gl\glu.h>
#include <SDL.h>
#include "Logging/logger.h"
#include "Types.h"
#include "AppWindow.h"

#ifdef __cplusplus
extern "C" {
#endif

static void RunAppWindow(SDLAppWindow* window) {
	window->app_running = 1;

	while (window->app_running) {
		window->app_running =
			window->on_update_callback(window, window->user_data);
	}
}

static void SwapWindow(SDLAppWindow* window) {
	SDL_GL_SwapWindow(window->window);
}

s8 InitWindow(SDLAppWindow* window, u16 width, u16 height, u8 fullscreen,
	const s8* name, void* user, OnUpdate callback) {
	s8 ret = 1;
	SDL_WindowFlags screen_type_flags = SDL_WINDOW_SHOWN;
	window->width = width;
	window->height = height;
	window->fullscreen = fullscreen;
	window->app_running = 0;
	window->on_update_callback = callback;
	window->user_data = user;
	window->RunAppWindow = RunAppWindow;
	window->SwapWindow = SwapWindow;

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		LOG_ERROR("Fail to initalize SDL! SDL Error: %s\n", SDL_GetError());
		ret = 0;
	}

	// TODO: Update this to support lower versions of OpenGL ES since only OpenGLES
	// are supported on setup boxes. 
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

	if (fullscreen) {
		screen_type_flags = SDL_WINDOW_FULLSCREEN;
	}

	window->window =
		SDL_CreateWindow(name, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			width, height, SDL_WINDOW_OPENGL | screen_type_flags);

	if (!window->window) {
		LOG_ERROR("Window could not be created! SDL Error: %s\n", SDL_GetError());
		ret = 0;
	}
	else {
		window->context = SDL_GL_CreateContext(window->window);
		if (!window->context) {
			LOG_ERROR("OpenGL context could not be created! SDL Error: %s\n", SDL_GetError());
			ret = 0;
		}
		else {
			glewExperimental = GL_TRUE;
			GLenum glewError = glewInit();
			if (glewError != GLEW_OK) {
				LOG_ERROR("Error initializing GLEW! %s\n", glewGetErrorString(glewError));
			}

			// Vsync setting.  Make sure they sync with the screen frame rate to make sure we don't
			// render more than necessary.
			if (SDL_GL_SetSwapInterval(1) < 0) {
				LOG_ERROR("Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError());
			}
		}
	}

	// TODO: Create input structure to handle the input setting.

	return ret;
}

void ReleaseWindow(SDLAppWindow* window) {
	SDL_DestroyWindow(window->window);
	SDL_Quit();
	window->window = NULL;
}

#ifdef __cplusplus
}
#endif

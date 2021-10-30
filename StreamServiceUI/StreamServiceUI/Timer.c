#include <SDL.h>

#include "Types.h"
#include "Timer.h"

#ifdef __cplusplus
extern "C" {
#endif

void TickTimer(Timer* time) {
	time->last_time = time->curr_time;
	time->curr_time = SDL_GetPerformanceCounter();

	time->delta_time = (f64)((time->curr_time - time->last_time)) / (f64)(SDL_GetPerformanceFrequency());
}

#ifdef __cplusplus
}
#endif

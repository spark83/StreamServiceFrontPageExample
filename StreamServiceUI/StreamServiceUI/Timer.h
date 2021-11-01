/*
 *! \brief Timer class to calculate delta time since the last update.
 *! \author Sang Park
 *! \date Oct 2021
 */
#ifndef TIMER_H
#define TIMER_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	u64 curr_time;
	u64 last_time;
	f32 delta_time;  // Use this to simulate stuff.
} Timer;

void TickTimer(Timer* time);

#ifdef __cplusplus
}
#endif

#endif
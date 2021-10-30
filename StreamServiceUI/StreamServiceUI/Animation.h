#ifndef ANIMATION_H
#define ANIMATION_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	f32 from;
	f32 to;
	f32 speed;
	f32* value_pointer;
	s8 running;
} NumberAnimation;

void UpdateAnimation(NumberAnimation* animation, f32 dt);

#ifdef __cplusplus
}
#endif

#endif

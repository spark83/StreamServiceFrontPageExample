/*
 *! \brief Implementation of animation functions.
 *! \author Sang Park
 *! \date Oct 2021
 */
#include <math.h>
#include "Types.h"
#include "Animation.h"

#ifdef __cplusplus
extern "C" {
#endif

void UpdateAnimation(NumberAnimation* animation, f32 dt) {
	if (!animation->value_pointer)
		return;
	f32 tf_v = animation->to - animation->from;
	f32 vel = tf_v * animation->speed;
	vel *= dt;

	*animation->value_pointer += vel;

	if (animation->to < animation->from) {
		if (*animation->value_pointer < animation->to) {
			*animation->value_pointer = animation->to;
			animation->running = 0;
			animation->value_pointer = NULL;
		}
	} else {
		if (*animation->value_pointer > animation->to) {
			*animation->value_pointer = animation->to;
			animation->running = 0;
			animation->value_pointer = NULL;
		}
	}
}

#ifdef __cplusplus
}
#endif

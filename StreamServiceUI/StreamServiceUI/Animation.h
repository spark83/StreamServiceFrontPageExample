/*
 *! \brief Various animation structures that helps with animation of a given value.
 *! \author Sang Park 
 *! \date Oct 2021
 */
#ifndef ANIMATION_H
#define ANIMATION_H

/* TODO:
   - Add animation manager that would pull from list of currently unused animations and add them into
     currently used list and then update only those animations during update cycle.
   - Add animation structures that would also handle ease-in-out and also 1D or 2D spring animation.
   - If sprite animation becomes part of requirement, add in sprite animmation as well.
   - Repeating animation?
*/

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	f32 from;  // Number to animate from.
	f32 to;    // Destination number.
	f32 speed; // Animation speed.
	f32* value_pointer;  // Value that needs to be updated during update cycle.
	s8 running; // Set this to true to enable the animation.  Will set to false when animation is done.
} NumberAnimation;

void UpdateAnimation(NumberAnimation* animation, f32 dt);

#ifdef __cplusplus
}
#endif

#endif

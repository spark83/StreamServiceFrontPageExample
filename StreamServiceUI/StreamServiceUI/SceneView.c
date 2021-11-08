/*
 *! \brief Scene view model containing scene data that is translated and copied
 *         from scene model data.
 *! \author Sang Park
 *! \date Oct 2021
 */
#include <stdio.h>
#include <string.h>
#include <cglm/cglm.h>

#include "Types.h"
#include "Animation.h"
#include "Renderer.h"
#include "SceneView.h"

#ifdef __cplusplus
extern "C" {
#endif

void InitViewScene(ViewScene* scene,
	f32 scene_width, f32 scene_height,
	f32 item_offset, f32 collection_offset,
	f32 xpos, f32 ypos,
	f32 pos_offset_x, f32 pos_offset_y) {
	memset(scene, 0, sizeof(ViewScene));
	scene->width = scene_width;
	scene->height = scene_height;
	scene->item_offset = item_offset;
	scene->collection_offset = collection_offset;
	scene->pos[0] = xpos;
	scene->pos[1] = ypos;
	scene->pos_offset[0] = pos_offset_x;
	scene->pos_offset[1] = pos_offset_y;
}

void UpdateLocalPosition(ViewScene* scene, SceneNavigator* navigator,
						 NumberAnimation* side_ani, NumberAnimation* vert_ani) {
	f32 left_edge;
	f32 right_edge;
	f32 top_edge;
	f32 bottom_edge;
	f32 h_move_val;
	f32 v_move_val;
	f32 ypos = scene->height - scene->pos[1];
	f32 xpos = scene->pos[0];
	f32 item_offset = scene->item_offset;
	f32 scene_width = scene->width;
	f32 scene_height = scene->height;

	s16 collection_idx = navigator->collection_idx;
	ViewItemCollectionList* collection_list = &scene->main_scene;
	ViewItemCollection* collection = &collection_list->collections[collection_idx];
	s16 itm_idx = navigator->item_idx + collection->start_idx;
	ViewItem* item = &scene->item_list.item_list[itm_idx];
	f32 item_width = item->width * item->bias_scale;
	f32 item_height = item->height * item->bias_scale;
	f32 item_half_width = item_width / 2.0f;
	f32 item_half_height = item_height / 2.0f;

	h_move_val = item_width + item_offset;
	v_move_val = scene->collection_offset;

	xpos += h_move_val * navigator->item_idx;
	ypos -= v_move_val * navigator->collection_idx;

	left_edge = xpos - item_half_width;
	right_edge = xpos + item_half_width;

	top_edge = ypos + item_half_height;
	bottom_edge = ypos - item_half_height;

	// Set side scrolling animation when currently selected item is outside of
	// viewing window.
	if (left_edge < -collection->pos[0]) {
		f32 remain = left_edge;
		f32 shift = remain / h_move_val;
		collection->shift_idx = (int)shift;

		side_ani->from = collection->pos[0];
		side_ani->to = -(int)(remain / h_move_val) * h_move_val;
		side_ani->value_pointer = &collection->pos[0];
		side_ani->speed = 7.0f;
		side_ani->running = 1;
	} else if (right_edge > -collection->pos[0] + scene_width) {
		f32 remain = right_edge - scene_width;
		f32 shift = remain / h_move_val;
		collection->shift_idx = (int)shift + 1;

		side_ani->from = collection->pos[0];
		side_ani->to = -((int)(remain / h_move_val) + 1.0f) * h_move_val;
		side_ani->value_pointer = &collection->pos[0];
		side_ani->speed = 7.0f;
		side_ani->running = 1;
	}

	printf("shift idx: %d \n", collection->shift_idx);

	if (top_edge > scene->y_nav_pos + scene_height) {
		f32 remain = bottom_edge - scene_height;
		side_ani->from = scene->y_nav_pos;
		side_ani->to = (int)(remain / v_move_val + 1.0f) * v_move_val;
		side_ani->value_pointer = &scene->y_nav_pos;
		side_ani->speed = 7.0f;
		side_ani->running = 1;
	} else if (bottom_edge < scene->y_nav_pos) {
		f32 remain = bottom_edge;
		side_ani->from = scene->y_nav_pos;
		side_ani->to = (int)(remain / v_move_val - 1.0f) * v_move_val;
		side_ani->value_pointer = &scene->y_nav_pos;
		side_ani->speed = 7.0f;
		side_ani->running = 1;
	}
}

#ifdef __cplusplus
}
#endif

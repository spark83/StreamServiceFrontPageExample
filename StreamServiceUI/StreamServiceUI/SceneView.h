/*
 *! \brief Scene view model containing scene data that is translated and copied
 *         from scene model data.
 *! \author Sang Park
 *! \date Oct 2021
 */
#ifndef SCENEVIEW_H
#define SCENEVIEW_H

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_ITEM_MODELS_PER_SCREEN		256
#define MAX_ITEM_CATEGORY_PER_SCREEN	32

// Uses tile texture
typedef struct {
	s16 tile_id;
	s16 tile_index;
	s16 row, col;
	f32 width;
	f32 height;
	f32 scalar;
	f32 bias_scale;
	vec2 pos;
} ViewItem;

typedef struct {
	u16 num_items;
	ViewItem item_list[MAX_ITEM_MODELS_PER_SCREEN];
} ViewItemList;

typedef struct {
	vec2 pos;
	s8 name[64];
	s16 start_idx, end_idx;
	u16 num_items;
	s16 shift_idx;
} ViewItemCollection;

typedef struct {
	u16 num_collections;
	ViewItemCollection collections[MAX_ITEM_CATEGORY_PER_SCREEN];
} ViewItemCollectionList;

typedef struct {
	f32 width;
	f32 height;
	f32 item_offset;
	f32 collection_offset;
	f32 y_nav_pos;
	vec2 pos;
	vec2 pos_offset;
	ViewItemCollectionList main_scene;
	ViewItemList item_list;
} ViewScene;

// Uses single texture
// Currently used for highlighting current item selection
typedef struct {
	Texture texture;
	f32 width, height;
	f32 scalar;
	f32 opacity;
	vec2 pos;
} QuadItem;

// Used for navigating through the main view scene.
typedef struct {
	s16 item_idx;
	s16 collection_idx;
} SceneNavigator;

void InitViewScene(ViewScene* scene,
	f32 scene_width, f32 scene_height,
	f32 item_offset, f32 collection_offset,
	f32 xpos, f32 ypos,
	f32 pos_offset_x, f32 pos_offset_y);

void UpdateLocalPosition(ViewScene* scene, SceneNavigator* navigator,
	NumberAnimation* side_ani, NumberAnimation* vert_ani);

#ifdef __cplusplus
}
#endif

#endif

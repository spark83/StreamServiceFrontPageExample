#ifndef SCENEMODEL_H
#define SCENEMODEL_H

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_ITEM_MODELS		256
#define MAX_ITEM_CATEGORY	32

typedef struct {
	s8 image_loaded;
	s8 image_loaded_in_gpu;
	u16 category_index;
	u16 width, height;
	u32 image_buffer_size;
	u8* image_buffer;
} ItemModel;

typedef struct {
	u16 num_items;
	ItemModel items[MAX_ITEM_MODELS];
} ItemModelList;

typedef struct {
	char name[64];
	s16 begin_index;
	s16 end_index;
	s16 num_items;
} CategoryModel;

typedef struct {
	u16 num_category;
	CategoryModel catagories[MAX_ITEM_CATEGORY];
} CategoryList;

typedef struct {
	ItemModelList item_list;
	CategoryList category_list;
	s8 populated;
} Collection;

void InitCollection(Collection* collection);

void AddCategolry(Collection* collection, char* name);

void AddItem(Collection* collection, int category_id, u8* image, u32 image_size);

void ReleaseCollection(Collection* collection);

#ifdef __cplusplus
}
#endif

#endif

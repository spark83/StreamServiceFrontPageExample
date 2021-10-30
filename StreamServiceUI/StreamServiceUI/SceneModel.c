#include <stb_image.h>
#include <stb_image_write.h>

#include "logger.h"
#include "Types.h"
#include "SceneModel.h"

#ifdef __cplusplus
extern "C" {
#endif

void InitCollection(Collection* collection) {
	memset(&collection->item_list, 0, sizeof(ItemModelList));
	memset(&collection->category_list, 0, sizeof(CategoryList));
	collection->populated = 0;
}

void AddCategolry(Collection* collection, char* name) {
	u16 index = collection->category_list.num_category;
	CategoryModel* model = &collection->category_list.catagories[index];
	strcpy(model->name, name);
	collection->category_list.num_category++;
}

void AddItem(Collection* collection, int category_id, u8* image, u32 image_size) {
	int w, h, c;
	u8* image_buffer = stbi_load_from_memory(image, image_size, &w, &h, &c, STBI_rgb);
	u32 image_buffer_size = w * h * c;

	ItemModelList* item_list = &collection->item_list;
	ItemModel* item_model = &item_list->items[item_list->num_items];

	item_model->image_buffer = image_buffer;
	item_model->image_buffer_size = image_buffer_size;
	item_model->category_index = category_id;
	item_model->image_loaded = 1;
	item_model->image_loaded_in_gpu = 0;
	item_model->width = w;
	item_model->height = h;
	item_list->num_items++;

	CategoryModel* model = &collection->category_list.catagories[category_id];
	if (category_id == 0) {
		model->end_index = ++model->num_items;
	}
	else {
		CategoryModel* prev_model = &collection->category_list.catagories[category_id - 1];
		model->begin_index = prev_model->end_index;
		model->end_index = model->begin_index + (++model->num_items);
	}
}

void ReleaseCollection(Collection* collection) {
	u16 i;

	const u16 num_items = collection->item_list.num_items;
	for (i = 0; i < num_items; ++i) {
		ItemModel* item = &collection->item_list.items[i];
		stbi_image_free(item->image_buffer);
		item->image_buffer = NULL;
	}

	memset(&collection->item_list, 0, sizeof(ItemModelList));
	memset(&collection->category_list, 0, sizeof(CategoryList));
}

#ifdef __cplusplus
}
#endif

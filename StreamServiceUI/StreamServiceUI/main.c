#define _CRT_SECURE_NO_DEPRECATE
#define CURL_STATICLIB
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION
//#define STB_DXT_IMPLEMENTATION

#include <gl\glew.h>
#include <gl\gl.h>
#include <gl\glu.h>
#include <SDL.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <curl/curl.h>
#include <cjson/cJSON.h>
#include <stb_image.h>
#include <stb_image_write.h>
#include <stb_dxt.h>
#include <stb_truetype.h>
#include <tinycthread.h>
#include <cglm/cglm.h>

#include "logger.h"
#include "Types.h"

#include "Animation.h"
#include "DefaultDatas.h"
#include "DataTypes.h"
#include "AppWindow.h"
#include "Timer.h"
#include "Renderer.h"
#include "SceneModel.h"
#include "JsonHelper.h"
#include "CurlHelper.h"
#include "SceneView.h"

#define MAX_JSON_STR_BUFFER_SIZE	2097152
#define MAX_JSON_TOKEN				2000
#define MAX_IMAGE_BUFFER_SIZE		131072
#define MAX_STRING_BUFFER_SIZE		128
#define MAX_DEFAULT_WIN_WIDTH		1920
#define MAX_DEFAULT_WIN_HEIGHT		1080

const char k_bamgridUrl[] = "https://cd-static.bamgrid.com/dp-117731241344/home.json";
const char k_bamgridRefSet[] = "https://cd-static.bamgrid.com/dp-117731241344/sets/";

mtx_t g_collection_model_mutex;

// TODO: Change these values to make sure different size textures can be added to this too.
const TextureSize k_texture_sizes[MAX_TILE_TEXTURES] = {
	[0] = { .width = 500, .height = 281 }
};

typedef struct {
	Timer timer;
	GLRenderer* renderer;
	GLShaderProgram* tile_effect;
	GLShaderProgram* selection_effect;
	QuadItem menu_selection;

	// TODO: Add different type of collection or sets
	Collection* main_page_collection;
	ViewScene* main_view_scene;

	// Animation for scrolling and scaling.
	NumberAnimation side_scroll_ani;
	NumberAnimation vert_scroll_ani;
	NumberAnimation item_scale_minimize_ani;
	NumberAnimation item_scale_maximize_ani;
	NumberAnimation select_scale_maximize_ani;

	SceneNavigator navigator;
} UpdateHelperContainer;

void JSON_ParseAndAddItem(CURL* curl_req, cJSON* items,
	DataBufferChunk* data_buffer,
	Collection* main_page,
	u16 category_id,
	OnDataAcquiredFunc callback) {
	cJSON* item;

	cJSON_ArrayForEach(item, items) {
		cJSON* image = cJSON_GetObjectItemCaseSensitive(item, "image");
		cJSON* tile = cJSON_GetObjectItemCaseSensitive(image, "tile");
		cJSON* img_1_78 = cJSON_GetObjectItemCaseSensitive(tile, "1.78");
		cJSON* default_set = img_1_78->child->child;
		cJSON* url = cJSON_GetObjectItemCaseSensitive(default_set, "url");
		cJSON* masterId = cJSON_GetObjectItemCaseSensitive(default_set, "masterId");

		if (cJSON_IsString(url) && (url->valuestring != NULL) &&
			cJSON_IsString(masterId) && (masterId->valuestring != NULL)) {
			char img_filename[256];
			int offset = 0;

			offset = sprintf(img_filename + offset, masterId->valuestring);
			offset = sprintf(img_filename + offset, ".jpg");

			ResetDataBuffer(data_buffer);

			if (!CURL_ParseIntoDataBuffer(curl_req, url->valuestring, data_buffer, callback)) {
				LOG_ERROR("Parse into buffer failed");
			} else {
				// TODO: Save as a file instead and render thread to load that file instead?
				mtx_lock(&g_collection_model_mutex);
				AddItem(main_page, category_id, data_buffer->buffer, data_buffer->buffer_size);
				mtx_unlock(&g_collection_model_mutex);
			}
		}
	}
}

size_t OnJSONStrBufferAcquired(char* buffer, size_t itemsize, size_t nitms, void* json_buffer) {
	size_t bytesize = itemsize * nitms;
	DataBufferChunk* data_buffer = (DataBufferChunk*)json_buffer;
	s8* str_buffer = data_buffer->buffer;
	sprintf(str_buffer + data_buffer->used_size, buffer);
	data_buffer->used_size += (u32)bytesize;
	return bytesize;
}

size_t OnImageDataAcquired(char* buffer, size_t itemsize, size_t nitms, void* data_buffer) {
	size_t bytesize = itemsize * nitms;
	DataBufferChunk* data = (DataBufferChunk*)data_buffer;
	memcpy(data->buffer + data->used_size, buffer, bytesize);
	data->used_size += (u32)bytesize;
	return itemsize * nitms;
}

s32 ScaneLoadThread(void* data) {
	DataBufferChunk json_data_buffer;
	DataBufferChunk image_data_buffer;
	CURL* curl_req;

	Collection* main_page = (Collection*)data;

	// TODO: Allocate this using stack allocator
	InitlDataBuffer(&json_data_buffer, MAX_JSON_STR_BUFFER_SIZE);
	InitlDataBuffer(&image_data_buffer, MAX_IMAGE_BUFFER_SIZE);
	curl_req = CURL_Init();

	if (curl_req) {
		cJSON* media_json;
		cJSON* containers;
		cJSON* container;
		s32 category_id = 0;

		if (!CURL_ParseIntoDataBuffer(curl_req, k_bamgridUrl, &json_data_buffer, OnJSONStrBufferAcquired)) {
			LOG_ERROR("Parse into buffer failed");
			return 0;
		}

		if (!(media_json = JSON_ParseJsonString(json_data_buffer.buffer))) {
			return 0;
		}

		containers = JSON_GetToContainers(media_json);

		// Loop through and get the categories first
		cJSON_ArrayForEach(container, containers) {
			cJSON* cont_set = cJSON_GetObjectItemCaseSensitive(container, "set");
			cJSON* content = JSON_GetCategoryContentText(cont_set);

			if (cJSON_IsString(content) && (content->valuestring != NULL)) {
				mtx_lock(&g_collection_model_mutex);
				AddCategolry(main_page, content->valuestring);
				mtx_unlock(&g_collection_model_mutex);
			}
		}

		// Load images for each categories
		cJSON_ArrayForEach(container, containers) {
			cJSON* cont_set = cJSON_GetObjectItemCaseSensitive(container, "set");
			cJSON* items = JSON_GetItemsNode(cont_set);
			cJSON* content = JSON_GetCategoryContentText(cont_set);

			if (items) {
				JSON_ParseAndAddItem(curl_req, items, &image_data_buffer,
					main_page, category_id, OnImageDataAcquired);
			}
			else {
				char curatedSets[MAX_STRING_BUFFER_SIZE];
				cJSON* data;
				size_t ref_str_len = strlen(k_bamgridRefSet);
				strcpy(curatedSets, k_bamgridRefSet);

				cJSON* refid = cJSON_GetObjectItemCaseSensitive(cont_set, "refId");

				if (cJSON_IsString(refid) && (refid->valuestring != NULL)) {
					ResetDataBuffer(&json_data_buffer);
					size_t ref_id_len = strlen(refid->valuestring);
					sprintf(curatedSets + ref_str_len, refid->valuestring);
					sprintf(curatedSets + ref_str_len + ref_id_len, ".json");

					if (!CURL_ParseIntoDataBuffer(curl_req, curatedSets, &json_data_buffer, OnJSONStrBufferAcquired)) {
						LOG_ERROR("Parse into buffer failed");
						return 0;
					}

					if (!(media_json = JSON_ParseJsonString(json_data_buffer.buffer))) {
						return 0;
					}

					data = cJSON_GetObjectItemCaseSensitive(media_json, "data");
					cJSON* item_set = data->child;
					JSON_ParseAndAddItem(curl_req, JSON_GetItemsNode(item_set),
						&image_data_buffer, main_page, category_id, OnImageDataAcquired);
				}
			}

			category_id++;
		}
	}

	mtx_lock(&g_collection_model_mutex);
	main_page->populated = 1;
	mtx_unlock(&g_collection_model_mutex);

	CURL_Release(curl_req);
	ReleaseDataBuffer(&image_data_buffer);
	ReleaseDataBuffer(&json_data_buffer);
	return 1;
}

void UpdateNavigationViewScene(GLRenderer* gl_renderer, ViewScene* main_view_scene, Collection* main_page) {
	// Do scene data update here.
	// Copy from scene data in scene load thread and trasform them
	// into scene 
	// ----------------------------------------
	mtx_lock(&g_collection_model_mutex);

	// TODO: Instead of iterating through the whole scene replace this with
	// some sort of scene event queue and pull from the event and build 
	// matching 2d scene here.
	if (!main_page->populated) {
		for (u16 i = 0; i < main_page->item_list.num_items; ++i) {
			ItemModel* item = &main_page->item_list.items[i];
			ViewItem* view_item = &main_view_scene->item_list.item_list[i];
			if (item->image_loaded && !item->image_loaded_in_gpu) {
				s32 index = gl_renderer->AppendToTileTexture(gl_renderer, 0, item->image_buffer);
				const TileTexture* tile_texture_info = gl_renderer->GetTileTextureInfo(gl_renderer, 0);

				if (index != -1) {
					u32 tile_width = tile_texture_info->max_width;
					u32 tile_height = tile_texture_info->max_height;

					int num_cols = tile_width / item->width;
					int num_rows = tile_height / item->height;

					int row_ind = index / num_cols;
					int col_ind = index % num_cols;

					item->image_loaded_in_gpu = TRUE;
					view_item->tile_id = 0;
					view_item->tile_index = (s16)index;
					view_item->width = item->width;
					view_item->height = item->height;
					view_item->row = row_ind;
					view_item->col = col_ind;
					view_item->bias_scale = 0.5f;
					view_item->scalar = i == 0 ? 1.2f : 1.0f;
				}

				stbi_image_free(item->image_buffer);
				item->image_buffer = NULL;
				item->image_loaded = FALSE;
			}
		}
		main_view_scene->item_list.num_items = main_page->item_list.num_items;

		for (u16 i = 0; i < main_page->category_list.num_category; ++i) {
			CategoryModel* cat_model = &main_page->category_list.catagories[i];
			ViewItemCollection* collection = &main_view_scene->main_scene.collections[i];
			collection->start_idx = cat_model->begin_index;
			collection->end_idx = cat_model->end_index;
			collection->num_items = collection->end_idx - collection->start_idx;
			memcpy(collection->name, cat_model->name, sizeof(collection->name));
		}

		main_view_scene->main_scene.num_collections = main_page->category_list.num_category;
	}
	mtx_unlock(&g_collection_model_mutex);
}

void HandleInputEvents(UpdateHelperContainer* container, SDL_Keycode input_key) {
	ViewItemCollection* collection;
	u16 num_collections;
	u16 num_items;
	s16 itm_idx;
	ViewItem* prev_item;
	ViewItem* new_item;
	s8 index_changed = 1;
	ViewScene* main_view_scene = container->main_view_scene;
	SceneNavigator* navigator = &container->navigator;

	collection = &main_view_scene->main_scene.collections[navigator->collection_idx];
	itm_idx = collection->start_idx + navigator->item_idx;
	
	prev_item = &main_view_scene->item_list.item_list[itm_idx];

	switch (input_key) {
	case SDLK_LEFT:
		navigator->item_idx--;
		break;
	case SDLK_RIGHT:
		navigator->item_idx++;
		break;
	case SDLK_UP:
		navigator->collection_idx--;
		break;
	case SDLK_DOWN:
		navigator->collection_idx++;
		break;
	}

	num_collections = main_view_scene->main_scene.num_collections;

	if (navigator->collection_idx < 0) {
		navigator->collection_idx = 0;
		index_changed = 0;
	} else if (navigator->collection_idx >= num_collections) {
		navigator->collection_idx = num_collections - 1;
		index_changed = 0;
	}

	collection = &main_view_scene->main_scene.collections[navigator->collection_idx];
	num_items = collection->num_items;

	if (navigator->item_idx < 0) {
		navigator->item_idx = 0;
		index_changed = 0;
	} else if (navigator->item_idx >= num_items) {
		navigator->item_idx = num_items - 1;
		index_changed = 0;
	}

	if (index_changed) {
		container->item_scale_minimize_ani.from = prev_item->scalar;
		container->item_scale_minimize_ani.to = 1.0f;
		container->item_scale_minimize_ani.value_pointer = &prev_item->scalar;
		container->item_scale_minimize_ani.speed = 7.5f;
		container->item_scale_minimize_ani.running = 1;

		container->menu_selection.scalar = 0.5f;
		container->select_scale_maximize_ani.from = container->menu_selection.scalar;
		container->select_scale_maximize_ani.to = 0.6f;
		container->select_scale_maximize_ani.value_pointer = &container->menu_selection.scalar;
		container->select_scale_maximize_ani.speed = 7.5f;
		container->select_scale_maximize_ani.running = 1;
	}

	itm_idx = collection->start_idx + navigator->item_idx;
	new_item = &main_view_scene->item_list.item_list[itm_idx];

	container->item_scale_maximize_ani.from = new_item->scalar;
	container->item_scale_maximize_ani.to = 1.2f;
	container->item_scale_maximize_ani.value_pointer = &new_item->scalar;
	container->item_scale_maximize_ani.speed = 7.5f;
	container->item_scale_maximize_ani.running = 1;

	UpdateLocalPosition(main_view_scene, navigator, &container->side_scroll_ani, &container->vert_scroll_ani);
}

s8 OnScreenUpdate(SDLAppWindow* window, void* user) {
	SDL_Event e;
	s8 ret = 1;
	UpdateHelperContainer* container = (UpdateHelperContainer*)user;
	GLRenderer* gl_renderer = container->renderer;
	GLShaderProgram* tile_effect = container->tile_effect;
	ViewScene* main_view_scene = container->main_view_scene;
	SceneNavigator* navigator = &container->navigator;
	
	UpdateNavigationViewScene(gl_renderer, main_view_scene, container->main_page_collection);

	// Update input events.
	// ----------------------------------------
	while (SDL_PollEvent(&e) != 0) {
		switch (e.type) {
			case SDL_QUIT:
				ret = 0;
				break;
			case SDL_KEYDOWN:
				HandleInputEvents(container, e.key.keysym.sym);
				break;
			default:
				break;
		}
	}
	
	// Update animation.
	// Since there is no need for complex animation, doing this in the
	// rendering thread is ok for now. 
	// ----------------------------------------
	UpdateAnimation(&container->side_scroll_ani, container->timer.delta_time);
	UpdateAnimation(&container->vert_scroll_ani, container->timer.delta_time);
	UpdateAnimation(&container->item_scale_minimize_ani, container->timer.delta_time);
	UpdateAnimation(&container->item_scale_maximize_ani, container->timer.delta_time);
	UpdateAnimation(&container->select_scale_maximize_ani, container->timer.delta_time);
	
	// Do simple forward rendering here.
	// ----------------------------------------
	gl_renderer->BeginRender(gl_renderer);
	
	// Render quads.
	// ----------------------------------------
	gl_renderer->BeginRenderTileQuad(gl_renderer, tile_effect, 0); // Currently we only need tile 0 to render all our texture needs

	mat4 ortho;
	glm_ortho(0, (float)window->width, 0, (float)window->height, -1.0, 100, ortho);

	const u16 num_collections = main_view_scene->main_scene.num_collections;
	const f32 item_offset = main_view_scene->item_offset;
	const f32 collection_offset = main_view_scene->collection_offset;
	const TileTexture* tile_texture = gl_renderer->GetTileTextureInfo(gl_renderer, 0);

	f32 ypos = window->height - main_view_scene->pos[1] - main_view_scene->y_nav_pos;
	for (u16 i = 0; i < num_collections; ++i) {
		ViewItemCollection* collection = &main_view_scene->main_scene.collections[i];
		ViewItemList* item_list = &main_view_scene->item_list;
		f32 xpos = main_view_scene->pos[0] + collection->pos[0];

		const u16 item_start = collection->start_idx;
		const u16 item_end = collection->end_idx;

		for (u16 j = item_start; j < item_end; ++j) {
			ViewItem* view_item = &item_list->item_list[j];
			view_item->pos[0] = xpos;
			view_item->pos[1] = ypos;

			gl_renderer->RenderTiledQuad(gl_renderer, tile_effect, ortho,
				view_item->width / tile_texture->max_width, view_item->height / tile_texture->max_width,
				view_item->col, view_item->row,
				view_item->width * view_item->bias_scale, view_item->height * view_item->bias_scale,
				xpos, ypos, view_item->scalar, view_item->scalar);

			xpos += view_item->width * view_item->bias_scale + item_offset;
		}
		
		ypos -= collection_offset;
	}

	// Render selection.
	if (main_view_scene->item_list.num_items > 0) {
		ViewItemCollection* collection = &main_view_scene->main_scene.collections[navigator->collection_idx];
		s16 itm_idx = collection->start_idx + navigator->item_idx;
		ViewItem* new_item = &main_view_scene->item_list.item_list[itm_idx];
		QuadItem* selection_item = &container->menu_selection;
		gl_renderer->ApplyShader(*container->selection_effect);
		gl_renderer->ApplyTexture(&selection_item->texture);
		gl_renderer->RenderQuad(gl_renderer, container->selection_effect, ortho, selection_item->width, selection_item->height,
			new_item->pos[0], new_item->pos[1], selection_item->scalar, selection_item->scalar, selection_item->opacity);
	}

	// Render strings.
	// ----------------------------------------
	gl_renderer->BeginRenderString(gl_renderer);

	ypos = window->height - main_view_scene->pos[1] - main_view_scene->y_nav_pos;
	for (u16 i = 0; i < num_collections; ++i) {
		ViewItemCollection* collection = &main_view_scene->main_scene.collections[i];
		f32 xpos = main_view_scene->pos[0] + main_view_scene->pos_offset[0];
		gl_renderer->RenderString(gl_renderer, ortho, collection->name, xpos,
								  ypos + main_view_scene->pos_offset[1], 22.0f);
		ypos -= collection_offset;
	}

	gl_renderer->EndRender(gl_renderer);
	window->SwapWindow(window);

	TickTimer(&container->timer);

	return ret;
}

int main(int argc, char* argv[]) {
	thrd_t webservice_thread;

	// Data used in loaing thread
	Collection* main_page_collection;
	// Data used in graphic rendering scene
	ViewScene* main_view_scene;
	SDLAppWindow* window;
	GLRenderer* renderer;

	// Setup logger
	logger_initConsoleLogger(NULL);
	logger_initFileLogger("LOG.log", 1024 * 1024, 5);
	logger_setLevel(LogLevel_ERROR);

	main_page_collection = malloc(sizeof(Collection));
	main_view_scene = malloc(sizeof(ViewScene));
	window = malloc(sizeof(SDLAppWindow));
	renderer = malloc(sizeof(GLRenderer));

	UpdateHelperContainer container;
	GLShaderSource tile_shader_source;
	GLShaderSource select_shader_source;
	GLShaderProgram tile_shader_effect;
	GLShaderProgram selection_shader_effect;

	mtx_init(&g_collection_model_mutex, mtx_plain);

	tile_shader_source.vertex_src = INDEXED_VERTEX_SHADER_SRC;
	tile_shader_source.fragment_src = INDEXED_FRAGMENT_SHADER_SRC;
	select_shader_source.vertex_src = QUAD_VERTEX_SHADER_SRC;
	select_shader_source.fragment_src = QUAD_FRAGMENT_SHADER_SRC;
	vec4 clear_color = { 0.12f, 0.12f, 0.12f, 0.0f };

	// Initalize main page collection model.
	InitCollection(main_page_collection);
	InitViewScene(main_view_scene, MAX_DEFAULT_WIN_WIDTH, MAX_DEFAULT_WIN_HEIGHT, 40, 250, 220, 200, -140, 110);

	// Create window.
	InitWindow(window, MAX_DEFAULT_WIN_WIDTH, MAX_DEFAULT_WIN_HEIGHT, FALSE, "Disney+", &container, OnScreenUpdate);

	container.renderer = renderer;
	container.tile_effect = &tile_shader_effect;
	container.selection_effect = &selection_shader_effect;
	container.main_page_collection = main_page_collection;
	container.main_view_scene = main_view_scene;
	container.navigator.item_idx = 0;
	container.navigator.collection_idx = 0;
	container.side_scroll_ani.value_pointer = NULL;
	container.vert_scroll_ani.value_pointer = NULL;
	container.item_scale_minimize_ani.value_pointer = NULL;
	container.item_scale_maximize_ani.value_pointer = NULL;
	container.select_scale_maximize_ani.value_pointer = NULL;
	container.side_scroll_ani.running = 0;
	container.vert_scroll_ani.running = 0;
	container.item_scale_minimize_ani.running = 0;
	container.item_scale_maximize_ani.running = 0;
	container.select_scale_maximize_ani.running = NULL;

	TickTimer(&container.timer);

	// TODO: Create these into some type of thread pool and reuse them when necessary
	// Use cases for thread pool will be:
	// - When entire set needs to be refreshed.
	// - Other type of resource loading needs to be done.
	thrd_create(&webservice_thread, ScaneLoadThread, main_page_collection);

	// Initalize renderer.
	InitGLRenderer(renderer, k_texture_sizes);
	renderer->SetClearColor(renderer, clear_color);
	tile_shader_effect = renderer->CompileShader(renderer, &tile_shader_source);
	selection_shader_effect = renderer->CompileShader(renderer, &select_shader_source);

	// Load selection image.
	int width, height, channel;
	stbi_uc* image_buffer = stbi_load("Selection.png", &width, &height, &channel, 4);
	renderer->CreateTexture(&container.menu_selection.texture, image_buffer, width, height);
	container.menu_selection.width = (f32)width;
	container.menu_selection.height = (f32)height;
	container.menu_selection.opacity = 0.94f;
	container.menu_selection.scalar = 0.6f;
	stbi_image_free(image_buffer);

	thrd_join(&webservice_thread, NULL);

	// Run the window.
	window->RunAppWindow(window);

	// Release any main thread stuff below here. 
	ReleaseWindow(window);
	ReleaseGLRenderer(renderer);
	ReleaseCollection(main_page_collection);

	mtx_destroy(&g_collection_model_mutex);

	free(main_page_collection);
	free(main_view_scene);
	free(window);
	free(renderer);

	return 0;
}

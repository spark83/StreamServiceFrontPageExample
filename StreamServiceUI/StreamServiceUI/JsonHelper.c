#include <cjson/cJSON.h>
#include "JsonHelper.h"

#ifdef __cplusplus
extern "C" {
#endif

cJSON* JSON_ParseJsonString(char* buffer) {
	return cJSON_Parse(buffer);
}

cJSON* JSON_GetToContainers(cJSON* media_json) {
	cJSON* data = media_json->child;
	cJSON* collection = data->child;
	return cJSON_GetObjectItemCaseSensitive(collection, "containers");
}

cJSON* JSON_GetItemsNode(cJSON* cont_set) {
	return cJSON_GetObjectItemCaseSensitive(cont_set, "items");
}

cJSON* JSON_GetCategoryContentText(cJSON* cont_set) {
	cJSON* text = cJSON_GetObjectItemCaseSensitive(cont_set, "text");
	cJSON* title = text->child;
	cJSON* full = title->child;
	cJSON* set = full->child;
	cJSON* default_set = set->child;

	return cJSON_GetObjectItemCaseSensitive(default_set, "content");
}

#ifdef __cplusplus
}
#endif

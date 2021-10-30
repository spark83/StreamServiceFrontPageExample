#ifndef JSONHELPER_H
#define JSONHELPER_H

#ifdef __cplusplus
extern "C" {
#endif

typedef size_t(*OnDataAcquiredFunc) (char*, size_t, size_t, void*);

cJSON* JSON_ParseJsonString(char* buffer);

cJSON* JSON_GetToContainers(cJSON* media_json);

cJSON* JSON_GetItemsNode(cJSON* cont_set);

cJSON* JSON_GetCategoryContentText(cJSON* cont_set);

#ifdef __cplusplus
}
#endif

#endif
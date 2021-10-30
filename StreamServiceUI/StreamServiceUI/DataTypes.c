#include <stdio.h>
#include <stdlib.h>

#include "Types.h"
#include "DataTypes.h"

#ifdef __cplusplus
extern "C" {
#endif

void InitlDataBuffer(DataBufferChunk* buffer, u32 size) {
	buffer->buffer_size = size;
	buffer->used_size = 0;
	buffer->buffer = (s8*)malloc(size);
}

void ResetDataBuffer(DataBufferChunk* buffer) {
	buffer->used_size = 0;
}

void ReleaseDataBuffer(DataBufferChunk* buffer) {
	free(buffer->buffer);
}

#ifdef __cplusplus
}
#endif

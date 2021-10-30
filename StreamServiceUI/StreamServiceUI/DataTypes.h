#ifndef DATATYPES_H
#define DATATYPES_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	u32 buffer_size;
	u32 used_size;
	s8* buffer;
} DataBufferChunk;

void InitlDataBuffer(DataBufferChunk* buffer, u32 size);

void ResetDataBuffer(DataBufferChunk* buffer);

void ReleaseDataBuffer(DataBufferChunk* buffer);

#ifdef __cplusplus
}
#endif

#endif

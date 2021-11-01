/*
 *! \brief Data buffer used for storing curl respond datas.
 *! \author Sang Park
 *! \date Oct 2021
 */
#ifndef DATATYPES_H
#define DATATYPES_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	u32 buffer_size;  // Total buffer size.
	u32 used_size;    // Size currentlly in use.
	s8* buffer;
} DataBufferChunk;

void InitlDataBuffer(DataBufferChunk* buffer, u32 size);

void ResetDataBuffer(DataBufferChunk* buffer);

void ReleaseDataBuffer(DataBufferChunk* buffer);

#ifdef __cplusplus
}
#endif

#endif

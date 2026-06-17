#include "data.h"

#include <stdlib.h>
#include <math.h>

#include <fileioc.h>

/*
### save data structure (v0):
```c
uint8_t data[] = "wds:1332:010002EE000F5508F8";
```
list of values prefixed by an index and null terminator
  data version	(uint8_t)
  highscore   	(uint24_t)
  score       	(uint24_t)
  hydration   	(uint16_t)

---

### 4-bit to ASCII:
```c
num += 48 // 0x30
if (num > 58) // num >= 0x0A + 0x30
  num += 7;
```
*/

const int WDS_DATA_VERSION = 1;

char wds_data_encodePart(uint8_t decodedChar);
uint8_t wds_data_decodePart(char encodedChar);
size_t wds_data_streamSize(const char *data, char endOfStream);

void wds_openData(char **dataPtr)
{
  // allocate memory for data string
  char *data = malloc(WDS_DATA_SIZE); /// data string

  // store allocated memory pointer to data pointer
  *dataPtr = data;

  // update data with new data sample
  for (uint8_t i = 0; i < WDS_DATA_SIZE; i++)
    data[i] = WDS_DATA_SAMPLE[i];

  // copy current data version to data sample
  wds_encode(&WDS_DATA_VERSION, 1, &data, 0);

  // attempt open AppVar
  uint8_t data_AppVar = ti_Open(WDS_DATA_APPVAR, "r+"); // data AppVar handle

  // execute if open AppVar successful
  if (data_AppVar)
  {
    // read AppVar and overwrite data string
    ti_Read(data, WDS_DATA_SIZE, 1, data_AppVar);
  }
  // execute if open AppVar failed
  else
  {
    // create new AppVar
    data_AppVar = ti_Open(WDS_DATA_APPVAR, "w");

    // write data to AppVar
    ti_Write(data, WDS_DATA_SIZE, 1, data_AppVar);
  }

  // execute if open AppVar successful
  if (data_AppVar)
  {
    // move AppVar to archive
    ti_SetArchiveStatus(true, data_AppVar);

    // close data AppVar
    ti_Close(data_AppVar);
  }
}

void wds_closeData(const char *data)
{
  // attempt open AppVar
  uint8_t data_AppVar = ti_Open(WDS_DATA_APPVAR, "w"); /// data AppVar handle

  // execute if open AppVar successful
  if (data_AppVar)
  {
    // write data to AppVar
    ti_Write(data, WDS_DATA_SIZE, 1, data_AppVar);

    // move AppVar to archive
    ti_SetArchiveStatus(true, data_AppVar);

    // close data AppVar
    ti_Close(data_AppVar);
  }

  // free allocated memory
  free(data);
}

uint64_t wds_decodeValue(const char *data, wds_index_t index)
{
  /// character length of wds data header's file prefix
  size_t data_header_prefix_size = wds_data_streamSize(data, ':');
  /// character length of wds data header
  size_t data_header_size = data_header_prefix_size + wds_data_streamSize(data + data_header_prefix_size, ':');
  /// size of value stored in encoded data, in half-bytes
  size_t data_value_size = wds_data_decodePart(data[data_header_prefix_size + index]) * 2;

  /// data value offset
  size_t data_value_position = data_header_size;
  // calculate data value offset
  for (wds_index_t i = 0; i < index; i++)
    data_value_position += wds_data_decodePart(data[data_header_prefix_size + i]) * 2;

  /// decoded data value to return
  uint64_t data_value = 0;
  // copy decoded value from data string to value
  for (size_t i = 0; i < data_value_size; i += 2)
  {
    size_t valueSegment_position = data_value_position + data_value_size - (i + 2);
    data_value += wds_data_decodePart(data[valueSegment_position + 1]) << (i * 4);
    data_value += wds_data_decodePart(data[valueSegment_position]) << ((i * 4) + 4);
  }

  // return decoded data value
  return data_value;
}

void wds_encode(const void *valuePtr, size_t size, char **dataPtr, wds_index_t index)
{
  /// data string
  char *data = *dataPtr;
  /// data string size
  size_t data_size = wds_data_streamSize(data, 0);
  /// character length of wds data header's file prefix
  size_t data_header_prefix_size = wds_data_streamSize(data, ':');
  /// character length of wds data header
  size_t data_header_size = data_header_prefix_size + wds_data_streamSize(data + data_header_prefix_size, ':');
  /// size of old value stored in encoded data, in half-bytes
  size_t data_oldValue_size = wds_data_decodePart(data[data_header_prefix_size + index]) * 2;
  /// half-byte size of new value
  size_t data_value_size = size * 2;

  /// data value position
  size_t data_value_position = data_header_size;
  // calculate position offset from data header
  for (wds_index_t i = 0; i < index; i++)
    data_value_position += wds_data_decodePart(data[data_header_prefix_size + i]) * 2;

  // match specified size
  if (data_oldValue_size > data_value_size)
  {
    /// new value size offset
    size_t size_offset = data_value_size - data_oldValue_size;

    // calculate new data size
    data_size += size_offset;

    // reallocate memory
    data = realloc(data, data_size);

    // shift data
    for (size_t i = 0; i < data_size - size_offset; i++)
      data[i + (size_offset * (i < data_value_position))] = *dataPtr[i];

    // update parameter data pointer
    free(*dataPtr);
    *dataPtr = data;

    // encode and store new value size
    data[data_value_position] = wds_data_encodePart(size);
  }

  // update data value
  for (size_t i = 0; i < data_value_size; i += 2)
  {
    uint8_t valueSegment = *(uint8_t *)(valuePtr + size - ((i / 2) + 1));
    data[data_value_position + i + 1] = wds_data_encodePart(valueSegment & 15);
    data[data_value_position + i] = wds_data_encodePart((valueSegment >> 4) & 15);
  }
}

// --- //

char wds_data_encodePart(uint8_t decodedChar)
{
  decodedChar &= 15;

  if (decodedChar > 9)
    decodedChar += 7;
  decodedChar += 48;

  return (uint8_t)decodedChar;
}

uint8_t wds_data_decodePart(char encodedChar)
{
  encodedChar -= 48;
  if (encodedChar > 9)
    encodedChar -= 7;

  encodedChar &= 15;

  return (char)encodedChar;
}

size_t wds_data_streamSize(const char *data, char endOfStream)
{
  size_t streamSize = 1;
  for (streamSize;
       data[streamSize - 1] != endOfStream;
       streamSize++)
    ;
  return streamSize;
}

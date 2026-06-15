#include "data.h"

#include <stdlib.h>
#include <math.h>

#include <fileioc.h>

/*
### save data structure (v0):
```c
uint8_t data[] = "wds:*,,+@_____+%%___&.._<&<";
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

char wds_data_encode(uint8_t decodedChar);
uint8_t wds_data_decode(char encodedChar);
size_t wds_data_streamSize(const char *data, char endOfStream);

void wds_openData(char **dataPtr)
{
  // allocate memory for data string
  char *data = malloc(WDS_DATA_SIZE);

  // store allocated memory pointer to data pointer
  *dataPtr = data;

  // update data with new data sample
  for (uint8_t i = 0; i < WDS_DATA_SIZE; i++)
    data[i] = WDS_DATA_SAMPLE[i];

  // init vars for updating version number
  /// character length of wds data header's file prefix
  size_t data_header_prefix_size = wds_data_streamSize(data, ':');
  /// character length of wds data header
  size_t data_header_size = data_header_prefix_size + wds_data_streamSize(data + data_header_prefix_size, ':');
  /// size of version value stored in encoded data in half-bytes
  size_t data_version_size = wds_data_decode(data[data_header_prefix_size]) * 2;

  // copy current data's version to data
  for (uint8_t i = 0; i < data_version_size; i++) /// data character incrementer
    data[i + data_header_size] = wds_data_encode(WDS_DATA_VERSION >> ((data_version_size - (i + 1)) * 4) & 15);

  // attempt open AppVar
  uint8_t data_AppVar = ti_Open(WDS_DATA_APPVAR, "r"); // data AppVar handle

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

  // close data AppVar
  ti_Close(data_AppVar);
}

bool wds_closeData(const char *data)
{
  // init boolean for success state
  bool successState = true; /// success state boolean

  // attempt open AppVar
  uint8_t data_AppVar = ti_Open(WDS_DATA_APPVAR, "w"); /// data AppVar handle

  // execute if open AppVar successful
  if (data_AppVar)
  {
    // write data to AppVar
    ti_Write(data, WDS_DATA_SIZE, 1, data_AppVar);

    // close data AppVar
    ti_Close(data_AppVar);
  }
  // execute if open AppVar failed
  else
  {
    successState = false;
  }

  // free allocated memory
  free(data);

  // return success state boolean
  return successState;
}

// --- //

char wds_data_encode(uint8_t decodedChar)
{
  if (decodedChar > 10)
    decodedChar += 7;
  decodedChar += 48;

  return (uint8_t)decodedChar;
}

uint8_t wds_data_decode(char encodedChar)
{
  encodedChar -= 48;
  if (encodedChar > 10)
    encodedChar -= 7;

  return (char)encodedChar;
}

size_t wds_data_streamSize(const char *data, char endOfStream)
{
  size_t streamSize = 1;
  for (streamSize;
       data[streamSize - 1] != endOfStream;
       streamSize++)
    NULL;
  return streamSize;
}

#ifndef WDS_DATA_H
#define WDS_DATA_H

#include <stdint.h>
#include <stdbool.h>

typedef uint8_t wds_index_t;                          /// wds data index value variable type
#define WDS_DATA_APPVAR "wdsData"                     /// data AppVar name
extern const int WDS_DATA_VERSION;                    /// current data version
#define WDS_DATA_SAMPLE "wds:1332:000000000000000000" /// data sample
#define WDS_DATA_DIVIDER ':'                          /// character in a data string dividing the header, addresses, and actual data from each other
#define WDS_DATA_SIZE 28                              /// size of data sample

void wds_openData(char **dataPtr);
void wds_closeData(const char *data);
uint64_t wds_decodeValue(const char *data, wds_index_t index);
void wds_encode(const void *valuePtr, size_t size, char **dataPtr, wds_index_t index);

#endif

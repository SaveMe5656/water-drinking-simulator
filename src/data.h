#ifndef WDS_DATA_H
#define WDS_DATA_H

#include <stdint.h>
#include <stdbool.h>

#define WDS_DATA_APPVAR "wdsData"                     /// data AppVar name
#define WDS_DATA_VERSION 1                            /// current data version
#define WDS_DATA_SAMPLE "wds:1332:000000000000000000" /// data sample
#define WDS_DATA_DIVIDER ':'                          /// character in a data string dividing the header, addresses, and actual data from each other
#define WDS_DATA_SIZE sizeof(WDS_DATA_SAMPLE)         /// size of data sample

void wds_openData(char **dataPtr);

bool wds_closeData(const char *data);

#endif

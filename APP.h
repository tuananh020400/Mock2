#ifndef _CHECK_H_
#define _CHECK_H_
#include "FATFS.h"
#include "HAL.h"
#include "APP.h"
void RunApp();

FATFS_EntryList_Struct_t * FATFS_ReadFileAndDirectory(FATFS_EntryList_Struct_t *head, uint8_t select);
#endif


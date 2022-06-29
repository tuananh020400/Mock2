#ifndef _FATFS_H_
#define _FATFS_H_

/*******************************************************************************
* Include
*******************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "HAL.h"

/*******************************************************************************
* Definition
*******************************************************************************/
#define STARTCLUSTER(X,Y) (((X) << 16) | (Y)) /* X is clusterHight, Y is clusterLow */

typedef struct{
    uint16_t bytesOfSector;
    uint8_t sectorsOfCluster;
    uint16_t sectorsOfBoot;
    uint8_t numOfFAT;
    uint16_t entrysOfRDET;
    uint16_t sectorsOfVolume;
    uint32_t sectorsOfFAT;
    int8_t typeOfFAT[8];
} FATFS_FatInfor_Struct_t;

typedef struct
{
    int8_t fileName[11];
    uint8_t attribute;
    uint8_t reserved[2];
    uint16_t timeStamp;
    uint16_t dateStamp;
    uint16_t accessDate;
    uint16_t clusterHight;
    uint16_t editDate;
    uint16_t editTime;
    uint16_t clusterLow;
    uint32_t fileSize;
} FATFS_Entry_Struct_t;

typedef struct EntryList
{
    FATFS_Entry_Struct_t entry;
    struct EntryList *next;
} FATFS_EntryList_Struct_t;

typedef enum
{
    READ_FAILED = 0,
    NOT_ENOUGH_MEMORY = 1,
    SUCCESSFULLY = 2,
} FATFS_Status_t;

/*******************************************************************************
* API
*******************************************************************************/

FATFS_Status_t FATFS_ReadBoot(void);

/**
 * @brief Read file and sub directory in a directory
 * 
 * @param[in] startCluster Start cluster of directory 
 * @param[inout] head Linked list save entries in directory
 * @return Status Reading Status
 */
FATFS_Status_t ReadDirectory(uint32_t startCluster, FATFS_EntryList_Struct_t **head);

/**
 * @brief Read the content of a file
 * 
 * @param[in] startCluster Start cluster of file 
 * @param[in] size Size of file
 * @param[out] buffer String to store the data of the file
 * @return Status Reading Status
 */
FATFS_Status_t ReadFile(uint32_t startCluster, uint32_t size, uint8_t *buffer);

#endif /* _FATFS_H_ */

/*******************************************************************************
* End of file
*******************************************************************************/

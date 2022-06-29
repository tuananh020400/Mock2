#ifndef _HAL_H_
#define _HAL_H_

/*******************************************************************************
* Include
*******************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "FATFS.h"

/*******************************************************************************
* Definition
*******************************************************************************/

typedef enum
{
    HAL_READ_FAILED = 0,
    HAL_NOT_ENOUGH_MEMORY = 1,
    HAL_SUCCESSFULLY = 2,
} HAL_Status_t;

/*******************************************************************************
* API
*******************************************************************************/

/**
 * @brief Initialize the reading process
 * 
 * @param[in] filePath The path to the file
 * @return Status Notification of successful or failed file reading
 */
HAL_Status_t HAL_Init(const char *filePath);

/**
 * @brief Update size of sector after read boot sector 
 * 
 * @param[in] size size of sector
 */
void HAL_UpdateSectorSize(uint16_t size);

/**
 * @brief Read data of a sector
 * 
 * @param[in] index Position of sector
 * @param[inout] buff Data storage array
 * @return uint32_t Number of bytes read
 */
int32_t HAL_ReadSector(uint32_t index, uint8_t *buff);

/**
 * @brief Read data of multiple sector
 * 
 * @param[in] index Position of first sector
 * @param[in] num Number of sectors need to read
 * @param[inout] buff Data storage array
 * @return uint32_t Number of bytes read
 */
int32_t HAL_ReadMultiSector(uint32_t index, uint32_t num, uint8_t *buff);

#endif /* _HAL_H */

/*******************************************************************************
* End of file
*******************************************************************************/


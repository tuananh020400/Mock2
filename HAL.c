/*******************************************************************************
* Include
*******************************************************************************/

#include "HAL.h"

/*******************************************************************************
* Prototypes
*******************************************************************************/

/*******************************************************************************
* Variables
*******************************************************************************/

static FILE *s_file;
static uint16_t sectorSize = 512;

/*******************************************************************************
* Code
*******************************************************************************/

HAL_Status_t HAL_Init(const char * filePath)
{
    HAL_Status_t readStatus = HAL_SUCCESSFULLY;

    /* Open stream to file */
    s_file = fopen(filePath, "rb");
    /* Cannot open stream */
    if(s_file == NULL)
    {
        readStatus = READ_FAILED;
    }
    /* Read boot sector */
    readStatus = FATFS_ReadBoot();
    
    return readStatus;
}

void HAL_UpdateSectorSize(uint16_t size)
{
    sectorSize = size; 
}

int32_t HAL_ReadSector(uint32_t index, uint8_t *buff)
{
    uint32_t byte = 0;
    /* Point to sector location */
    fseek(s_file, (uint64_t)(sectorSize * index), SEEK_SET);
    /* Read sector */
    byte = fread(buff, 1, sectorSize, s_file);

    return byte;
}

int32_t HAL_ReadMultiSector(uint32_t index, uint32_t num, uint8_t *buff)
{
    uint32_t byte = 0;
    
    /* Point to sector location */
    fseek(s_file, (uint64_t)(sectorSize * index), SEEK_SET);
    /* Read sector */
    byte = fread(buff, 1, sectorSize * num, s_file);

    return byte;
}

void HAL_DeInit()
{
    fclose(s_file);
}
/*******************************************************************************
* End of file
*******************************************************************************/


/*******************************************************************************
* Include
*******************************************************************************/

#include "FATFS.h"

/*******************************************************************************
* Prototypes
*******************************************************************************/

/**
 * @brief Add entry to linked list 
 * 
 * @param[inout] head Linked list save entries in directory
 * @param[in] buffer Buffer entry data 
 * @return Status Reading Status
 */
static FATFS_Status_t FATFS_AddEntry(FATFS_EntryList_Struct_t **head, FATFS_Entry_Struct_t *buffer);

/**
 * @brief Read Root Directory
 * 
 * @param[inout] head Linked list save entries in directory
 * @return Status Reading Status
 */
static FATFS_Status_t FATFS_ReadRootDirectory(FATFS_EntryList_Struct_t **head);

/**
 * @brief Read Sub Directory
 * 
 * @param[inout] head Linked list save entries in directory
 * @param startCluster Start cluster of the directory
 * @return Status Reading Status
 */
static FATFS_Status_t FATFS_ReadSubDirectory(FATFS_EntryList_Struct_t **head, uint32_t startCluster);

/**
 * @brief Read next FAT value of next startCluster
 * 
 * @param[inout] startCluster Start cluster of directories or files
 * @return Status Reading Status
 */
static FATFS_Status_t ReadFATValue(uint32_t *startCluster);

/*******************************************************************************
* Variables
*******************************************************************************/

static FATFS_FatInfor_Struct_t s_fatInfor;

static FATFS_Status_t ReadFATValue(uint32_t *startCluster);


/*******************************************************************************
* Code
*******************************************************************************/

FATFS_Status_t FATFS_ReadBoot(void)
{
    uint8_t index = 0;
    uint8_t *buffer = NULL;
    FATFS_Status_t readStatus = SUCCESSFULLY;

    /* Allocation buffer array */
    buffer = (uint8_t*)malloc(512);
    if(buffer == NULL)
    {
        readStatus = NOT_ENOUGH_MEMORY;
    }
    else
    {
        if(HAL_ReadSector(0, buffer) != 512)
        {
            readStatus = READ_FAILED;
        }
        else
        {
            /* Save data of boot sector */
            s_fatInfor.bytesOfSector = (((uint16_t)buffer[0x0B + 1]) << 8) | ((uint16_t)buffer[0x0B]);
            s_fatInfor.sectorsOfCluster = buffer[0x0D];
            s_fatInfor.sectorsOfBoot = ((uint16_t)buffer[0x0E + 1] << 8) | ((uint16_t)buffer[0x0E]);
            s_fatInfor.numOfFAT = buffer[0x10];
            s_fatInfor.entrysOfRDET = (((uint16_t)buffer[0x11 + 1]) << 8) | ((uint16_t)buffer[0x11]);
            s_fatInfor.sectorsOfVolume = ((uint16_t)buffer[0x13 + 1] << 8) | ((uint16_t)buffer[0x13]);
            s_fatInfor.sectorsOfFAT = (((uint16_t)buffer[0x16 + 1]) << 8) | ((uint16_t)buffer[0x16]);
            for(index = 0; index < 8; index++)
            {
                s_fatInfor.typeOfFAT[index] = buffer[index + 0x36];
            }
        }
    }
    HAL_UpdateSectorSize(s_fatInfor.bytesOfSector);
    /* Free buffer array */
    free(buffer);

    return readStatus;
}

static FATFS_Status_t FATFS_AddEntry(FATFS_EntryList_Struct_t **head, FATFS_Entry_Struct_t *buffer)
{
    FATFS_Status_t status = SUCCESSFULLY;
    FATFS_EntryList_Struct_t *nodeAdd = NULL;
    FATFS_EntryList_Struct_t *count = NULL;

    /* Allocate a node */
    nodeAdd = (FATFS_EntryList_Struct_t *)malloc(sizeof(FATFS_EntryList_Struct_t));
    if((nodeAdd) == NULL)
    {
        status = NOT_ENOUGH_MEMORY;
    }
    else
    {
        /* Save data */
        memcpy((nodeAdd)->entry.fileName, buffer->fileName, 11);
        (nodeAdd)->entry.attribute = buffer->attribute;
        memcpy((nodeAdd)->entry.reserved, buffer->reserved, 11);
        (nodeAdd)->entry.timeStamp = buffer->timeStamp;
        (nodeAdd)->entry.dateStamp = buffer->dateStamp;
        (nodeAdd)->entry.accessDate = buffer->accessDate;
        (nodeAdd)->entry.clusterHight = buffer->clusterHight;
        (nodeAdd)->entry.editTime = buffer->editTime;
        (nodeAdd)->entry.editDate = buffer->editDate;
        (nodeAdd)->entry.clusterLow = buffer->clusterLow;
        (nodeAdd)->entry.fileSize = buffer->fileSize;
        nodeAdd->next = NULL;
        /* Add nodeAdd to the end of linked list */
        if ((*head) == NULL)
        {
            *head = nodeAdd;
        }
        else
        {
            count = *head;
            while (count->next != NULL)
            {
                count = count->next;
            }
            count->next = nodeAdd;
        }
    }

    return status;
}

static FATFS_Status_t ReadFATValue(uint32_t *startCluster)
{
    uint16_t sectorOfFatValue = 0;
    uint16_t byteOfFatValue = 0;
    uint32_t FATValue = 0;
    uint8_t *buffer = NULL;
    FATFS_Status_t status = SUCCESSFULLY;

    sectorOfFatValue = ((*startCluster) * s_fatInfor.sectorsOfCluster * 1.5) / s_fatInfor.bytesOfSector;
    byteOfFatValue = (uint32_t)((*startCluster) * s_fatInfor.sectorsOfCluster * 1.5) % (uint32_t)s_fatInfor.bytesOfSector;
    /* If position of FAT: Sector1[end] & Sector2[start] -> Cannot read FAT if read 1 sector */
    buffer = (uint8_t*)malloc(s_fatInfor.bytesOfSector * 2);
    if(buffer == NULL)
    {
        status = NOT_ENOUGH_MEMORY;
    }
    else
    {
        if(HAL_ReadMultiSector(s_fatInfor.sectorsOfBoot + sectorOfFatValue, 2, buffer) != (s_fatInfor.bytesOfSector * 2))
        {
            status = READ_FAILED;
        }
        else
        {
            if((*startCluster) % 2 == 0) /* Start cluster is even number */
            {
                FATValue = (((uint32_t)(buffer[byteOfFatValue + 1] &~ (0xF0))) << 8) | (uint32_t)(buffer[byteOfFatValue]);
            }
            else /* Start cluster is odd number */
            {
                FATValue = ((uint32_t)(buffer[byteOfFatValue + 1]) <<  4) | (uint32_t)(buffer[byteOfFatValue] >> 4);
            }
        }
        free(buffer);
    }
    (*startCluster) = FATValue;

    return status;
}

static FATFS_Status_t FATFS_ReadRootDirectory(FATFS_EntryList_Struct_t **head)
{
    uint8_t *buffer = NULL;
    uint8_t *offset = NULL;
    uint32_t index = 0;
    FATFS_Status_t status = SUCCESSFULLY;

    buffer = (uint8_t*)malloc(32 * s_fatInfor.entrysOfRDET);
    if(buffer == NULL)
    {
        status = NOT_ENOUGH_MEMORY;
    }
    else
    {
        if(HAL_ReadMultiSector((s_fatInfor.sectorsOfBoot + s_fatInfor.sectorsOfFAT * s_fatInfor.numOfFAT), \
        ((s_fatInfor.entrysOfRDET * 32) / s_fatInfor.bytesOfSector), buffer) != (s_fatInfor.entrysOfRDET * \
        s_fatInfor.bytesOfSector * 32) / s_fatInfor.bytesOfSector)
        {
            status = READ_FAILED;
        }
        else
        {
            /* Add entry */
            offset = &buffer[index];
            while (offset[0] != 0 && status == SUCCESSFULLY)
            {
                if(offset[0x0B] == 0x00 || offset[0x0B] == 0x10) /* Main entry */
                {
                    status = FATFS_AddEntry(head, (FATFS_Entry_Struct_t *)offset);
                }
                /* Shift offset to next entry */
                index = index + 32;
                offset = &buffer[index];
            }
        }
        free(buffer);
    }

    return status;
}

static FATFS_Status_t FATFS_ReadSubDirectory(FATFS_EntryList_Struct_t **head, uint32_t startCluster)
{
    uint8_t *buffer = NULL;
    uint8_t *offset = NULL;
    uint32_t index = 0;
    FATFS_Status_t status = SUCCESSFULLY;

    buffer = (uint8_t*)malloc(s_fatInfor.bytesOfSector * s_fatInfor.sectorsOfCluster);
    if(buffer == NULL)
    {
        status = NOT_ENOUGH_MEMORY;
    }
    else
    {
        while (startCluster != 0xFFF && status == SUCCESSFULLY)
        {
            /* Read Cluster */
            if(HAL_ReadMultiSector((s_fatInfor.sectorsOfBoot) + (s_fatInfor.sectorsOfFAT * s_fatInfor.numOfFAT) + \
            ((s_fatInfor.entrysOfRDET * 32) / s_fatInfor.bytesOfSector) + (s_fatInfor.sectorsOfCluster * (startCluster - 2)),\
            s_fatInfor.sectorsOfCluster, buffer) != (s_fatInfor.sectorsOfCluster * s_fatInfor.bytesOfSector))
            {
                status = READ_FAILED;
            }
            else
            {
                index = 0;
                offset = &buffer[index];
                while (offset[0] != 0 && status == SUCCESSFULLY)
                {
                    status = FATFS_AddEntry(head, (FATFS_Entry_Struct_t *)offset);
                    index = index + 32;
                    offset = &buffer[index];
                }
                status = ReadFATValue(&startCluster);
            }
        }
        free(buffer);
    }
    
    return status;
}

FATFS_Status_t ReadDirectory(uint32_t startCluster, FATFS_EntryList_Struct_t **head)
{
    FATFS_Status_t status = SUCCESSFULLY;
    FATFS_EntryList_Struct_t *temp = NULL;

    /* Free linked list */
    while ((*head) != NULL)
    {
        temp = *head;
        *head = (*head)->next;
        free(temp);
    }
    /* ReadDirectory */
    if(startCluster == 0)
    {
        status = FATFS_ReadRootDirectory(head);
    }
    else
    {
        status = FATFS_ReadSubDirectory(head, startCluster);
    }

    return status;
}

FATFS_Status_t ReadFile(uint32_t startCluster, uint32_t size, uint8_t *buffer)
{
    uint8_t *bufferCluster = NULL;
    uint32_t indexCluster = 0;
    uint32_t indexBuffer = 0;
    FATFS_Status_t status = SUCCESSFULLY;

    while ((startCluster != 4095) && (indexBuffer < size) && (status == SUCCESSFULLY))
    {
        if(HAL_ReadMultiSector(s_fatInfor.sectorsOfBoot + s_fatInfor.sectorsOfFAT * s_fatInfor.numOfFAT + \
        (s_fatInfor.entrysOfRDET * 32) / (s_fatInfor.bytesOfSector) + s_fatInfor.sectorsOfCluster * (startCluster - 2),\
        s_fatInfor.sectorsOfCluster, (buffer + indexBuffer)) != (s_fatInfor.sectorsOfCluster * s_fatInfor.bytesOfSector))
        {
            status = READ_FAILED;
        }
        status = ReadFATValue(&startCluster);
        indexBuffer = indexBuffer + ((s_fatInfor.sectorsOfCluster) * (s_fatInfor.bytesOfSector));
    }

//     bufferCluster = (uint8_t*)malloc(s_fatInfor.bytesOfSector * s_fatInfor.sectorsOfCluster);
//     if(bufferCluster == NULL)
//     {
//         status = NOT_ENOUGH_MEMORY;
//     }
//     else
//     {
//         while ((startCluster != 4095) && (indexBuffer < size) && (status == SUCCESSFULLY))
//         {
//             indexCluster = 0;
//             if(HAL_ReadMultiSector(s_fatInfor.sectorsOfBoot + s_fatInfor.sectorsOfFAT * s_fatInfor.numOfFAT + \
//             (s_fatInfor.entrysOfRDET * 32) / (s_fatInfor.bytesOfSector) + s_fatInfor.sectorsOfCluster * (startCluster - 2),\
//             s_fatInfor.sectorsOfCluster, bufferCluster) != (s_fatInfor.sectorsOfCluster * s_fatInfor.bytesOfSector))
//             {
//                 status = READ_FAILED;
//             }
//             while(indexCluster < s_fatInfor.bytesOfSector * s_fatInfor.sectorsOfCluster && (indexBuffer < size))
//             {
//                 buffer[indexBuffer] = bufferCluster[indexCluster];
//                 indexBuffer++;
//                 indexCluster++;
//             }
//             status = ReadFATValue(&startCluster);
//         }
//         free(bufferCluster);
//     }

    return status;
}





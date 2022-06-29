#include "APP.h"
#include "math.h"
#include "stdint.h"
#define STARTCLUSTER(X,Y) (((X) << 16) | (Y))

uint8_t CheckSelect(FATFS_EntryList_Struct_t *head, uint32_t position);
void PrintFile(uint32_t cluster, uint32_t size);

static uint32_t StringToInt(char *ch)
{
    uint32_t num = 0;
    uint8_t len = strlen(ch);
    int i = 0;
    
    for(i = len - 1; i >= 0; i--)
    {
        num += (int)(ch[i] - '0') * pow(10,len - i - 1);
    }
    return num;
}

uint32_t InputInt()
{
    char ch[8] = "";
    uint8_t check = 1;
    int i = 0;
    uint8_t num = 0;
    
    while(check)
    {
        fflush(stdin);
        gets(ch);
        num = 0;
        
        /* Check input number */
        if((ch[0] < '0') || ch[0] > '9')
        {
            num++;
        }
        for(i = 1; ch[i] != 0; i++)
        {
                if((ch[i] < '0') || ch[i] > '9')
                {
                    num++;
                }
        }
        
        if(num == 0) /* num > 0 <=> Contains wrong characters */
        {
            check = 0; /* break */
        }
        else
        {
            printf("Error: Syntax error. Please re-enter: ");
        }
    }
    return StringToInt(ch);
}

FATFS_EntryList_Struct_t * FATFS_ReadFileAndDirectory(FATFS_EntryList_Struct_t *head, uint8_t select)
{
    uint8_t count = 1;
    FATFS_EntryList_Struct_t *temp = head;
    
    while (count != select)
    {
        temp = temp->next;
        count++;
    }
    if(temp->entry.attribute == 0x10)
    {
        system("cls");
        ReadDirectory(STARTCLUSTER((temp->entry.clusterHight), (temp->entry.clusterLow)), &head);
        DisplayDirectory(head);
    }
    else
    {
        system("cls");
        PrintFile(STARTCLUSTER((temp->entry.clusterHight), (temp->entry.clusterLow)), temp->entry.fileSize);
        printf("\n");
        system("pause");
        system("cls");
        DisplayDirectory(head);
    }
    
    return head;
}

void PrintFile(uint32_t cluster, uint32_t size)
{
    uint8_t *buffer = NULL;

    buffer = (uint8_t*)malloc((size/512 + 1) * 512);
    ReadFile(cluster, (size/512 + 1) * 512, buffer);
    for(int i = 0; i < size; i++)
    {
        printf("%c", buffer[i]);
    }
    free(buffer);
}
void RunApp()
{
    FATFS_EntryList_Struct_t *head = NULL;
    uint8_t select;
    FATFS_EntryList_Struct_t * FATFS_ReadFileAndDirectory(FATFS_EntryList_Struct_t *head, uint8_t select);
    
    HAL_Init("D:\\OneDrive - vnu.edu.vn\\FPT SoftWare\\Basic_C\\Mock2\\floppy.img");
        ReadDirectory(0, &head);
        DisplayDirectory(head);
        while(1)
        {
            do
            {
                printf("\nEnter your select: ");
                select = InputInt();
            } while (CheckSelect(head, select) != 0);
            head = FATFS_ReadFileAndDirectory(head, select);
        }
}

void DisplayDirectory(FATFS_EntryList_Struct_t *head)
{
    uint8_t no = 1;
    FATFS_EntryList_Struct_t *count = NULL;
    
    printf("---------------------------------------------------------------------------------------\n");
    printf("%-4s%-15s%-9s%-25s%-17s%-10s\n", "No", "Name", "Size", "Type", "Day Modified", "Time Modified");
    printf("---------------------------------------------------------------------------------------\n");
    for(count = head; count != NULL; count = count->next)
    {
        if(count->entry.fileName[0] == 0x2E)
        {
            if(count->entry.fileName[1] == 0x2E)
            printf("%-4d< BACK\n\n", no);
            else
            printf("%-4d%c REFRESH\n", no,236);
        }
        else
        {
            printf("%-4d", no);
            /* Print name */
                printf("%s - %x - %x - %x - %x - %x - %x - %x - %x - %x - %x - %x - %d \n", count->entry.fileName,count->entry.attribute, count->entry.reserved[0], count->entry.reserved[1], count->entry.timeStamp, count->entry.dateStamp\
                , count->entry.accessDate, count->entry.clusterHight, count->entry.editTime, count->entry.editDate, count->entry.editTime, count->entry.clusterLow, count->entry.fileSize);

        }
        no++;
    } 
}

uint32_t Elements(FATFS_EntryList_Struct_t *head)
{
    uint32_t elements = 0;
    FATFS_EntryList_Struct_t *temp = head;

    if(head != NULL)
    {
        while (temp != NULL) /* Browse all element */
        {
            temp = temp->next;
            elements++;
        }
    }
    return elements;
}

uint8_t CheckSelect(FATFS_EntryList_Struct_t *head, uint32_t position)
{
    uint8_t check = 0;
    uint32_t elements = Elements(head);

    if(position < 1 || position > elements) /* Limit input position 1 - numbers element of list */
    {
        check++;
        printf("Error: Input position is not in range 1 - %d!\n", elements);
    }
    return check;
}


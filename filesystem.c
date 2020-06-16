#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <unistd.h>
#include "filesystem.h"

void getDataAndTime(unsigned short *date, unsigned short *time_)
{
    time_t tempTime = time(NULL);
    struct tm *pTime = localtime(&tempTime);

    *date = ((((pTime->tm_year - 80) & 0b1111111) << 9) |
             (((pTime->tm_mon) & 0b1111) << 5) |
             ((pTime->tm_mday) & 0b11111));
    *time_ = (((pTime->tm_hour & 0b11111) << 11) |
              (((pTime->tm_min) & 0b111111) << 4) |
              ((pTime->tm_sec) & 0b1111));
}

void initDirFCB(pFcb dirFCB, char *name, unsigned short date, unsigned short time, int index)
{
    strcpy(dirFCB->filename, name);
    dirFCB->attribute = 0;
    dirFCB->date = date;
    dirFCB->time = time;
    dirFCB->length = 0;
    dirFCB->free = 1;
    dirFCB->fatIndex = index;
}

unsigned short SetFatTable(int index, unsigned short status)
{
    if (index > SIZE / BLOCKSIZE)
    {
        PrintError("set FAT ERROR", "%s");
        return FILE_SYSTEM_SET_FAT_ERROR;
    }
    pFat start = (pFat) & (myvhard[1 * BLOCKSIZE]);  //FAT1
    pFat start2 = (pFat) & (myvhard[3 * BLOCKSIZE]); //FAT2
    (start + index)->id = status;
    (start2 + index)->id = status;

    return FILE_SYSTEM_SET_FAT_OK;
}

void FcbToUser(pFcb tempFCB, pUseropen tempUSEROPEN)
{
    memcpy(tempUSEROPEN->filename, tempFCB->filename, sizeof(tempFCB->filename));
    memcpy(tempUSEROPEN->exname, tempFCB->exname, sizeof(tempFCB->exname));
    tempUSEROPEN->attribute = tempFCB->attribute;
    tempUSEROPEN->time = tempFCB->time;
    tempUSEROPEN->date = tempFCB->date;
    tempUSEROPEN->length = tempFCB->length;
    tempUSEROPEN->fatIndex = tempFCB->fatIndex;
    tempUSEROPEN->count = 0;
    tempUSEROPEN->fcbstate = 0;
    tempUSEROPEN->topenfile = 1;
}

/*
        1        3       5 
        |   1    |   2   |root|
*/
void my_format()
{
    unsigned date, time_;
    memset(&myvhard[1 * BLOCKSIZE], FREE, BLOCKSIZE * 2); //FAT1
    memset(&myvhard[3 * BLOCKSIZE], FREE, BLOCKSIZE * 2); //FAT2
    memset(&myvhard[5 * BLOCKSIZE], FREE, BLOCKSIZE);     //ROOT
    getDataAndTime(date, time_);
    pFcb root = (pFcb)&myvhard[5 * BLOCKSIZE];
    initDirFCB(root, "/", date, time_, 5);
    initDirFCB(root + 1, ".", date, time_, 5);
    initDirFCB(root + 2, "..", date, time_, 5);
    root->length = 3 * sizeof(fcb);
    (root + 1)->length = 3 * sizeof(fcb);

    FcbToUser((pFcb)&myvhard[5 * BLOCKSIZE], &openfilelist[0]);

    if (SetFatTable(5, END) == FILE_SYSTEM_SET_FAT_ERROR) // can not format Virtual disk , free it
        free(myvhard);
}

void startsys()
{
    myvhard = (char *)malloc(sizeof(char) * SIZE);
    FILE *fp = fopen("temp.dmp", "rb");
    if (fp)
    {
        fread(myvhard, SIZE, 1, fp);
        fclose(fp);
    }
    else
    {
        my_format();
        curdir = 0;
        startp = &myvhard[5 * BLOCKSIZE];
    }
    strcpy(currentdir, "/");
    memset(openfilelist, 0, sizeof(useropen) * 10);
    FcbToUser((pFcb)&myvhard[5 * BLOCKSIZE], &openfilelist[0]);
}

void test()
{
    unsigned short date, time_;
    getDataAndTime(&date, &time_);
    printf("time : %d \n, data %d", time_, date);
}

int main()
{
    test();
    printf("Welcome to beta filesystem!\n");
}

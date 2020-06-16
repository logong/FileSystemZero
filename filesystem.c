#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <unistd.h>
#include "filesystem.h"
void parseLocation(char **parseList, char *location);             // 解析路径
void getDateAndTime(unsigned short *date, unsigned short *time_); //获取时间 和 日期
void getAbsPath(char *Path, char *ret, bool isDir);               //获取绝对路径
pFcb FindFreeFCB(pFcb DirFCB);                                    //找到空闲FCB
pFcb ExFcb(pFcb DirFCB);                                          //拓展FCB

void getDateAndTime(unsigned short *date, unsigned short *time_)
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

void getAbsPath(char *Path, char *ret, bool isDir)
{
    char *ori[MAXDEPTH] = {
        0,
    };
    char *purpose[MAXDEPTH] = {
        0,
    };
    char *final[MAXDEPTH] = {
        0,
    };
    int oriLength;
    parseLocation(ori, currentdir); //现在所在的目录
    parseLocation(purpose, Path);
    for (oriLength = 0; ori[oriLength]; oriLength++)
        ;
    memcpy(final, ori, sizeof(final));
    for (int i = 0; purpose[i]; i++)
    {
        if (!strcmp(purpose[i], "."))
        {
            continue;
        }
        else if (!strcmp(purpose[i], ".."))
        {
            if (oriLength == 1)
                continue;
            final[--oriLength] = NULL;
        }
        else if (!strcmp(purpose[i], "/"))
            continue;
        else
        {
            final[oriLength++] = purpose[i];
        }
    }
    memset(ret, 0, 80);
    for (int i = 0; final[i]; i++)
    {
        strcat(ret, final[i]);
        if (i)
            strcat(ret, "/");
    }
    if (!isDir)
    {
        ret[strlen(ret) - 1] = 0;
    }
    for (int i = 0; ori[i];
         free(ori[i++]))
        ;
    for (int i = 0; purpose[i]; free(purpose[i++]))
        ;
}

int getFreeOpen()
{
    for (int i = 0; i < MAXOPENFILE; i++)
    {
        if (!openfilelist[i].topenfile)
            return i;
    }
    for (int i = 0; i < MAXOPENFILE; i++)
    {
        if (i == curdir)
            continue;
        return i;
    }
}

void initDirFCB(pFcb dirFCB, char *name, unsigned short date, unsigned short time, int index)
{
    strcpy(dirFCB->filename, name);
    dirFCB->attribute = 0;
    dirFCB->date = date;
    dirFCB->time = time;
    dirFCB->length = 0;
    dirFCB->free = located_fcb;
    dirFCB->fatIndex = index;
}

void initNormalFCB(pFcb fileFCB, char *name, char *exname, unsigned short date, unsigned short time,
                   unsigned long length, int index)
{
    strcpy(fileFCB->filename, name);
    if (exname)
        strcpy(fileFCB->exname, exname);
    fileFCB->attribute = 1;
    fileFCB->date = date;
    fileFCB->time = time;
    fileFCB->length = length;
    fileFCB->free = located_fcb;
    fileFCB->fatIndex = index;
}
unsigned short SetFatTable(int index, unsigned short status)
{
    if (index > SIZE / BLOCKSIZE)
        return FILE_SYSTEM_SET_FAT_ERROR;
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

int FindFreeBlock()
{
    pFat start = (pFat) & (myvhard[1 * BLOCKSIZE]);
    for (int i = 6; i < 1000; i++)
    {
        if (!(start + i)->id)
        {
            return i;
        }
    }
    return -1;
}

void parseLocation(char **parseList, char *location)
{
    int i = 0;
    char *tempLoc = location;
    while ((tempLoc = strchr(location, '/')))
    {
        int length = (tempLoc - location) ? (tempLoc - location) : 1;
        char *temp = (char *)malloc(sizeof(char) * (length + 1));
        memcpy(temp, location, length);
        temp[length] = 0;
        parseList[i++] = temp;
        while (*(location = ++tempLoc) == '/')
            ;
    }
    if (*location)
    {
        parseList[i] = strdup(location);
    }
}

void splitPath(char *Path, char *dir, char *filename, char *exname)
{
    char path[80] = {
        0,
    };
    if (dir)
    {
        getAbsPath(Path, path, 0);
        char *final = strrchr(path, '/');
        memcpy(dir, path, final - path + 1);
        dir[final - path + 1] = 0;
    }
    char *final = strrchr(Path, '/');
    if (!final)
    {
        final = Path;
    }
    else if (final == Path)
        ;
    else
    {
        final = final + 1;
    }
    char *ex = NULL;
    int length = strlen(final);
    if (exname && length > 2)
    {
        if (ex = strchr(final, '.'))
        {
            strcpy(exname, ex + 1);
            length -= (ex - final + 1);
        }
    }
    memset(filename, 0, 8);
    memcpy(filename, final, strlen(final) - length - 1);
}

/* 通过fat_index遍历dir*/
pFcb TraverseDir(int fat_index, char *filename, char *exname)
{
    char finalName[12];
    for (int index = fat_index; index != END; index = ((pFat)(&myvhard[1 * BLOCKSIZE]) + index)->id)
    {
        for (int i = 0; i < FCBNUM; i++)
        {
            pFcb tempFcb = ((pFcb)&myvhard[index * BLOCKSIZE]) + i;
            if (!tempFcb->free)
                continue;
            if (!strcmp(tempFcb->filename, filename))
            {
                if (exname)
                {
                    if (strcmp(tempFcb->exname, exname))
                        continue;
                }
                return tempFcb;
            }
        }
    }
    return NULL;
}

/*检查新建目录/文件是否重名 返回找到的Fcb*/
pFcb FindFcb(char *purposeName, pFcb *DirFCB)
{
    char *tempLoc[MAXDEPTH] = {
        0,
    };
    parseLocation(tempLoc, purposeName);
    int fatIndex, length;
    pFcb retNum, dirFcb;
    char name[8] = {
        0,
    };
    char exname[3] = {
        0,
    };
    if (!strcmp(tempLoc[0], "/"))
    {
        fatIndex = 5;
    }
    else
    {
        fatIndex = openfilelist[curdir].fatIndex;
    }

    //根据文件名寻找
    dirFcb = (pFcb)&myvhard[fatIndex * BLOCKSIZE];
    for (int i = 0; tempLoc[i]; i++)
    {
        if (!tempLoc[i + 1])
        {
            if (DirFCB)
            {
                *DirFCB = dirFcb;
            }
            splitPath(tempLoc[i], NULL, name, exname);
            retNum = TraverseDir(fatIndex, name, exname);
        }
        else
        {
            dirFcb = TraverseDir(fatIndex, tempLoc[i], NULL);
            if (!dirFcb || dirFcb->attribute)
            {
                retNum = NULL;
                break;
            }
        }
        fatIndex = dirFcb->fatIndex;
    }

    for (int i = 0; tempLoc[i];
         free(tempLoc[i++]))
        ;
    return retNum;
}

pFcb FindFreeFcb(pFcb DirFCB)
{
    int fatIndex = DirFCB->fatIndex;
    for (int index = fatIndex; index != END; index = ((pFat)(&myvhard[1 * BLOCKSIZE]) + index)->id)
    {
        for (int i = 0; i < FCBNUM; i++)
        {
            pFcb temp = ((pFcb)&myvhard[index * BLOCKSIZE]) + i;
            if (temp->free == free_fcb)
                return temp;
        }
        return ExFcb(DirFCB);
    }
}

/*拓展fcb*/
pFcb ExFcb(pFcb DirFCB)
{
    int fatIndex = DirFCB->fatIndex;
    int blockIndex;
    for (; ((pFat)(&myvhard[1 * BLOCKSIZE]) + fatIndex)->id != END; fatIndex = ((pFat)(&myvhard[1 * BLOCKSIZE]) + fatIndex)->id)
        ;
    if ((blockIndex = FindFreeBlock()) == -1)
    {
        printf("The disk is full!!!!!!\n");
        return NULL;
    }
    SetFatTable(blockIndex, END);
    SetFatTable(fatIndex, blockIndex);
    return (pFcb)&myvhard[blockIndex * BLOCKSIZE];
}

/*
        1        3       5 
        |   1    |   2   |root|
*/
void my_format()
{
    unsigned short date, time_;
    memset(&myvhard[1 * BLOCKSIZE], FREE, BLOCKSIZE * 2); //FAT1
    memset(&myvhard[3 * BLOCKSIZE], FREE, BLOCKSIZE * 2); //FAT2
    memset(&myvhard[5 * BLOCKSIZE], FREE, BLOCKSIZE);     //ROOT
    getDateAndTime(&date, &time_);
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
    memset(openfilelist, 0, sizeof(useropen) * 10);
    FcbToUser((pFcb)&myvhard[5 * BLOCKSIZE], &openfilelist[0]);
}

void my_mkdir(char *dirname)
{
    char *temp[MAXDEPTH] = {
        0,
    };
    int length;
    pFcb dirFcb = NULL;
    unsigned short date, time;
    int index;
    parseLocation(temp, dirname);
    if (FindFcb(dirname, &dirFcb))
    {
        printf("%s: It's already exist!\n", dirname);
        return;
    }
    for (length = 0; temp[length++];)
        ;
    length--;
    if (!dirFcb)
    {
        printf("%s: No such file or dictonary\n", dirname);
        return;
    }

    if ((index = FindFreeBlock()) == -1)
        return;

    memset(&myvhard[BLOCKSIZE * index], 0, BLOCKSIZE);
    SetFatTable(index, END);
    getDateAndTime(&date, &time);

    //建立目录项
    initDirFCB((pFcb)(&myvhard[BLOCKSIZE * index]), ".", date, time, index);
    initDirFCB((pFcb)(&myvhard[BLOCKSIZE * index]) + 1, "..", date, time, dirFcb->fatIndex);

    ((pFcb) & (myvhard[BLOCKSIZE * index]))->length = 2 * sizeof(fcb); // 为 . .. 分配空间

    pFcb newDirFCB = FindFreeFcb(dirFcb);
    initDirFCB(newDirFCB, temp[--length], date, time, index);
    newDirFCB->length = 2 * sizeof(fcb);
    dirFcb->length += sizeof(fcb);
    if (dirFcb->fatIndex == 5)
        (dirFcb + 1)->length += sizeof(fcb);
    for (int i = 0; temp[i];
         free(temp[i++]))
        ;
}

void my_cd(char *dirname)
{
    pFcb temp = FindFcb(dirname, NULL);
    if (!temp)
    {
        printf("%s: No such file or dictonary!\n", dirname);
        return;
    }
    if (temp)
    {
        if (temp->attribute)
        {
            printf("%s: Not such dictonary!\n", dirname);
            return;
        }
        int tempNum = curdir;
        curdir = getFreeOpen();
        memset(&openfilelist[tempNum], 0, sizeof(useropen));
        FcbToUser(temp, &openfilelist[curdir]);
        getAbsPath(dirname, currentdir, true);
    }
}

/*  只能删除不空的文件 */
void my_rmdir(char *dirname)
{
    pFcb Dir, parentDir; // 当前 目录和父目录
    int index;
    if (Dir = FindFcb(dirname, &parentDir))
    {
        if (!Dir->attribute)
        {
            for (int index = Dir->fatIndex; index != END; index = ((pFat)(&myvhard[1 * BLOCKSIZE]) + index)->id)
            {
                for (int i = 0; i < FCBNUM; i++)
                {
                    pFcb tempFcb = ((pFcb)&myvhard[index * BLOCKSIZE]) + i;
                    if (!strcmp(tempFcb->filename, "..") || !strcmp(tempFcb->filename, "."))
                        continue;
                    if (tempFcb->free == located_fcb)
                    {
                        printf("rmdir failed cause this dir is not empty\n");
                        return;
                    }
                }
            }
            for (index = Dir->fatIndex; ((pFat)(&myvhard[1 * BLOCKSIZE]) + index)->id != END; index = ((pFat)(&myvhard[1 * BLOCKSIZE]) + index)->id)
                SetFatTable(index, 0);

            SetFatTable(index, 0);
            Dir->free = free_fcb;
            parentDir->length -= sizeof(fcb);
            if (parentDir->fatIndex == 5)
                (parentDir + 1)->length -= sizeof(fcb);
        }
        else
        {
            printf("rmdir failed: %s is not a dictonary\n", dirname);
            return;
        }
    }
    else
    {
        printf("No such file or dictonary\n");
    }
}

void my_ls(void)
{
    int index = openfilelist[curdir].fatIndex;
    for (; index != END; index = ((pFat)(&myvhard[1 * BLOCKSIZE]) + index)->id)
    {
        for (int i = (index == 5 ? 1 : 0); i < FCBNUM; i++)
        {
            pFcb tempFcb = ((pFcb)&myvhard[index * BLOCKSIZE]) + i;
            if (!tempFcb->free)
                continue;
            if (tempFcb->exname[0] != '\x00')
            {
                printf("%s.%s\t%d.%d.%d:%d:%d:%d\t%d\t%s\n",
                       tempFcb->filename,
                       tempFcb->exname,
                       ((tempFcb->date >> 9) & 0b1111111) + 1980,
                       ((tempFcb->date >> 5) & 0b1111) + 1,
                       (tempFcb->date & 0b11111),
                       (tempFcb->time >> 11) & 0b11111,
                       (tempFcb->time >> 4) & 0b111111,
                       tempFcb->time & 0b1111,
                       tempFcb->length,
                       typeStr[tempFcb->attribute]);
            }
            else
            {
                printf("%s\t%d.%d.%d:%d:%d:%d\t%d\t%s\n",
                       tempFcb->filename,
                       ((tempFcb->date >> 9) & 0b1111111) + 1980,
                       ((tempFcb->date >> 5) & 0b1111) + 1,
                       (tempFcb->date & 0b11111),
                       (tempFcb->time >> 11) & 0b11111,
                       (tempFcb->time >> 4) & 0b111111,
                       tempFcb->time & 0b1111,
                       tempFcb->length,
                       typeStr[tempFcb->attribute]);
            }
        }
    }
}

void my_create(char *filename)
{
    pFcb File, parentDir; // 文件
    unsigned short date, time;
    int length;
    char tempName[8];
    char exname[3];
    if (File = FindFcb(filename, &parentDir))
    {
        printf("%s: File exist!\n", filename);
        return;
    }
    else
    {
        if (!parentDir)
        {
            printf("%s: No such file or dictonary!\n", filename);
            return;
        }
        int fatIndex = FindFreeBlock(); //先找空块
        SetFatTable(fatIndex, END);
        pFcb fileFCB = FindFreeFcb(parentDir);
        getDateAndTime(&date, &time);
        splitPath(filename, NULL, tempName, exname);
        printf("%s\n", tempName);
        initNormalFCB(fileFCB, tempName, exname, date, time, 0, fatIndex);
        parentDir->length += sizeof(fcb);
        if (parentDir->fatIndex == 5)
            (parentDir + 1)->length += sizeof(fcb);
    }
}
void my_rm(char *filename)
{
    pFcb FIle = NULL, parentDir = NULL;
    int index;
    if (!(FIle = FindFcb(filename, &parentDir)))
    {
        printf("%s: No such file or dictnary!\n", filename);
        return;
    }
    else
    {
        if (!FIle->attribute)
        {
            printf("%s: It's not a file!\n", filename);
            return;
        }
        else
        {
            for (int i = 0; i < MAXOPENFILE; i++)
            {
                if (!openfilelist[i].topenfile || !openfilelist[i].attribute)
                    continue;
                if (!strcmp(FIle->filename, openfilelist[i].filename) && !strcmp(FIle->exname, openfilelist[i].exname))
                {
                    printf("File is opened! Please close it first!\n");
                    return;
                }
            }
            for (index = FIle->fatIndex; ((pFat)(&myvhard[1 * BLOCKSIZE]) + index)->id != END;)
            {
                int temp = index;
                index = ((pFat)(&myvhard[1 * BLOCKSIZE]) + index)->id;
                SetFatTable(temp, 0);
            }

            SetFatTable(index, 0);
            FIle->free = 0;
            parentDir->length -= sizeof(fcb);
            if (parentDir->fatIndex == 5)
                (parentDir + 1)->length -= sizeof(fcb);
        }
    }
}

void my_open(char *filename)
{
    char path[80] = {
        0,
    };
    getAbsPath(filename, path, false);
    char dir[80] = {
        0,
    },
         name[8] = {
             0,
         },
         exname[3] = {
             0,
         };
    pFcb File;
    splitPath(path, dir, name, exname);
    for (int i = 0; i < MAXOPENFILE; i++)
    {
        if (!openfilelist[i].topenfile)
            continue;
        if (!strcmp(dir, openfilelist[i].dir) && !strcmp(name, openfilelist[i].filename) &&
            !strcmp(exname, openfilelist[i].exname))
        {
            printf("File had opened!\n");
            return;
        }
    }
    File = FindFcb(filename, NULL);
    if (!File)
    {
        printf("No such file!\n");
        return;
    }
    int index = getFreeOpen();// 寻找空闲文件描述符
    printf("Now get fd:%d\n", index);
    FcbToUser(File, &openfilelist[index]);
    strcpy(openfilelist[index].dir, dir);
}
int main()
{
    printf("Welcome to beta filesystem!\n%s\n", Version);
    startsys();
    // my_ls();
    // my_cd("233");
    // my_mkdir("233");
    // my_cd("233");
    // printf("%s\n", currentdir);
    // my_rmdir("233");
    // my_cd("..");
    // printf("%s\n", currentdir);
    my_create("test.ve");
    my_ls();
    my_open("test.ve");
    my_rm("test.ve");
    my_ls();
}

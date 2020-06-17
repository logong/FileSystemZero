#define BLOCKSIZE 1024 // 磁盘块大小
#define SIZE 1024000 //总大小
#define END 65535 
#define FREE 0
#define MAXOPENFILE 10 //最大打开数目
#define FCBNUM 32 // FCB 数量
#define MAXDEPTH 32
#define Version "FileSystem_0.0.1"
 



typedef struct FCB
{
    char filename[8];
    char exname[3];
    unsigned char attribute; //0     1     2       3
    unsigned short time;     //dir   file  system   ---
    unsigned short date;
    unsigned short fatIndex;
    unsigned int length;
    int useless;
    char free; //0 free 1 located
}fcb, *pFcb;
#pragma pack(8)

typedef struct FAT
{
    unsigned short id;
}fat, *pFat;

typedef struct USEROPEN
{
    char filename[8];
    char exname[3];
    unsigned char attribute;
    unsigned short time;
    unsigned short date;
    unsigned short fatIndex;
    unsigned long length;
    char dir[80];
    int count;
    char fcbstate; //0 free 1 located
    char topenfile;
}useropen, *pUseropen;

// typedef struct BLOCK0
// {
//     char information[200];
//     unsigned short root;
//     unsigned char *startblock;
// }block0, *pBlock0;

typedef struct funcStruct
{
    char *name;
    void (*func)();
    int type;
    char *describe;
}func, *pFunc;

unsigned char *myvhard;
useropen openfilelist[MAXOPENFILE];
int curdir;
char currentdir[80];
unsigned char *startp;
char buf[BLOCKSIZE];

char *typeStr[] = {"dir", "file"};


void my_format();
void startsys();



enum{
    dir_type,
    file_type
};
enum 
{
    FILE_SYSTEM_SET_FAT_OK,
    FILE_SYSTEM_SET_FAT_ERROR
};

enum{
    free_fcb,
    located_fcb
};
#include "pathsystem.hpp"
#include "filesystem.hpp"
#include "memory.h"
#include "api.h"


typedef struct directory_info
{
    WCHAR* Directory;
    SLIST_ENTRY(directory_info) Entries;
}DIRECTORY_INFO, *PDIRECTORY_INFO;

typedef struct drive_info
{
    WCHAR* Filename;
    WCHAR* Exst;
    WCHAR* FullPath;
    WCHAR* Path;
    SLIST_ENTRY(drive_info) Entries;
}DRIVE_INFO, * PDRIVE_INFO;

SLIST_HEAD(, drive_info) DriveList;
SLIST_HEAD(, directory_info) DirectoryList;



STATIC WCHAR* MakeSearchMask
(
    __in WCHAR* Directory
)
{
    INT DirLen = memory::StrLen(Directory);
    WCHAR wch = Directory[DirLen - 1];
    
    if (wch != L'\\')
    {
        WCHAR* Mask = (WCHAR*)memory::m_new(DirLen + 3);
        wmemcpy_s(Mask, DirLen, Directory, DirLen);
        wmemcpy_s(&Mask[DirLen], 3, L"\\*", 3);
        return Mask;
    }
    else if (wch == L'\\')   
    {
        WCHAR* Mask = (WCHAR*)memory::m_new(DirLen + 2);
        wmemcpy_s(Mask, DirLen, Directory, DirLen);
        wmemcpy_s(&Mask[DirLen], 2, L"*", 2);
        return Mask;
    }
    
    
    return NULL;
}


STATIC WCHAR* MakePath
(
    __in WCHAR* Directory,
    __in INT DirLen,
    __in WCHAR* Filename
)
{
    INT FileLen = Filename == NULL ? 0 : memory::StrLen(Filename);     
    WCHAR* Path = (WCHAR*)memory::m_new(DirLen + FileLen + 1);    
    wmemcpy_s(Path, DirLen, Directory, DirLen);
    wmemcpy_s(&Path[DirLen], FileLen, Filename, FileLen);
    Path[DirLen + FileLen] = '\0';    
        
    return Path;
}

STATIC WCHAR* MakeExs
(
    __in WCHAR* Filename
)
{    
    INT Pointer = 0;
    INT FileLen = memory::StrLen(Filename);
    for (INT i = FileLen; i != 0; --i)
    {
        if (Filename[i] == L'.')
        {
            Pointer = i + 1;
            break;
        }
    }
    if (Pointer == 0)
    {
        WCHAR* Exs = (WCHAR*)memory::m_new(7);
        wmemcpy_s(Exs, 7, L".clown", 7);
        return Exs;
    }

    INT ExsSize = FileLen - Pointer + 1;
    WCHAR* Exs = (WCHAR*)memory::m_new(ExsSize + 1);
    wmemcpy_s(Exs, ExsSize, &Filename[Pointer - 1], ExsSize);
    Exs[ExsSize] = '\0';
    return Exs;
}



VOID SearchFiles
(
    __in WCHAR* StartDirectory,
    PDIRECTORY_INFO StartDirectoryInfo,
    PDRIVE_INFO DriveInfo
)
{        
    StartDirectory = MakeSearchMask(StartDirectory);
    if (!StartDirectoryInfo || !DriveInfo)
    {
        return;
    }
    StartDirectoryInfo->Directory = StartDirectory;
    SLIST_INSERT_HEAD(&DirectoryList, StartDirectoryInfo, Entries);    
        
    WIN32_FIND_DATA FindData;
    PDIRECTORY_INFO DirectoryInfo = SLIST_FIRST(&DirectoryList);
    if (DirectoryInfo == NULL)
    {
        return;
    }

    WCHAR* CurrentDirectory = DirectoryInfo->Directory;
    INT DirLen = memory::StrLen(CurrentDirectory);
    HANDLE hSearchFile = pFindFirstFileW(CurrentDirectory, &FindData);
    if (hSearchFile == INVALID_HANDLE_VALUE)
    {
        printf_s("FindFirstFile fails in directory %ls. GetLastError = %lu.", StartDirectory, pGetLastError());
        return;
    }
    
    do
    {
        if (!plstrcmpW(FindData.cFileName, L".") ||
            !plstrcmpW(FindData.cFileName, L"..") ||
            FindData.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
        {
            continue;
        }
        if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {            
            WCHAR* cDirectory = (WCHAR*)memory::m_new(260);
            wmemcpy_s(cDirectory, 260, FindData.cFileName, 260);
            
            PDIRECTORY_INFO Directory = new DIRECTORY_INFO;            
            Directory->Directory = MakePath(CurrentDirectory, DirLen - 1, cDirectory);          
            SLIST_INSERT_HEAD(&DirectoryList, Directory, Entries);            
            ++pathsystem::fs->dir;
        }
        else
        {
            WCHAR* cFilename = (WCHAR*)memory::m_new(260);
            wmemcpy_s(cFilename, 260, FindData.cFileName, 260);
            
            PDRIVE_INFO DriveInfo = new DRIVE_INFO;
            DriveInfo->Filename = cFilename;
            DriveInfo->Exst = MakeExs(cFilename);
            DriveInfo->FullPath = MakePath(CurrentDirectory, DirLen - 1, cFilename);            
            DriveInfo->Path = MakePath(CurrentDirectory, DirLen - 2, NULL);

            SLIST_INSERT_HEAD(&DriveList, DriveInfo, Entries);

            ++pathsystem::fs->fle;
        }

    } while (pFindNextFileW(hSearchFile, &FindData));     
    SLIST_REMOVE(&DirectoryList, StartDirectoryInfo, directory_info, Entries);
    pFindClose(hSearchFile);
}

STATIC VOID print_end
(
    PDIRECTORY_INFO StartDirectoryInfo,
    PDRIVE_INFO DriveInfo
)
{
    wprintf_s(L"\nDriveInfo\n\n");
    SLIST_FOREACH(DriveInfo, &DriveList, Entries)
    {
        printf_s("PathFile:  %ls\n", DriveInfo->Path);
        printf_s("Filename:\t%ls\n", DriveInfo->Filename);
    }
    
    wprintf_s(L"\nDirectories\n\n");
    SLIST_FOREACH(StartDirectoryInfo, &DirectoryList, Entries)
    {
        printf_s("Directory:\t%ls\n", StartDirectoryInfo->Directory);        
    }

    printf_s("\nCount Files: %d\n", pathsystem::fs->fle);
}

STATIC CHAR* MultyToWideChar(WCHAR* Source)
{
    INT Len = memory::StrLen(Source) + 1;
    CHAR* buf = (CHAR*)memory::m_new(Len);
    wcstombs(buf, Source, Len);
    return buf;
}


DWORD WINAPI pathsystem::StartLocalSearch
(
    const char* arg[]
)
{
    filesystem::FILE_INFO FileInfo;
    if (memory::StrStrC(arg[2], "-file"))
    {
        INT FileSize = 0;        
        INT SizePath;
        INT SizeArg = memory::StrLen(arg[1]);
        for (SizePath = SizeArg; SizePath != 0; --SizePath)
        {
            if (arg[1][SizePath] == '\\')
            {                
                FileSize = SizeArg - SizePath;
                break;
            }
        }        
        if (!FileSize || !SizePath)
        {
            printf_s("Check filename %s", arg[1]);
            return EXIT_FAILURE;
        }

        CHAR* Filename = (CHAR*)memory::m_new(FileSize);
        if (!Filename)
        {
            printf_s("Heap crash %s", arg[1]);
            return EXIT_FAILURE;
        }
        memory::m_memcpy(Filename, &arg[1][SizeArg - FileSize + 1], FileSize);


        INT Pointer = 0;
        for (INT i = FileSize; i != 0; --i)
        {
            if (Filename[i] == L'.')
            {
                Pointer = i + 1;
                break;
            }
        }
        INT ExsSize = FileSize - Pointer + 1;
        CHAR* Exs = (CHAR*)memory::m_new(ExsSize + 1);
        memcpy_s(Exs, ExsSize, &Filename[Pointer - 1], ExsSize);
        Exs[ExsSize] = '\0';        

                      
        CHAR* Path = (CHAR*)memory::m_new(SizePath + 1);
        if (!Path)
        {
            printf_s("Heap crash %s", arg[1]);
            return EXIT_FAILURE;
        }
        memory::m_memcpy(Path, arg[1], SizePath);
        Path[SizePath] = '\0';        
        FileInfo = filesystem::GenKey(arg[3], arg[4]);
        filesystem::map_init(arg[1], arg[3], arg[4], Path, Filename, Exs, &FileInfo);                 
        

        
        memory::m_delete(Exs);
        memory::m_delete(Filename);
        memory::m_delete(Path);
        return EXIT_SUCCESS;
    }

    PDIRECTORY_INFO StartDirectoryInfo = new DIRECTORY_INFO;
    PDRIVE_INFO DriveInfo = new DRIVE_INFO;
    SLIST_INIT(&DriveList);
    SLIST_INIT(&DirectoryList);
    WCHAR* Start = memory::m_wchar(arg[1], memory::StrLen(arg[1]));    
    SearchFiles(Start, StartDirectoryInfo, DriveInfo);    
    
    if (memory::StrStrC(arg[2], "-dir"))
    {
        for (DriveInfo = SLIST_FIRST(&DriveList); DriveInfo; DriveInfo = SLIST_NEXT(DriveInfo, Entries))
        {                
            FileInfo = filesystem::GenKey(arg[3], arg[4]);
            filesystem::map_init(MultyToWideChar(DriveInfo->FullPath), arg[3], arg[4],
                arg[1], MultyToWideChar(DriveInfo->Filename), MultyToWideChar(DriveInfo->Exst), &FileInfo);
        }
    }
    else if (memory::StrStrC(arg[2], "-indir"))
    {        
        for (int i = 0; i < pathsystem::fs->dir; ++i)
        {
            for (StartDirectoryInfo = SLIST_FIRST(&DirectoryList); StartDirectoryInfo; StartDirectoryInfo = SLIST_NEXT(StartDirectoryInfo, Entries))        
            {
                PDIRECTORY_INFO sub_dir = new DIRECTORY_INFO;
                PDRIVE_INFO sub_drive = new DRIVE_INFO;
                if (!sub_dir || !sub_drive)
                    continue;
                sub_dir->Directory = StartDirectoryInfo->Directory;
                SearchFiles(sub_dir->Directory, sub_dir, sub_drive);                
                printf_s("Start mapping indir:\t%ls\n", sub_dir->Directory);
                SLIST_REMOVE(&DirectoryList, StartDirectoryInfo, directory_info, Entries);
            }
        }        

        SLIST_FOREACH(DriveInfo, &DriveList, Entries)
        {
            FileInfo = filesystem::GenKey(arg[3], arg[4]);
            filesystem::map_init(MultyToWideChar(DriveInfo->FullPath), arg[3], arg[4],
                arg[1], MultyToWideChar(DriveInfo->Filename), MultyToWideChar(DriveInfo->Exst), &FileInfo);            
        }
        
        
        memory::m_delete(Start);
        delete StartDirectoryInfo;
        delete DriveInfo;
        return EXIT_SUCCESS;

    }
    
    print_end(StartDirectoryInfo, DriveInfo);
    
    memory::m_delete(Start);    
    delete StartDirectoryInfo;
    delete DriveInfo;
    return EXIT_SUCCESS;
}

/*
    SLIST_HEAD(, drive_info) DriveList;
    SLIST_INIT(&DriveList);

    SLIST_HEAD(, directory_info) DirectoryList;
    SLIST_INIT(&DirectoryList);

    /*PDIRECTORY_INFO StartDirectoryInfo = new DIRECTORY_INFO;
    PDRIVE_INFO DriveInfo = new DRIVE_INFO;
*/

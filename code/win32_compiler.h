/* date = July 5th 2022 10:08 pm */

#ifndef WIN32_COMPILER_H
#define WIN32_COMPILER_H

#include <stdint.h>

#define internal static
#define local static
#define global static

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef uint8_t u8;
#define MAX_U8 0xFF
typedef uint16_t u16;
#define MAX_U16 0xFFFF
typedef uint32_t u32;
#define MAX_U32 0xFFFFFFFF
typedef uint64_t u64;
#define MAX_U64 0xFFFFFFFFFFFFFFFF

typedef u8 b8;
typedef u16 b16;
typedef u32 b32;
typedef u64 b64;

typedef float r32;
typedef double r64;

#define Assert(Expression) if(!(Expression)) {*(int *)0 = 0;}

#define Kilobytes(Value) ((Value)*1024LL)
#define Megabytes(Value) (Kilobytes(Value)*1024LL)
#define Gigabytes(Value) (Megabytes(Value)*1024LL)
#define Terabytes(Value) (Gigabytes(Value)*1024LL)

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

struct Buffer
{
    u8 *memory;
    u8 *end;
    u32 size;
};

Buffer
create_buffer
(u32 size, s32 permission)
{
    Buffer buffer = {};
    buffer.memory = (u8 *)(VirtualAlloc(0, (SIZE_T)size, MEM_COMMIT | MEM_RESERVE, (DWORD)permission));
    Assert(buffer.memory);
    buffer.end = buffer.memory;
    buffer.size = size;
    
    return(buffer);
}

inline u32
SafeTruncateU64toU32(u64 Value)
{
    Assert(Value <= MAX_U32);
    u32 Result = (u32)Value;
    return(Result);
}

struct read_file_result
{
    u32 ContentsSize;
    void *Contents;
};

void
Win32FreeMemory
(void *Memory)
{
    if(Memory)
    {
        VirtualFree(Memory, 0, MEM_RELEASE);
    }
}

read_file_result
Win32ReadEntireFile
(char *FileName)
{
    read_file_result Result = {};
    
    HANDLE FileHandle = CreateFileA(FileName, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if(FileHandle != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER FileSize;
        if(GetFileSizeEx(FileHandle, &FileSize))
        {
            u32 FileSize32 = SafeTruncateU64toU32((u64)(FileSize.QuadPart));
            Result.Contents = VirtualAlloc(0, FileSize32, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
            if(Result.Contents)
            {
                DWORD BytesRead;
                if(ReadFile(FileHandle, Result.Contents, FileSize32, &BytesRead, 0) && (FileSize32 == BytesRead))
                {
                    Result.ContentsSize = FileSize32;
                }
                else
                {
                    Win32FreeMemory(Result.Contents);
                    Result.Contents = 0;
                }
            }
            else
            {
                Assert(!"Failed to allocate memory for file read.\n");
            }
        }
        else
        {
            Assert(!"Failed to get file size after opening.\n");
        }
        
        CloseHandle(FileHandle);
    }
    else
    {
        Assert(!"Failed to open file.\n");
    }
    
    return(Result);
}

b32
win32WriteEntireFile
(char *FileName, u32 MemorySize, void *Memory)
{
    b32 Result = false;
    
    HANDLE FileHandle = CreateFileA(FileName, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
    if(FileHandle != INVALID_HANDLE_VALUE)
    {
        DWORD BytesWritten;
        if(WriteFile(FileHandle, Memory, MemorySize, &BytesWritten, 0))
        {
            Result = (BytesWritten == MemorySize);
        }
        else
        {
            Assert(!"Failed to write file.\n");
        }
        
        CloseHandle(FileHandle);
    }
    else
    {
        Assert(!"Failed to create file to write.\n");
    }
    
    return(Result);
}

#endif //WIN32_COMPILER_H

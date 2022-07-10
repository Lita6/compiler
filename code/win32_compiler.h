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

u8 *
buffer_allocate
(Buffer *buffer, u32 amount)
{
    
    Assert((buffer->end + amount) <= (buffer->memory + buffer->size));
    
    u8 *Result = buffer->end;
    buffer->end += amount;
    
    return(Result);
}

#define define_buffer_append(Type) \
inline void \
buffer_append_##Type \
(Buffer *buffer, Type value) \
{ \
Assert((buffer->end + sizeof(Type)) <= (buffer->memory + buffer->size)); \
*(Type *)buffer->end = value; \
buffer->end += sizeof(Type); \
}

define_buffer_append(u8)
define_buffer_append(u16)
define_buffer_append(u32)
define_buffer_append(u64)
#undef define_buffer_append

typedef void (*fn_void_to_void)();
typedef u32 (*fn_void_to_u32)();
typedef s32 (*fn_void_to_s32)();

struct String
{
    u8 *chars;
    u32 len;
};

String
create_string
(Buffer *buffer, char *str)
{
    
    String result = {};
    result.chars = buffer->end;
    
    u8 *index = (u8 *)str;
    while(*index)
    {
        buffer_append_u8(buffer, *index++);
        result.len++;
    }
    
    return(result);
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
win32ReadEntireFile
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

global u8 DOSHeaderData[] = {
    0x4D, 0x5A, 0x90, 0x00, 0x03, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0xB8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA8, 0x00, 0x00, 0x00, 0x0E, 0x1F, 0xBA, 0x0E, 0x00, 0xB4, 0x09, 0xCD, 0x21, 0xB8, 0x01, 0x4C, 0xCD, 0x21, 0x54, 0x68, 0x69, 0x73, 0x20, 0x70, 0x72, 0x6F, 0x67, 0x72, 0x61, 0x6D, 0x20, 0x63, 0x61, 0x6E, 0x6E, 0x6F, 0x74, 0x20, 0x62, 0x65, 0x20, 0x72, 0x75, 0x6E, 0x20, 0x69, 0x6E, 0x20, 0x44, 0x4F, 0x53, 0x20, 0x6D, 0x6F, 0x64, 0x65, 0x2E, 0x0D, 0x0D, 0x0A, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x61, 0x6A, 0x31, 0xDF, 0x25, 0x0B, 0x5F, 0x8C, 0x25, 0x0B, 0x5F, 0x8C, 0x25, 0x0B, 0x5F, 0x8C, 0xE4, 0x7E, 0x5A, 0x8D, 0x24, 0x0B, 0x5F, 0x8C, 0xE4, 0x7E, 0x5D, 0x8D, 0x24, 0x0B, 0x5F, 0x8C, 0x52, 0x69, 0x63, 0x68, 0x25, 0x0B, 0x5F, 0x8C, 0x50, 0x45, 0x00, 0x00
};

/*

    Buffer executable = create_buffer(PAGE, PAGE_READWRITE);
    u32 BytesToWrite = ArrayCount(DOSHeaderData);
    for(u32 i = 0; i < BytesToWrite; i++)
    {
        buffer_append_u8(&executable, DOSHeaderData[i]);
    }
    
    coff_header *COFFHeader = (coff_header *)buffer_allocate(&executable, sizeof(coff_header));
    COFFHeader->machine = 0x8664;
    COFFHeader->numberOfSections = 1;
    
    FILETIME filetime = {};
    GetSystemTimePreciseAsFileTime(&filetime);
    COFFHeader->timeDateStamp = filetime.dwLowDateTime;
    COFFHeader->sizeOfOptionalHeader = sizeof(pe_opt_header) + sizeof(coff_extension) + (sizeof(data_directory) * 16);
    COFFHeader->characteristics = IMAGE_FILE_EXECUTABLE_IMAGE | IMAGE_FILE_LARGE_ADDRESS_AWARE;
    
    pe_opt_header *PEOptHeader = (pe_opt_header *)buffer_allocate(&executable, sizeof(pe_opt_header));
    PEOptHeader->PESignature = 0x20b;
    PEOptHeader->SizeOfCode = 0x200;
    PEOptHeader->AddressOfEntryPoint = PAGE;
    PEOptHeader->BaseOfCode = PAGE;
    
    coff_extension *COFFExtension = (coff_extension *)buffer_allocate(&executable, sizeof(coff_extension));
    COFFExtension->ImageBase = 0x400000;
    COFFExtension->SectionAlignment = PAGE;
    COFFExtension->FileAlignment = 0x200;
    COFFExtension->MajorOSVersion = 6;
    COFFExtension->MajorSubsystemVersion = 6;
    COFFExtension->SizeOfImage = /*The header gets its own page.(1 + COFFHeader->numberOfSections) * COFFExtension->SectionAlignment;
COFFExtension->SizeOfHeaders = COFFExtension->FileAlignment;
COFFExtension->Subsystem = IMAGE_SUBSYSTEM_WINDOWS_GUI;
COFFExtension->DLLCharacteristics = IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE | IMAGE_DLLCHARACTERISTICS_TERMINAL_SERVER_AWARE | IMAGE_DLLCHARACTERISTICS_NX_COMPAT | IMAGE_DLLCHARACTERISTICS_HIGH_ENTROPY_VA;
COFFExtension->SizeOfStackReserve = 0x100000;
COFFExtension->SizeOfStackCommit = 0x100000;
COFFExtension->SizeOfHeapReserve = PAGE;
COFFExtension->SizeOfHeapCommit = PAGE;
COFFExtension->NumberOfRvaAndSizes = 16;
data_directory *DataDirectory = (data_directory *)buffer_allocate(&executable, sizeof(data_directory)*16);
(void)DataDirectory;

image_section_header *SectionHeader = (image_section_header *)buffer_allocate(&executable, sizeof(image_section_header));
SectionHeader->Name[0] = '.';
SectionHeader->Name[1] = 't';
SectionHeader->Name[2] = 'e';
SectionHeader->Name[3] = 'x';
SectionHeader->Name[4] = 't';
SectionHeader->VirtualAddress = PAGE;
SectionHeader->SizeOfRawData = COFFExtension->FileAlignment;
SectionHeader->PointerToRawData = COFFExtension->FileAlignment;
SectionHeader->Characteristics = IMAGE_SCN_CNT_CODE | IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_MEM_READ;

void *codeStart = executable.end = executable.memory + 0x200;

read_file_result sourceCode = win32ReadEntireFile("..\\data\\source.txt");
u8 *ch = (u8 *)sourceCode.Contents;
u8 *chMax = (u8 *)sourceCode.Contents + sourceCode.ContentsSize;
Expression(&executable, ch, chMax);

// ret
buffer_append_u8(&executable, 0xc3);

SectionHeader->Misc.VirtualSize = (u32)((u8 *)executable.end - (u8 *)codeStart);

u32 Size = (/*for the header1 + COFFHeader->numberOfSections) * COFFExtension->FileAlignment;
win32WriteEntireFile("..\\data\\MyProgram.exe", Size, executable.memory);
*/

#endif //WIN32_COMPILER_H

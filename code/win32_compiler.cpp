#include <windows.h>

#include "win32_compiler.h"

#pragma pack(push, 1)
struct data_directory
{ 
    u32 VirtualAddress;
    u32 Size;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct dos_header
{
    char signature[2];
    u16 lastsize;
    u16 nblocks;
    u16 nreloc;
    u16 hdrsize;
    u16 minalloc;
    u16 maxalloc;
    u16 ss;
    u16 sp;
    u16 checksum;
    u16 ip;
    u16 cs;
    u16 relocpos;
    u16 noverlay;
    u16 reserved1[4];
    u16 oem_id;
    u16 oem_info;
    u16 reserved2[10];
    u32 e_lfanew;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct coff_header
{
    u16 machine;
    u16 numberOfSections;
    u32 timeDateStamp;
    u32 pointerToSymbolTable;
    u32 numberOfSymbols;
    u16 sizeOfOptionalHeader;
    u16 characteristics;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct pe_opt_header
{
    u16 PESignature;
    u8 MajorLinkerVersion; 
    u8 MinorLinkerVersion;
    u32 SizeOfCode;
    u32 SizeOfInitializedData;
    u32 SizeOfUninitializedData;
    u32 AddressOfEntryPoint;
    u32 BaseOfCode;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct coff_extension
{
    u64 ImageBase;
    u32 SectionAlignment;
    u32 FileAlignment;
    u16 MajorOSVersion;
    u16 MinorOSVersion;
    u16 MajorImageVersion;
    u16 MinorImageVersion;
    u16 MajorSubsystemVersion;
    u16 MinorSubsystemVersion;
    u32 Win32VersionValue;
    u32 SizeOfImage;
    u32 SizeOfHeaders;
    u32 Checksum;
    u16 Subsystem;
    u16 DLLCharacteristics;
    u64 SizeOfStackReserve;
    u64 SizeOfStackCommit;
    u64 SizeOfHeapReserve;
    u64 SizeOfHeapCommit;
    u32 LoaderFlags;
    u32 NumberOfRvaAndSizes;
};
#pragma pack(pop)

#pragma pack(push, 1)
struct image_section_header
{
    u8  Name[8];
    union {
        u32 PhysicalAddress;
        u32 VirtualSize;
    } Misc;
    u32 VirtualAddress;
    u32 SizeOfRawData;
    u32 PointerToRawData;
    u32 PointerToRelocations;
    u32 PointerToLinenumbers;
    u16 NumberOfRelocations;
    u16 NumberOfLinenumbers;
    u32 Characteristics;
};
#pragma pack(pop)

struct win32_file_header
{
    dos_header *DOSHeader;
    coff_header *COFFHeader;
    pe_opt_header *PEOptHeader;
    
    b32 use32bit;
    u32 BaseOfData; // If the file is 32 bit, then this gets used and takes up space.
    
    coff_extension *COFFExtension;
    data_directory *dataDirectory;
    image_section_header *SectionHeader;
};

int __stdcall
WinMainCRTStartup
(void)
{
    
    SYSTEM_INFO SysInfo = {};
    GetSystemInfo(&SysInfo);
    Assert(SysInfo.dwPageSize != 0);
    u32 PAGE = SysInfo.dwPageSize;
    
    Buffer buffer_strings = create_buffer(PAGE, PAGE_READWRITE);
    
    /*
    read_file_result Test_EXE = Win32ReadEntireFile("..\\misc\\blank.exe");
    
    win32_file_header win32Header = {};
    win32Header.DOSHeader = (dos_header *)Test_EXE.Contents;
    
        Buffer buffer_DOSData = create_buffer(PAGE, PAGE_READWRITE);
        String intro = create_string(&buffer_DOSData, "global u8 DOSHeaderData[] = {\r\n");
        
        for(u8 *data = (u8 *)Test_EXE.Contents; data < ((u8 *)Test_EXE.Contents + win32Header.DOSHeader->e_lfanew + 4); data++)
        {
            
            buffer_append_u8(&buffer_DOSData, '0');
            buffer_append_u8(&buffer_DOSData, 'x');
            
            u8 high = (u8)(((*data & 0xf0) >> 4) & 0x0f);
            if(high <= 9)
            {
                high += '0';
            }
            else
            {
                high = (u8)(high - 10 + 'A');
            }
            
            u8 low = (u8)(*data & 0x0f);
            if(low <= 9)
            {
                low += '0';
            }
            else
            {
                low = (u8)(low - 10 + 'A');
            }
            
            buffer_append_u8(&buffer_DOSData, high);
            buffer_append_u8(&buffer_DOSData, low);
            buffer_append_u8(&buffer_DOSData, ',');
            buffer_append_u8(&buffer_DOSData, ' ');
        }
        
        buffer_DOSData.end -= 2;
        buffer_DOSData.size -= 2;
        buffer_append_u8(&buffer_DOSData, '\r');
        buffer_append_u8(&buffer_DOSData, '\n');
        buffer_append_u8(&buffer_DOSData, '}');
        buffer_append_u8(&buffer_DOSData, ';');
        
        u32 Size = (u32)(buffer_DOSData.end - buffer_DOSData.memory);
        b32 written = win32WriteEntireFile("..\\misc\\temp.txt", Size, buffer_DOSData.memory);
        (void)written;
    
    win32Header.COFFHeader = (coff_header *)((u8 *)Test_EXE.Contents + win32Header.DOSHeader->e_lfanew + 4);
    win32Header.PEOptHeader = (pe_opt_header *)((u8 *)win32Header.COFFHeader + sizeof(coff_header));
    
    if(win32Header.PEOptHeader->PESignature == 0x010b)
    {
        win32Header.use32bit = 1;
        win32Header.BaseOfData = *(u32 *)((u8 *)win32Header.PEOptHeader + sizeof(pe_opt_header));
    }
    
    win32Header.COFFExtension = (coff_extension *)((u8 *)win32Header.PEOptHeader + sizeof(pe_opt_header) + (win32Header.use32bit * 4));
    
    win32Header.dataDirectory = (data_directory *)((u8 *)win32Header.COFFExtension + sizeof(coff_extension));
    
    u32 TEXT = 0;
    image_section_header *Section = win32Header.SectionHeader = (image_section_header *)((u8 *)win32Header.PEOptHeader + win32Header.COFFHeader->sizeOfOptionalHeader);
    for(u32 i = 0; i < win32Header.COFFHeader->numberOfSections; i++)
    {
        u8 *Name = Section->Name;
        if((Name[0] == '.') &&
           (Name[1] == 't') &&
           (Name[2] == 'e') &&
           (Name[3] == 'x') &&
           (Name[4] == 't'))
        {
            TEXT = i;
        }
        
        Section++;
    }
    
    u8 *TextStart = win32Header.SectionHeader[TEXT].PointerToRawData + (u8 *)Test_EXE.Contents;
    
	u8 *MainFunction = win32Header.PEOptHeader->AddressOfEntryPoint - win32Header.PEOptHeader->BaseOfCode + TextStart;
    (void)MainFunction;
    */
    
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
    COFFExtension->SizeOfImage = /* The header gets its own page.*/(1 + COFFHeader->numberOfSections) * COFFExtension->SectionAlignment;
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
    
    executable.end = executable.memory + 0x200;
    u32 code_bytes = 0;
    buffer_append_u8(&executable, 0xb8);
    code_bytes++;
    buffer_append_u8(&executable, 0x01);
    code_bytes++;
    buffer_append_u8(&executable, 0x00);
    code_bytes++;
    buffer_append_u8(&executable, 0x00);
    code_bytes++;
    buffer_append_u8(&executable, 0x00);
    code_bytes++;
    buffer_append_u8(&executable, 0xc3);
    code_bytes++;
    
    SectionHeader->Misc.VirtualSize = code_bytes;
    
    u32 Size = (/*for the header*/1 + COFFHeader->numberOfSections) * COFFExtension->FileAlignment;
    win32WriteEntireFile("..\\misc\\MyProgram.exe", Size, executable.memory);
    
    return(0);
}
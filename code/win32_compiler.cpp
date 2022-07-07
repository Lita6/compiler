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
    (void)PAGE;
    
    read_file_result Test_EXE = Win32ReadEntireFile("..\\misc\\blank.exe");
    
    win32_file_header win32Header = {};
    win32Header.DOSHeader = (dos_header *)Test_EXE.Contents;
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
    
	// NOTE: AddressOfEntryPoint is relative to the image base when this file is loaded into memory. VirtualAddress is also relative to the image base when it's loaded into memory, so the difference will, hopefully, point straight to Main.
	u8 *MainFunction = win32Header.PEOptHeader->AddressOfEntryPoint - win32Header.SectionHeader[TEXT].VirtualAddress + TextStart;
    (void)MainFunction;
    
    return(0);
}
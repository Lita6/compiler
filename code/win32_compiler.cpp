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

b32
isAlpha
(u8 ch)
{
    b32 Result = 0;
    if(((ch >= 'A') && (ch <= 'Z')) ||
       ((ch >= 'a') && (ch <= 'z')))
    {
        Result = 1;
    }
    
    return(Result);
}

b32
isDigit
(u8 ch)
{
    b32 Result = 0;
    if((ch >= '0') && (ch <= '9'))
    {
        Result = 1;
    }
    
    return(Result);
}

u8 *
GetChar
(u8 *ch, u8 *chMax)
{
    
    ch++;
    if(ch >= chMax)
    {
        ch = 0;
    }
    return(ch);
}

void
Term
(Buffer *buffer, u8 *ch)
{
    u32 num = 0;
    if(isDigit(*ch))
    {
        num = (u32)((s32)(*ch) - '0');
        
        // mov eax, imm32
        buffer_append_u8(buffer, 0xb8);
        buffer_append_u32(buffer, num);
    }
    else
    {
        Assert(!"Only a single digit number accepted for now.");
    }
}

void
Add
(Buffer *buffer, u8 *ch)
{
    Term(buffer, ch);
    
    // add eax, ecx
    buffer_append_u8(buffer, 0x01);
    buffer_append_u8(buffer, 0xc8);
}

void
Sub
(Buffer *buffer, u8 *ch)
{
    Term(buffer, ch);
    
    // sub eax, ecx
    buffer_append_u8(buffer, 0x29);
    buffer_append_u8(buffer, 0xc8);
    
    // neg eax
    buffer_append_u8(buffer, 0xf7);
    buffer_append_u8(buffer, 0xd8);
}

void
Expression
(Buffer *buffer, u8 *ch, u8 *chMax)
{
    Term(buffer, ch);
    ch = GetChar(ch, chMax);
    
    if(ch != 0)
    {    
        // mov ecx, eax
        buffer_append_u8(buffer, 0x89);
        buffer_append_u8(buffer, 0xc1);
        
        u8 c = *ch;
        ch = GetChar(ch, chMax);
        
        switch(c)
        {
            case '+':
            {
                Add(buffer, ch);
            }break;
            
            case '-':
            {
                Sub(buffer, ch);
            }break;
            
            default:
            {
                Assert(!"Unsupported operation.");
            };
        }
    }
}

int __stdcall
WinMainCRTStartup
(void)
{
    
    SYSTEM_INFO SysInfo = {};
    GetSystemInfo(&SysInfo);
    Assert(SysInfo.dwPageSize != 0);
    u32 PAGE = SysInfo.dwPageSize;
    
    Buffer buffer_strings = create_buffer(PAGE, PAGE_READWRITE);
    Buffer buffer_functions = create_buffer(PAGE, PAGE_EXECUTE_READWRITE);
    
    {
        String src = create_string(&buffer_strings, "5");
        fn_void_to_s32 program = (fn_void_to_s32)buffer_functions.end;
        Expression(&buffer_functions, src.chars, (src.chars + src.len));
        buffer_append_u8(&buffer_functions, 0xc3); //ret
        s32 result = program();
        Assert(result == 5);
    }
    
    {
        String src = create_string(&buffer_strings, "4+7");
        fn_void_to_s32 program = (fn_void_to_s32)buffer_functions.end;
        Expression(&buffer_functions, src.chars, (src.chars + src.len));
        buffer_append_u8(&buffer_functions, 0xc3); //ret
        s32 result = program();
        Assert(result == 11);
    }
    
    {
        String src = create_string(&buffer_strings, "2-4");
        fn_void_to_s32 program = (fn_void_to_s32)buffer_functions.end;
        Expression(&buffer_functions, src.chars, (src.chars + src.len));
        buffer_append_u8(&buffer_functions, 0xc3); //ret
        s32 result = program();
        Assert(result == -2);
    }
    
    return(0);
}
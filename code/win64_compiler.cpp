#include <windows.h>

#include "win64_compiler.h"

int __stdcall
WinMainCRTStartup
(void)
{
    
    SYSTEM_INFO SysInfo = {};
    GetSystemInfo(&SysInfo);
    Assert(SysInfo.dwPageSize != 0);
    u32 PAGE = SysInfo.dwPageSize;
    
    Buffer buffer_strings = create_buffer(PAGE, PAGE_READWRITE);
    
    {
        String src = create_string(&buffer_strings, "int main() { return 2; }");
        clear_buffer(&buffer_strings);
    }
    
    return(0);
}
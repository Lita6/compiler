#include <windows.h>

#include "win64_compiler.h"

enum Token_Type
{
    Token_Type_None,
    Token_Type_Open_Brace,
    Token_Type_Close_Brace,
    Token_Type_Open_Parenthesis,
    Token_Type_Close_Parenthesis,
    Token_Type_Semicolon,
    Token_Type_S32_Keyword,
    Token_Type_Main_Keyword,
    Token_Type_Immediate,
};

struct Token
{
    Token_Type type;
    String string;
    s32 value;
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
    Buffer buffer_tokens = create_buffer(PAGE, PAGE_READWRITE);
    
    {
        String src = create_string(&buffer_strings, "s32 main() {\r\nreturn 2;\r\n}");
        
        String line[64] = {};
        u32 current_line = 0;
        line[current_line].chars = src.chars;
        for(u32 i = 0; i < src.len; i++)
        {
            if((src.chars[i] == '\r') || (src.chars[i] == '\n'))
            {
                while((src.chars[i] == '\r') || (src.chars[i] == '\n'))
                {
                    i++;
                }
                
                if(i >= src.len)
                {
                    break;
                }
                
                current_line++;
                line[current_line].chars = &src.chars[i];
            }
            
            line[current_line].len++;
        }
        
#if 0        
        String token  = {};
        token.chars = line.chars;
        for(u32 i = 0; i < line.len; i++)
        {
            if(line.chars[i] == ' ')
            {
                break;
            }
            
            token.len++;
        }
#endif
        
        clear_buffer(&buffer_strings);
    }
    
    return(0);
}
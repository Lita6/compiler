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
    Token_Type_Return,
    Token_Type_Immediate,
};

struct Token
{
    Token_Type type;
    String string;
    s32 value;
};

struct Token_List
{
    Token *start;
    u32 count;
};

void
inc_token
(Buffer *buffer, Token **token, Token_List *list, u8 *chars)
{
    
    *token = (Token *)buffer_allocate(buffer, sizeof(Token));
    list->count++;
    (*token)->string.chars = chars;
}

b32
isWhiteSpace
(u8 ch)
{
    b32 result = 0;
    if((ch == ' ') || (ch == '\t'))
    {
        result = 1;
    }
    return(result);
}

Token_List
lex
(Buffer *buffer, String line)
{
    Token_List result = {};
    result.start = (Token *)buffer->end;
    Token *current_token = 0;
    for(u32 i = 0; i < line.len; i++)
    {
        if(isWhiteSpace(line.chars[i]))
        {
            while(isWhiteSpace(line.chars[i]))
            {
                i++;
            }
            
            if(i >= line.len)
            {
                break;
            }
            
            current_token = 0;
        }
        
        if(line.chars[i] == '{')
        {
            inc_token(buffer, &current_token, &result, &line.chars[i]);
            current_token->type = Token_Type_Open_Brace;
        }
        else if(line.chars[i] == '}')
        {
            inc_token(buffer, &current_token, &result, &line.chars[i]);
            current_token->type = Token_Type_Close_Brace;
        }
        else if(line.chars[i] == '(')
        {
            inc_token(buffer, &current_token, &result, &line.chars[i]);
            current_token->type = Token_Type_Open_Parenthesis;
        }
        else if(line.chars[i] == ')')
        {
            inc_token(buffer, &current_token, &result, &line.chars[i]);
            current_token->type = Token_Type_Close_Parenthesis;
        }
        else if(line.chars[i] == ';')
        {
            inc_token(buffer, &current_token, &result, &line.chars[i]);
            current_token->type = Token_Type_Semicolon;
        }
        else if(current_token == 0)
        {
            inc_token(buffer, &current_token, &result, &line.chars[i]);
        }
        current_token->string.len++;
    }
    
    if(current_token != 0)
    {
        buffer->end -= (sizeof(Token));
        result.count--;
        current_token->string.chars = 0;
        current_token->string.len = 0;
        current_token->type = Token_Type_None;
        current_token = 0;
    }
    
    return(result);
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
    Buffer buffer_tokens = create_buffer(PAGE, PAGE_READWRITE);
    
    {
        String src = create_string(&buffer_strings, "s32 main() {\r\nreturn 2;\r\n}");
        
        String line[64] = {};
        u32 line_count = 0;
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
                
                if(line[line_count].chars != 0)
                {
                    line_count++;
                }
                line[line_count].chars = &src.chars[i];
            }
            
            if(line[line_count].chars == 0)
            {
                line[line_count].chars = &src.chars[i];
            }
            line[line_count].len++;
        }
        line_count++;
        
        for(u32 i = 0; i < line_count; i++)
        {
            Token_List tokens = lex(&buffer_tokens, line[i]);
            int a = 0;
            (void)a;
        }
        
        clear_buffer(&buffer_strings);
    }
    
    return(0);
}
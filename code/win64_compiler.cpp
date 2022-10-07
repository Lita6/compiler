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
    Token_Type_Return_Keyword,
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
    (*token)->string.len++;
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

Token_Type
isSpecialSymbol
(u8 ch)
{
    Token_Type result = Token_Type_None;
    
    switch (ch)
    {
        case '{':
        {
            result = Token_Type_Open_Brace;
        }break;
        
        case '}':
        {
            result = Token_Type_Close_Brace;
        }break;
        
        case '(':
        {
            result = Token_Type_Open_Parenthesis;
        }break;
        
        case ')':
        {
            result = Token_Type_Close_Parenthesis;
        }break;
        
        case ';':
        {
            result = Token_Type_Semicolon;
        }break;
        
        default:
        {
            break;
        };
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
            current_token = 0;
            continue;
        }
        
        Token_Type type = isSpecialSymbol(line.chars[i]);
        if(type != Token_Type_None)
        {
            inc_token(buffer, &current_token, &result, &line.chars[i]);
            current_token->type = type;
            current_token = 0;
        }
        else if(current_token == 0)
        {
            inc_token(buffer, &current_token, &result, &line.chars[i]);
        }
        else
        {
            Assert(current_token != 0);
            current_token->string.len++;
        }
    }
    
    for(u32 i = 0; i < result.count; i++)
    {
        current_token = &result.start[i];
        if(current_token->type == Token_Type_None)
        {
            // TODO: I actually need this to match all the letters.
            switch (current_token->string.chars[0])
            {
                case 's':
                {
                    current_token->type = Token_Type_S32_Keyword;
                }break;
                
                case 'm':
                {
                    current_token->type = Token_Type_Main_Keyword;
                }break;
                
                case 'r':
                {
                    current_token->type = Token_Type_Return_Keyword;
                }break;
                
                default:
                {
                    if(isDigit(current_token->string.chars[0]))
                    {
                        current_token->type = Token_Type_Immediate;
                        current_token->value = (s32)StringToS64(current_token->string);
                    }
                };
            };
        }
    }
    
    return(result);
}

struct Lines
{
    String *start;
    u32 count;
};

Lines
SplitLines
(Buffer *buffer, String string)
{
    Lines result = {};
    result.start = (String *)buffer->end;
    String *current_string = 0;
    for(u32 i = 0; i < string.len; i++)
    {
        if((string.chars[i] == '\r') || (string.chars[i] == '\n'))
        {
            current_string = 0;
            continue;
        }
        
        if(current_string == 0)
        {
            current_string = (String *)buffer_allocate(buffer, sizeof(String));
            result.count++;
            current_string->chars = &string.chars[i];
        }
        
        current_string->len++;
    }
    
    return(result);
}

void
compile
(Buffer *buffer_lines, Buffer *buffer_tokens, String src)
{
    
    Lines lines = SplitLines(buffer_lines, src);
    
    for(u32 i = 0; i < lines.count; i++)
    {
        Token_List tokens = lex(buffer_tokens, lines.start[i]);
        int a = 0;
        (void)a;
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
    Buffer buffer_tokens = create_buffer(PAGE, PAGE_READWRITE);
    Buffer buffer_lines = create_buffer(PAGE, PAGE_READWRITE);
    
    {
        String src = create_string(&buffer_strings, "s32 main() {\r\nreturn 2;\r\n}");
        compile(&buffer_lines, &buffer_tokens, src);
        
        clear_buffer(&buffer_strings);
        clear_buffer(&buffer_tokens);
    }
    
    {
        String src = create_string(&buffer_strings, "s32 main( {\r\nreturn 2;\r\n}");
        compile(&buffer_lines, &buffer_tokens, src);
        
        clear_buffer(&buffer_strings);
        clear_buffer(&buffer_tokens);
    }
    
    {
        String src = create_string(&buffer_strings, "s32 main() {\r\nreturn;\r\n}");
        compile(&buffer_lines, &buffer_tokens, src);
        
        clear_buffer(&buffer_strings);
        clear_buffer(&buffer_tokens);
    }
    
    {
        String src = create_string(&buffer_strings, "s32 main() {\r\nreturn 2;");
        compile(&buffer_lines, &buffer_tokens, src);
        
        clear_buffer(&buffer_strings);
        clear_buffer(&buffer_tokens);
    }
    
    {
        String src = create_string(&buffer_strings, "s32 main() {\r\nreturn 2\r\n}");
        compile(&buffer_lines, &buffer_tokens, src);
        
        clear_buffer(&buffer_strings);
        clear_buffer(&buffer_tokens);
    }
    
    {
        String src = create_string(&buffer_strings, "s32 main() {\r\nreturn2;\r\n}");
        compile(&buffer_lines, &buffer_tokens, src);
        
        clear_buffer(&buffer_strings);
        clear_buffer(&buffer_tokens);
    }
    
    {
        String src = create_string(&buffer_strings, "s32 main() {\r\nRETURN 2;\r\n}");
        compile(&buffer_lines, &buffer_tokens, src);
        
        clear_buffer(&buffer_strings);
        clear_buffer(&buffer_tokens);
    }
    
    {
        String src = create_string(&buffer_strings, "s32 main() {\r\nreturn 100;\r\n}");
        compile(&buffer_lines, &buffer_tokens, src);
        
        clear_buffer(&buffer_strings);
        clear_buffer(&buffer_tokens);
    }
    
    {
        String src = create_string(&buffer_strings, "\r\ns32\r\nmain\r\n(\r\n)\r\n{\r\nreturn\r\n2\r\n;\r\n}");
        compile(&buffer_lines, &buffer_tokens, src);
        
        clear_buffer(&buffer_strings);
        clear_buffer(&buffer_tokens);
    }
    
    {
        String src = create_string(&buffer_strings, "s32 main(){return 2;}");
        compile(&buffer_lines, &buffer_tokens, src);
        
        clear_buffer(&buffer_strings);
        clear_buffer(&buffer_tokens);
    }
    
    {
        String src = create_string(&buffer_strings, "s32 main() {\r\nreturn 0;\r\n}");
        compile(&buffer_lines, &buffer_tokens, src);
        
        clear_buffer(&buffer_strings);
        clear_buffer(&buffer_tokens);
    }
    
    {
        String src = create_string(&buffer_strings, "   s32   main   (   )   {   \r\nreturn   2   ;\r\n   }");
        compile(&buffer_lines, &buffer_tokens, src);
        
        clear_buffer(&buffer_strings);
        clear_buffer(&buffer_tokens);
    }
    
    return(0);
}
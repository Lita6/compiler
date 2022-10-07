// TODO: So I've decided that I don't want to do the lexer this way or the parser this way
//       for now. I've decided this is going to be really close to a C compiler, but not
//       quite. What I need from the testing part is output from the compiler saying whether
//       it was a successful compiler or not. I don't think I need to worry too much about
//       error codes just yet, though I could, if I changed my mind. I also need separate
//       output from running the compiled code itself and test that against what was expected.

// TODO: The simplest, but complete programs that my compiler will accept are:
//       "s32 main(){return 0;}"
//       "s32 main(){return 0}"
//       "return"
//       "return;"
//       "return 0"
//       "return 0;"
//       "move rax, 0\r\n ret"
//       "ret"

#include <windows.h>

#include "win64_compiler.h"

struct String_List
{
    String *start;
    u32 count;
};

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
(Buffer *tokens, Token_List keywords, String line)
{
    Token_List result = {};
    result.start = (Token *)tokens->end;
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
            inc_token(tokens, &current_token, &result, &line.chars[i]);
            current_token->type = type;
            current_token = 0;
        }
        else if(current_token == 0)
        {
            inc_token(tokens, &current_token, &result, &line.chars[i]);
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
            b32 isNum = 1;
            for(u32 n = 0; n < current_token->string.len; n++)
            {
                if(!(isDigit(current_token->string.chars[n])))
                {
                    isNum = 0;
                    break;
                }
            }
            
            if(isNum == 1)
            {
                current_token->type = Token_Type_Immediate;
                current_token->value = (s32)StringToS64(current_token->string);
            }
            else
            {            
                for(u32 n = 0; n < keywords.count; n++)
                {
                    Token *search = &keywords.start[n];
                    if(current_token->string.len == search->string.len)
                    {
                        b32 match = 1;
                        for(u32 d = 0; d < current_token->string.len; d++)
                        {
                            if(current_token->string.chars[d] != search->string.chars[d])
                            {
                                match = 0;
                                break;
                            }
                        }
                        
                        if(match == 1)
                        {
                            current_token->type = search->type;
                            break;
                        }
                    }
                }
            }
        }
    }
    
    return(result);
}

String_List
SplitString_List
(Buffer *buffer, String string)
{
    String_List result = {};
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
(Buffer *buffer_lines, Buffer *buffer_tokens, Token_List keywords, String src)
{
    
    String_List lines = SplitString_List(buffer_lines, src);
    
    for(u32 i = 0; i < lines.count; i++)
    {
        Token_List tokens = lex(buffer_tokens, keywords, lines.start[i]);
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
    Buffer buffer_temp = create_buffer(PAGE, PAGE_READWRITE);
    
    Token_List keywords = {};
    Buffer buffer_keywords = create_buffer(PAGE, PAGE_READWRITE);
    keywords.start = (Token *)buffer_keywords.end;
    keywords.start[keywords.count].string = create_string(&buffer_strings, "s32");
    keywords.start[keywords.count++].type = Token_Type_S32_Keyword;
    keywords.start[keywords.count].string = create_string(&buffer_strings, "main");
    keywords.start[keywords.count++].type = Token_Type_Main_Keyword;
    keywords.start[keywords.count].string = create_string(&buffer_strings, "return");
    keywords.start[keywords.count++].type = Token_Type_Return_Keyword;
    
    {
        String src = create_string(&buffer_temp, "s32 main() {\r\nreturn 2;\r\n}");
        compile(&buffer_lines, &buffer_tokens, keywords, src);
        
        clear_buffer(&buffer_temp);
        clear_buffer(&buffer_tokens);
    }
    
    {
        String src = create_string(&buffer_temp, "s32 main( {\r\nreturn 2;\r\n}");
        compile(&buffer_lines, &buffer_tokens, keywords, src);
        
        clear_buffer(&buffer_temp);
        clear_buffer(&buffer_tokens);
    }
    
    {
        String src = create_string(&buffer_temp, "s32 main() {\r\nreturn;\r\n}");
        compile(&buffer_lines, &buffer_tokens, keywords, src);
        
        clear_buffer(&buffer_temp);
        clear_buffer(&buffer_tokens);
    }
    
    {
        String src = create_string(&buffer_temp, "s32 main() {\r\nreturn 2;");
        compile(&buffer_lines, &buffer_tokens, keywords, src);
        
        clear_buffer(&buffer_temp);
        clear_buffer(&buffer_tokens);
    }
    
    {
        String src = create_string(&buffer_temp, "s32 main() {\r\nreturn 2\r\n}");
        compile(&buffer_lines, &buffer_tokens, keywords, src);
        
        clear_buffer(&buffer_temp);
        clear_buffer(&buffer_tokens);
    }
    
    {
        String src = create_string(&buffer_temp, "s32 main() {\r\nreturn2;\r\n}");
        compile(&buffer_lines, &buffer_tokens, keywords, src);
        
        clear_buffer(&buffer_temp);
        clear_buffer(&buffer_tokens);
    }
    
    {
        String src = create_string(&buffer_temp, "s32 main() {\r\nRETURN 2;\r\n}");
        compile(&buffer_lines, &buffer_tokens, keywords, src);
        
        clear_buffer(&buffer_temp);
        clear_buffer(&buffer_tokens);
    }
    
    {
        String src = create_string(&buffer_temp, "s32 main() {\r\nreturn 100;\r\n}");
        compile(&buffer_lines, &buffer_tokens, keywords, src);
        
        clear_buffer(&buffer_temp);
        clear_buffer(&buffer_tokens);
    }
    
    {
        String src = create_string(&buffer_temp, "\r\ns32\r\nmain\r\n(\r\n)\r\n{\r\nreturn\r\n2\r\n;\r\n}");
        compile(&buffer_lines, &buffer_tokens, keywords, src);
        
        clear_buffer(&buffer_temp);
        clear_buffer(&buffer_tokens);
    }
    
    {
        String src = create_string(&buffer_temp, "s32 main(){return 2;}");
        compile(&buffer_lines, &buffer_tokens, keywords, src);
        
        clear_buffer(&buffer_temp);
        clear_buffer(&buffer_tokens);
    }
    
    {
        String src = create_string(&buffer_temp, "s32 main() {\r\nreturn 0;\r\n}");
        compile(&buffer_lines, &buffer_tokens, keywords, src);
        
        clear_buffer(&buffer_temp);
        clear_buffer(&buffer_tokens);
    }
    
    {
        String src = create_string(&buffer_temp, "   s32   main   (   )   {   \r\nreturn   2   ;\r\n   }");
        compile(&buffer_lines, &buffer_tokens, keywords, src);
        
        clear_buffer(&buffer_temp);
        clear_buffer(&buffer_tokens);
    }
    
    return(0);
}
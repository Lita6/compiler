#include <windows.h>

#include "win64_compiler.h"

struct Variable
{
    String name;
    s64 value;
};

struct Program_Info
{
    u8 *ch;
    u8 *chMax;
    Buffer *var_table;
    u32 varCount;
};

void
GetChar
(Program_Info *info)
{
    do
    {    
        if(info->ch != 0)
        {    
            info->ch++;
            if(info->ch >= info->chMax)
            {
                info->ch = 0;
                break;
            }
        }
        else
        {
            break;
        }
    }while((*info->ch == ' ') || (*info->ch == '\t'));
}

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

b32
isAlphaNum
(u8 ch)
{
    b32 result = 0;
    if((isDigit(ch) == 1) || (isAlpha(ch) == 1))
    {
        result = 1;
    }
    return(result);
}

s64
GetNum
(String name)
{
    
    s64 result = 0;
    for(u32 i = 0; i < name.len; i++)
    {
        result = (10 * result) + (name.chars[i] - '0');
    }
    return(result);
}

String
GetName
(Program_Info *info)
{
    
    String result = {};
    result.chars = info->ch;
    while((info->ch != 0) && (isAlphaNum(*info->ch) == 1))
    {
        result.len++;
        GetChar(info);
    }
    
    return(result);
}


Variable *
FindVariable
(Program_Info *info, String toMatch)
{
    
    Variable *result = 0;
    if(info->varCount > 0)
    {
        Variable *end = (Variable *)(info->var_table->end);
        for(Variable *search = (Variable *)info->var_table->memory; search < end; search++)
        {
            if(search->name.len == toMatch.len)
            {
                u8 match = 1;
                for(u32 i = 0; i < toMatch.len; i++)
                {
                    if(search->name.chars[i] != toMatch.chars[i])
                    {
                        match = 0;
                        break;
                    }
                }
                
                if(match == 1)
                {
                    result = search;
                    break;
                }
            }
        }
    }
    
    if(result == 0)
    {
        result = (Variable *)buffer_allocate(info->var_table, sizeof(Variable));
        result->name = toMatch;
        info->varCount++;
    }
    
    return(result);
}

s64 Expression(Program_Info *info);

s64
Factor
(Program_Info *info)
{
    s64 result = 0;
    if((info->ch != 0) && (*info->ch == '('))
    {
        GetChar(info);
        result = Expression(info);
        if((info->ch != 0) && (*info->ch == ')'))
        {
            GetChar(info);
        }
        else
        {
            Assert(!"Closing parenthesis expected.");
        }
        
    }
    else if(info->ch != 0)
    {
        String name = GetName(info);
        
        b32 isNum = 1;
        for(u32 i = 0; i < name.len; i++)
        {
            if(isDigit(name.chars[i]) == 0)
            {
                isNum = 0;
                break;
            }
        }
        
        if(isNum == 0)
        {
            Variable *var = FindVariable(info, name);
            result = var->value;
        }
        else
        {
            result = GetNum(name);
        }
    }
    
    return(result);
}

s64
Term
(Program_Info *info)
{
    s64 result = Factor(info);
    
    while((info->ch != 0) && ((*info->ch == '*') || (*info->ch == '/')))
    {
        if(*info->ch == '*')
        {
            GetChar(info);
            result *= Factor(info);
        }
        else if(*info->ch == '/')
        {
            GetChar(info);
            result /= Factor(info);
        }
    }
    
    return(result);
}

s64
Expression
(Program_Info *info)
{
    s64 result = 0;
    
    if((info->ch != 0) && (!((*info->ch == '+') || (*info->ch == '-'))))
    {
        result = Term(info);
    }
    
    while((info->ch != 0) && ((*info->ch == '+') || (*info->ch == '-')))
    {
        if(*info->ch == '+')
        {
            GetChar(info);
            result += Term(info);
        }
        else if(*info->ch == '-')
        {
            GetChar(info);
            result -= Term(info);
        }
    }
    
    return(result);
}

s64
Assignment
(Program_Info *info)
{
    
    s64 result = 0;
    
    Variable *var = 0;
    for(u8 *ch = info->ch; ch < info->chMax; ch++)
    {
        if(*ch == '=')
        {
            if(isAlpha(*info->ch) == 0)
            {
                Assert(!"Cannot assign value to an integer.");
            }
            
            String name = GetName(info);
            var = FindVariable(info, name);
            
            GetChar(info); // eat the '='
            
            break;
        }
    }
    
    result = Expression(info);
    
    if(var != 0)
    {
        var->value = result;
        return(var->value);
    }
    else
    {
        return(result);
    }
}

s64
Interpret
(Program_Info *info)
{
    s64 result = 0;
    do
    {
        result = Assignment(info);
        
        if((info->ch != 0) && ((*info->ch == '\r') || (*info->ch == '\n')))
        {
            if(*info->ch == '\r')
            {
                GetChar(info);
                
            }
            GetChar(info);
        }
        
    }while(info->ch != 0);
    
    return(result);
}

void
test
(Buffer *vars, String src, s64 expected)
{
    
    Program_Info info = {};
    info.ch = src.chars;
    info.chMax = (src.chars + src.len);
    info.var_table = vars;
    
    s64 result = Interpret(&info);
    Assert(result == expected);
    
    clear_buffer(vars);
    
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
    Buffer buffer_vars = create_buffer(PAGE, PAGE_READWRITE);
    
    {
        String src = create_string(&buffer_strings, "5");
        test(&buffer_vars, src, 5);
        clear_buffer(&buffer_strings);
    }
    
    {
        String src = create_string(&buffer_strings, "4+7");
        test(&buffer_vars, src, 11);
        clear_buffer(&buffer_strings);
    }
    
    {
        String src = create_string(&buffer_strings, "2-4");
        test(&buffer_vars, src, -2);
        clear_buffer(&buffer_strings);
    }
    
    {
        String src = create_string(&buffer_strings, "7+8-9");
        test(&buffer_vars, src, 6);
        clear_buffer(&buffer_strings);
    }
    
    {
        String src = create_string(&buffer_strings, "2+3*4");
        test(&buffer_vars, src, 14);
        clear_buffer(&buffer_strings);
    }
    
    {
        String src = create_string(&buffer_strings, "9 / 3");
        test(&buffer_vars, src, 3);
        clear_buffer(&buffer_strings);
    }
    
    {
        String src = create_string(&buffer_strings, "1+2*3*4+5/6-7-8-9");
        s64 expected = 1+2*3*4+5/6-7-8-9;
        test(&buffer_vars, src, expected);
        clear_buffer(&buffer_strings);
    }
    
    {
        String src = create_string(&buffer_strings, "500*500+999");
        s64 expected = 500*500+999;
        test(&buffer_vars, src, expected);
        clear_buffer(&buffer_strings);
    }
    
    {
        String src = create_string(&buffer_strings, "1+2*(3+4)-5*(6-7*(8*9))");
        s64 expected = 1+2*(3+4)-5*(6-7*(8*9));
        test(&buffer_vars, src, expected);
        clear_buffer(&buffer_strings);
    }
    
    {
        String src = create_string(&buffer_strings, "-8");
        test(&buffer_vars, src, -8);
        clear_buffer(&buffer_strings);
    }
    
    {
        String src = create_string(&buffer_strings, "a+b");
        test(&buffer_vars, src, 0);
        clear_buffer(&buffer_strings);
    }
    
    {
        String src = create_string(&buffer_strings, "a=1+2");
        test(&buffer_vars, src, 3);
        clear_buffer(&buffer_strings);
    }
    
    {
        String src = create_string(&buffer_strings, "a=7\r\nb=15\r\na*b");
        s64 expected = 7*15;
        test(&buffer_vars, src, expected);
        clear_buffer(&buffer_strings);
    }
    
    {
        String src = create_string(&buffer_strings, "alpha=105+2000");
        test(&buffer_vars, src, 2105);
        clear_buffer(&buffer_strings);
    }
    
    {
        String src = create_string(&buffer_strings, "alpha = 50 + 99");
        test(&buffer_vars, src, 149);
        clear_buffer(&buffer_strings);
    }
    
    return(0);
}
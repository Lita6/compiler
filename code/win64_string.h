/* date = October 8th 2022 11:55 pm */

#ifndef WIN64_STRING_H
#define WIN64_STRING_H

struct String
{
    u8 *chars;
    u32 len;
};

struct String_List
{
    String *start;
    u32 count;
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
StringToS64
(String name)
{
    
    s64 result = 0;
    for(u32 i = 0; i < name.len; i++)
    {
        result = (10 * result) + (name.chars[i] - '0');
    }
    return(result);
}

b32
isMatch
(String a, String b)
{
    b32 result = 0;
    if(a.len == b.len)
    {
        result = 1;
        for(u32 i = 0; i < a.len; i++)
        {
            if(a.chars[i] != b.chars[i])
            {
                result = 0;
                break;
            }
        }
    }
    
    return(result);
}

#endif //WIN64_STRING_H

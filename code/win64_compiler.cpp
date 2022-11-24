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
    Token_Type_Comma,
    Token_Type_Minus,
    Token_Type_Immediate,
    Token_Type_Identifier,
    Token_Type_S32_Keyword,
    Token_Type_Main_Keyword,
    Token_Type_Return_Keyword,
    Token_Type_Mov_Keyword,
    Token_Type_RAX_Keyword,
    Token_Type_Ret_Keyword,
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

enum Return_Type
{
    Return_Type_Void,
    Return_Type_S32,
};

struct Function
{
    Return_Type ret;
    Token id;
    u8 *call_function;
};

struct Instruction
{
    Token instruction;
    Token operand[2];
    b32 isComplete;
};

enum Error
{
    Error_None,
    Error_Return_Value_Required,
    Error_Need_Return_Statement,
    Warning_Ret_Used_Instead_Of_Return,
    Error_Need_Open_Brace,
    Error_Need_Close_Brace,
    Error_Need_Open_Parenthesis,
    Error_Need_Close_Parenthesis,
    
};

void
fill_instruction_operands
(Instruction *instr, String string, Token_Type type, s32 value)
{
    if(instr->operand[0].type == Token_Type_None)
    {
        
        instr->operand[0].string = string;
        instr->operand[0].type = type;
        instr->operand[0].value = value;
    }
    else if(instr->operand[1].type == Token_Type_None)
    {
        
        instr->operand[1].string = string;
        instr->operand[1].type = type;
        instr->operand[1].value = value;
        instr->isComplete = 1;
    }
    else
    {
        Assert(!"Too many operands.");
    }
}

b32
isEndOfStatement
(u8 ch)
{
    b32 result = 0;
    
    if((ch == ';') || (ch == '\r') || (ch == '\n'))
    {
        result = 1;
    }
    
    return(result);
}

b32
isWhiteSpace
(u8 ch)
{
    b32 result = 0;
    if((ch == ' ') || (ch == '\t') || (ch == '\r') || (ch == '\n'))
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
        
        case ',':
        {
            result = Token_Type_Comma;
        }break;
        
        case '-':
        {
            result = Token_Type_Minus;
        }break;
        
        default:
        {
            break;
        };
    }
    
    return(result);
}

Token_Type
search_keyword
(Token_List keywords, String str)
{
    Token_Type result = Token_Type_None;
    for(u32 i = 0; i < keywords.count; i++)
    {
        if(isMatch(keywords.start[i].string, str))
        {
            result = keywords.start[i].type;
            break;
        }
    }
    
    return(result);
}

Error
compile
(Buffer *buffer_code, Token_List keywords, Function *function, String src)
{
    
    Error result = Error_None;
    
    String current_string = {};
    b32 must_find_return_value = 0;
    b32 need_return_statement = 0;
    s32 open_brace = 0;
    s32 open_parenthesis = 0;
    Instruction instr = {};
    for(u32 i = 0; i < src.len; i++)
    {
        Token_Type symbol_type = isSpecialSymbol(src.chars[i]);
        Token_Type string_type = Token_Type_None;
        b32 isDelimiter = ((symbol_type != Token_Type_None) || isWhiteSpace(src.chars[i]));
        
        switch (symbol_type)
        {
            
            case Token_Type_Open_Parenthesis:
            {
                open_parenthesis++;
            }break;
            
            case Token_Type_Close_Parenthesis:
            {
                open_parenthesis--;
                
                if(open_parenthesis < 0)
                {
                    result = Error_Need_Open_Parenthesis;
                }
            }break;
            
            case Token_Type_Open_Brace:
            {
                open_brace++;
                
                if(open_parenthesis > 0)
                {
                    result = Error_Need_Close_Parenthesis;
                }
            }break;
            
            case Token_Type_Close_Brace:
            {
                if(need_return_statement == 1)
                {
                    result = Error_Need_Return_Statement;
                }
                
                open_brace--;
                if(open_brace < 0)
                {
                    result = Error_Need_Open_Brace;
                }
            }break;
            
            case Token_Type_Minus:
            {
                
            }break;
        };
        
        if((current_string.chars == 0) && (!isDelimiter))
        {
            current_string.chars = &src.chars[i];
        }
        
        if((current_string.chars != 0) && (!isDelimiter))
        {
            current_string.len++;
        }
        
        b32 isLastChar = (((i + 1) >= src.len) && current_string.chars != 0);
        
        if(isDelimiter || isLastChar)
        {
            
            if(current_string.chars != 0)
            {
                string_type = search_keyword(keywords, current_string);
                
                if(string_type == Token_Type_None)
                {
                    
                    b32 isNum = 1;
                    for(u32 n = 0; n < current_string.len; n++)
                    {
                        if(!(isDigit(current_string.chars[n])))
                        {
                            isNum = 0;
                            break;
                        }
                    }
                    
                    if(isNum == 1)
                    {
                        string_type = Token_Type_Immediate;
                        
                        if((must_find_return_value == 1) || (instr.instruction.type != Token_Type_None))
                        {
                            s32 value = (s32)StringToS64(current_string);
                            must_find_return_value = 0;
                            
                            fill_instruction_operands(&instr, current_string, string_type, value);
                            instr.isComplete = 1;
                        }
                    }
                    else
                    {
                        string_type = Token_Type_Identifier;
                    }
                }
                
                switch (string_type)
                {
                    case Token_Type_S32_Keyword:
                    {
                        function->ret = Return_Type_S32;
                        need_return_statement = 1;
                    }break;
                    
                    case Token_Type_Main_Keyword:
                    {
                        function->id.type = string_type;
                        function->id.string = current_string;
                    }break;
                    
                    case Token_Type_Mov_Keyword:
                    {
                        instr.instruction.string = current_string;
                        instr.instruction.type = string_type;
                    }break;
                    
                    case Token_Type_RAX_Keyword:
                    {
                        fill_instruction_operands(&instr, current_string, string_type, 0);
                    }break;
                    
                    case Token_Type_Return_Keyword:
                    {
                        instr.instruction.string = current_string;
                        instr.instruction.type = string_type;
                        must_find_return_value = 1;
                        need_return_statement = 0;
                    }break;
                    
                    case Token_Type_Ret_Keyword:
                    {
                        instr.instruction.string = current_string;
                        instr.instruction.type = string_type;
                        instr.isComplete = 1;
                        need_return_statement = 0;
                        
                        if(function->ret != Return_Type_Void)
                        {
                            result = Warning_Ret_Used_Instead_Of_Return;
                        }
                    }break;
                    
                    default:
                    {
                        break;
                    };
                }
                
                current_string.chars = 0;
                current_string.len = 0;
            }
            
            if(instr.isComplete == 1)
            {
                if(function->call_function == 0)
                {
                    function->call_function = buffer_code->end;
                }
                
                switch (instr.instruction.type)
                {
                    case Token_Type_Ret_Keyword:
                    {
                        buffer_append_u8(buffer_code, 0xc3);
                    }break;
                    
                    case Token_Type_Mov_Keyword:
                    {
                        // For now this is only encoding an imm32 move into EAX
                        buffer_append_u8(buffer_code, 0xb8);
                        buffer_append_u32(buffer_code, (u32)instr.operand[1].value);
                    }break;
                    
                    case Token_Type_Return_Keyword:
                    {
                        buffer_append_u8(buffer_code, 0xb8);
                        buffer_append_u32(buffer_code, (u32)instr.operand[0].value);
                        buffer_append_u8(buffer_code, 0xc3);
                    }break;
                    
                    default:
                    {
                        break;
                    };
                };
                
                instr = {};
            }
            
            if((must_find_return_value == 1) && (isEndOfStatement(src.chars[i]) || isLastChar))
            {
                result = Error_Return_Value_Required;
            }
        }
        
        if(isLastChar && (open_brace > 0))
        {
            result = Error_Need_Close_Brace;
        }
        
        if(isLastChar && (open_parenthesis > 0))
        {
            result = Error_Need_Close_Parenthesis;
        }
        
        if((result != Error_None) && (result != Warning_Ret_Used_Instead_Of_Return))
        {
            if(function->call_function != 0)
            {
                Buffer temp = {};
                temp.memory = function->call_function;
                temp.end = buffer_code->end;
                temp.size = (u32)(buffer_code->end - function->call_function);
                clear_buffer(&temp);
            }
            *function = {};
            break;
        }
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
    Buffer buffer_lines = create_buffer(PAGE, PAGE_READWRITE);
    Buffer buffer_temp = create_buffer(PAGE, PAGE_READWRITE);
    Buffer buffer_code = create_buffer(PAGE, PAGE_EXECUTE_READWRITE);
    Buffer buffer_utility = create_buffer(PAGE, PAGE_EXECUTE_READWRITE);
    
    fn_void_to_void clear_rax = (fn_void_to_void)buffer_utility.end;
    buffer_append_u8(&buffer_utility, 0xb8);
    buffer_append_u32(&buffer_utility, 0);
    buffer_append_u8(&buffer_utility, 0xc3);
    
    Token_List keywords = {};
    Buffer buffer_keywords = create_buffer(PAGE, PAGE_READWRITE);
    keywords.start = (Token *)buffer_keywords.end;
    keywords.start[keywords.count].string = create_string(&buffer_strings, "s32");
    keywords.start[keywords.count++].type = Token_Type_S32_Keyword;
    keywords.start[keywords.count].string = create_string(&buffer_strings, "main");
    keywords.start[keywords.count++].type = Token_Type_Main_Keyword;
    keywords.start[keywords.count].string = create_string(&buffer_strings, "return");
    keywords.start[keywords.count++].type = Token_Type_Return_Keyword;
    keywords.start[keywords.count].string = create_string(&buffer_strings, "ret");
    keywords.start[keywords.count++].type = Token_Type_Ret_Keyword;
    keywords.start[keywords.count].string = create_string(&buffer_strings, "mov");
    keywords.start[keywords.count++].type = Token_Type_Mov_Keyword;
    keywords.start[keywords.count].string = create_string(&buffer_strings, "rax");
    keywords.start[keywords.count++].type = Token_Type_RAX_Keyword;
    
    /*
*
*  VALID
*
*/
    
    {
        String src = create_string(&buffer_temp, "ret");
        Function function = {};
        Error compile_result = compile(&buffer_code, keywords, &function, src);
        
        Assert(compile_result == Error_None);
        clear_rax();
        s32 function_result = ((fn_void_to_s32)function.call_function)();
        Assert(function_result == 0);
        
        clear_buffer(&buffer_temp);
        clear_buffer(&buffer_code);
    }
    
    {
        String src = create_string(&buffer_temp, "ret;");
        Function function = {};
        Error compile_result = compile(&buffer_code, keywords, &function, src);
        
        Assert(compile_result == Error_None);
        clear_rax();
        s32 function_result = ((fn_void_to_s32)function.call_function)();
        Assert(function_result == 0);
        
        clear_buffer(&buffer_temp);
        clear_buffer(&buffer_code);
    }
    
    {
        String src = create_string(&buffer_temp, "mov rax, 0\r\nret");
        Function function = {};
        Error compile_result = compile(&buffer_code, keywords, &function, src);
        
        Assert(compile_result == Error_None);
        s32 function_result = ((fn_void_to_s32)function.call_function)();
        Assert(function_result == 0);
        
        clear_buffer(&buffer_temp);
        clear_buffer(&buffer_code);
    }
    
    {
        String src = create_string(&buffer_temp, "mov rax, 100\r\nret");
        Function function = {};
        Error compile_result = compile(&buffer_code, keywords, &function, src);
        
        Assert(compile_result == Error_None);
        s32 function_result = ((fn_void_to_s32)function.call_function)();
        Assert(function_result == 100);
        
        clear_buffer(&buffer_temp);
        clear_buffer(&buffer_code);
    }
    
    {
        String src = create_string(&buffer_temp, "mov rax, 0;\r\nret;");
        Function function = {};
        Error compile_result = compile(&buffer_code, keywords, &function, src);
        
        Assert(compile_result == Error_None);
        s32 function_result = ((fn_void_to_s32)function.call_function)();
        Assert(function_result == 0);
        
        clear_buffer(&buffer_temp);
        clear_buffer(&buffer_code);
    }
    
    {
        String src = create_string(&buffer_temp, "return 0;");
        Function function = {};
        Error compile_result = compile(&buffer_code, keywords, &function, src);
        
        Assert(compile_result == Error_None);
        clear_rax();
        s32 function_result = ((fn_void_to_s32)function.call_function)();
        Assert(function_result == 0);
        
        clear_buffer(&buffer_temp);
        clear_buffer(&buffer_code);
    }
    
    {
        String src = create_string(&buffer_temp, "return 0");
        Function function = {};
        Error compile_result = compile(&buffer_code, keywords, &function, src);
        
        Assert(compile_result == Error_None);
        clear_rax();
        s32 function_result = ((fn_void_to_s32)function.call_function)();
        Assert(function_result == 0);
        
        clear_buffer(&buffer_temp);
        clear_buffer(&buffer_code);
    }
    
    {
        String src = create_string(&buffer_temp, "return 111");
        Function function = {};
        Error compile_result = compile(&buffer_code, keywords, &function, src);
        
        Assert(compile_result == Error_None);
        clear_rax();
        s32 function_result = ((fn_void_to_s32)function.call_function)();
        Assert(function_result == 111);
        
        clear_buffer(&buffer_temp);
        clear_buffer(&buffer_code);
    }
    
    {
        String src = create_string(&buffer_temp, "s32 main(){return 0}");
        Function function = {};
        Error compile_result = compile(&buffer_code, keywords, &function, src);
        
        Assert(compile_result == Error_None);
        clear_rax();
        s32 function_result = ((fn_void_to_s32)function.call_function)();
        Assert(function_result == 0);
        
        clear_buffer(&buffer_temp);
        clear_buffer(&buffer_code);
    }
    
    {
        String src = create_string(&buffer_temp, "s32 main() {\r\nreturn 2;\r\n}");
        Function function = {};
        Error compile_result = compile(&buffer_code, keywords, &function, src);
        
        Assert(compile_result == Error_None);
        clear_rax();
        s32 function_result = ((fn_void_to_s32)function.call_function)();
        Assert(function_result == 2);
        
        clear_buffer(&buffer_temp);
        clear_buffer(&buffer_code);
    }
    
    {
        String src = create_string(&buffer_temp, "s32 main() {\r\nreturn 100;\r\n}");
        Function function = {};
        Error compile_result = compile(&buffer_code, keywords, &function, src);
        
        Assert(compile_result == Error_None);
        s32 function_result = ((fn_void_to_s32)function.call_function)();
        Assert(function_result == 100);
        
        clear_buffer(&buffer_temp);
        clear_buffer(&buffer_code);
    }
    
    {
        String src = create_string(&buffer_temp, "   s32   main   (   )   {   \r\nreturn   2   ;\r\n   }");
        Function function = {};
        Error compile_result = compile(&buffer_code, keywords, &function, src);
        
        Assert(compile_result == Error_None);
        s32 function_result = ((fn_void_to_s32)function.call_function)();
        Assert(function_result == 2);
        
        clear_buffer(&buffer_temp);
        clear_buffer(&buffer_code);
    }
    
    {
        String src = create_string(&buffer_temp, "s32 main() {\r\nret\r\n}");
        Function function = {};
        Error compile_result = compile(&buffer_code, keywords, &function, src);
        
        Assert(compile_result == Warning_Ret_Used_Instead_Of_Return);
        clear_rax();
        s32 function_result = ((fn_void_to_s32)function.call_function)();
        Assert(function_result == 0);
        
        clear_buffer(&buffer_temp);
        clear_buffer(&buffer_code);
    }
    
    {
        String src = create_string(&buffer_temp, "s32 main() { return -5 }");
        Function function = {};
        Error compile_result = compile(&buffer_code, keywords, &function, src);
        
        Assert(compile_result == Warning_Ret_Used_Instead_Of_Return);
        clear_rax();
        s32 function_result = ((fn_void_to_s32)function.call_function)();
        Assert(function_result == 0);
        
        clear_buffer(&buffer_temp);
        clear_buffer(&buffer_code);
    }
    
    /*
*
*  INVALID
*
*/
    
    {
        String src = create_string(&buffer_temp, "s32 main() {\r\nreturn 2;");
        Function function = {};
        Error compile_result = compile(&buffer_code, keywords, &function, src);
        
        Assert(compile_result == Error_Need_Close_Brace);
        Assert(function.call_function == 0);
        
        clear_buffer(&buffer_temp);
        clear_buffer(&buffer_code);
    }
    
    {
        String src = create_string(&buffer_temp, "s32 main() \r\nreturn 2;}");
        Function function = {};
        Error compile_result = compile(&buffer_code, keywords, &function, src);
        
        Assert(compile_result == Error_Need_Open_Brace);
        Assert(function.call_function == 0);
        
        clear_buffer(&buffer_temp);
        clear_buffer(&buffer_code);
    }
    
    {
        String src = create_string(&buffer_temp, "s32 main( {\r\nreturn 2;\r\n}");
        Function function = {};
        Error compile_result = compile(&buffer_code, keywords, &function, src);
        
        Assert(compile_result == Error_Need_Close_Parenthesis);
        Assert(function.call_function == 0);
        
        clear_buffer(&buffer_temp);
        clear_buffer(&buffer_code);
    }
    
    {
        String src = create_string(&buffer_temp, "s32 main) {\r\nreturn 2;\r\n}");
        Function function = {};
        Error compile_result = compile(&buffer_code, keywords, &function, src);
        
        Assert(compile_result == Error_Need_Open_Parenthesis);
        Assert(function.call_function == 0);
        
        clear_buffer(&buffer_temp);
        clear_buffer(&buffer_code);
    }
    
    {
        // NOTE: I personally like the return statement and value on the same line.
        String src = create_string(&buffer_temp, "\r\ns32\r\nmain\r\n(\r\n)\r\n{\r\nreturn\r\n2\r\n;\r\n}");
        Function function = {};
        Error compile_result = compile(&buffer_code, keywords, &function, src);
        
        Assert(compile_result == Error_Return_Value_Required);
        Assert(function.call_function == 0);
        
        clear_buffer(&buffer_temp);
        clear_buffer(&buffer_code);
    }
    
    {
        String src = create_string(&buffer_temp, "return;");
        Function function = {};
        Error compile_result = compile(&buffer_code, keywords, &function, src);
        
        Assert(compile_result == Error_Return_Value_Required);
        Assert(function.call_function == 0);
        
        clear_buffer(&buffer_temp);
        clear_buffer(&buffer_code);
    }
    
    {
        String src = create_string(&buffer_temp, "return");
        Function function = {};
        Error compile_result = compile(&buffer_code, keywords, &function, src);
        
        Assert(compile_result == Error_Return_Value_Required);
        Assert(function.call_function == 0);
        
        clear_buffer(&buffer_temp);
        clear_buffer(&buffer_code);
    }
    
    {
        String src = create_string(&buffer_temp, "s32 main() {\r\n}");
        Function function = {};
        Error compile_result = compile(&buffer_code, keywords, &function, src);
        
        Assert(compile_result == Error_Need_Return_Statement);
        Assert(function.call_function == 0);
        
        clear_buffer(&buffer_temp);
        clear_buffer(&buffer_code);
    }
    
    {
        String src = create_string(&buffer_temp, "s32 main() {\r\nreturn;\r\n}");
        Function function = {};
        Error compile_result = compile(&buffer_code, keywords, &function, src);
        
        Assert(compile_result == Error_Return_Value_Required);
        Assert(function.call_function == 0);
        
        clear_buffer(&buffer_temp);
        clear_buffer(&buffer_code);
    }
    
    {
        String src = create_string(&buffer_temp, "s32 main() {\r\nreturn2;\r\n}");
        Function function = {};
        Error compile_result = compile(&buffer_code, keywords, &function, src);
        
        Assert(compile_result == Error_Need_Return_Statement);
        Assert(function.call_function == 0);
        
        clear_buffer(&buffer_temp);
        clear_buffer(&buffer_code);
    }
    
    {
        String src = create_string(&buffer_temp, "s32 main() {\r\nRETURN 2;\r\n}");
        Function function = {};
        Error compile_result = compile(&buffer_code, keywords, &function, src);
        
        Assert(compile_result == Error_Need_Return_Statement);
        Assert(function.call_function == 0);
        
        clear_buffer(&buffer_temp);
        clear_buffer(&buffer_code);
    }
    
    return(0);
}
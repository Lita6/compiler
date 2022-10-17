// TODO: The simplest, but complete programs that my compiler will accept are:
//       "s32 main(){return 0;}"
//       "s32 main(){return 0}"
//       "return 0"
//       "return 0;"
//       "mov rax, 0\r\nret"
//       "ret"

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
    Return_Type_Unknown,
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
    Instruction instr = {};
    for(u32 i = 0; i < src.len; i++)
    {
        Token_Type symbol_type = isSpecialSymbol(src.chars[i]);
        Token_Type string_type = Token_Type_None;
        b32 isDelimiter = ((symbol_type != Token_Type_None) || isWhiteSpace(src.chars[i]));
        
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
            
#if 0            
            if(isLastChar && (!isDelimiter))
            {
                current_string.len++;
            }
#endif
            
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
                    }break;
                    
                    case Token_Type_Main_Keyword:
                    {
                        function->id.type = string_type;
                        function->id.string.chars = current_string.chars;
                        function->id.string.len = current_string.len;
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
                    }break;
                    
                    case Token_Type_Ret_Keyword:
                    {
                        instr.instruction.string = current_string;
                        instr.instruction.type = string_type;
                        instr.isComplete = 1;
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
                function = {};
                break;
            }
            
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
    
    {
        String src = create_string(&buffer_temp, "ret");
        Function test_function = {};
        Error compile_result = compile(&buffer_code, keywords, &test_function, src);
        
        Assert(compile_result == Error_None);
        clear_rax();
        s32 function_result = ((fn_void_to_s32)test_function.call_function)();
        Assert(function_result == 0);
        
        clear_buffer(&buffer_temp);
        clear_buffer(&buffer_code);
    }
    
    {
        String src = create_string(&buffer_temp, "ret;");
        Function test_function = {};
        Error compile_result = compile(&buffer_code, keywords, &test_function, src);
        
        Assert(compile_result == Error_None);
        clear_rax();
        s32 function_result = ((fn_void_to_s32)test_function.call_function)();
        Assert(function_result == 0);
        
        clear_buffer(&buffer_temp);
        clear_buffer(&buffer_code);
    }
    
    {
        String src = create_string(&buffer_temp, "mov rax, 0\r\nret");
        Function test_function = {};
        Error compile_result = compile(&buffer_code, keywords, &test_function, src);
        
        Assert(compile_result == Error_None);
        s32 function_result = ((fn_void_to_s32)test_function.call_function)();
        Assert(function_result == 0);
        
        clear_buffer(&buffer_temp);
        clear_buffer(&buffer_code);
    }
    
    {
        String src = create_string(&buffer_temp, "mov rax, 100\r\nret");
        Function test_function = {};
        Error compile_result = compile(&buffer_code, keywords, &test_function, src);
        
        Assert(compile_result == Error_None);
        s32 function_result = ((fn_void_to_s32)test_function.call_function)();
        Assert(function_result == 100);
        
        clear_buffer(&buffer_temp);
        clear_buffer(&buffer_code);
    }
    
    {
        String src = create_string(&buffer_temp, "mov rax, 0;\r\nret;");
        Function test_function = {};
        Error compile_result = compile(&buffer_code, keywords, &test_function, src);
        
        Assert(compile_result == Error_None);
        s32 function_result = ((fn_void_to_s32)test_function.call_function)();
        Assert(function_result == 0);
        
        clear_buffer(&buffer_temp);
        clear_buffer(&buffer_code);
    }
    
    {
        String src = create_string(&buffer_temp, "return 0;");
        Function test_function = {};
        Error compile_result = compile(&buffer_code, keywords, &test_function, src);
        
        Assert(compile_result == Error_None);
        clear_rax();
        s32 function_result = ((fn_void_to_s32)test_function.call_function)();
        Assert(function_result == 0);
        
        clear_buffer(&buffer_temp);
        clear_buffer(&buffer_code);
    }
    
    {
        String src = create_string(&buffer_temp, "return 0");
        Function test_function = {};
        Error compile_result = compile(&buffer_code, keywords, &test_function, src);
        
        Assert(compile_result == Error_None);
        clear_rax();
        s32 function_result = ((fn_void_to_s32)test_function.call_function)();
        Assert(function_result == 0);
        
        clear_buffer(&buffer_temp);
        clear_buffer(&buffer_code);
    }
    
    {
        String src = create_string(&buffer_temp, "return 111");
        Function test_function = {};
        Error compile_result = compile(&buffer_code, keywords, &test_function, src);
        
        Assert(compile_result == Error_None);
        clear_rax();
        s32 function_result = ((fn_void_to_s32)test_function.call_function)();
        Assert(function_result == 111);
        
        clear_buffer(&buffer_temp);
        clear_buffer(&buffer_code);
    }
    
    {
        String src = create_string(&buffer_temp, "return;");
        Function test_function = {};
        Error compile_result = compile(&buffer_code, keywords, &test_function, src);
        
        Assert(compile_result == Error_Return_Value_Required);
        
        clear_buffer(&buffer_temp);
        clear_buffer(&buffer_code);
    }
    
    {
        String src = create_string(&buffer_temp, "return");
        Function test_function = {};
        Error compile_result = compile(&buffer_code, keywords, &test_function, src);
        
        Assert(compile_result == Error_Return_Value_Required);
        
        clear_buffer(&buffer_temp);
        clear_buffer(&buffer_code);
    }
    
    {
        String src = create_string(&buffer_temp, "s32 main(){return 0}");
        Function test_function = {};
        Error compile_result = compile(&buffer_code, keywords, &test_function, src);
        
        Assert(compile_result == Error_None);
        clear_rax();
        s32 function_result = ((fn_void_to_s32)test_function.call_function)();
        Assert(function_result == 0);
        
        clear_buffer(&buffer_temp);
        clear_buffer(&buffer_code);
    }
    
    {
        String src = create_string(&buffer_temp, "s32 main() {\r\nreturn 2;\r\n}");
        Function test_function = {};
        Error compile_result = compile(&buffer_code, keywords, &test_function, src);
        
        Assert(compile_result == Error_None);
        clear_rax();
        s32 function_result = ((fn_void_to_s32)test_function.call_function)();
        Assert(function_result == 2);
        
        clear_buffer(&buffer_temp);
        clear_buffer(&buffer_code);
    }
    
#if 0
    
    {
        // TODO: Need error for missing return statement of any kind.
        String src = create_string(&buffer_temp, "s32 main() {\r\n}");
        Function test_function = {};
        Error compile_result = compile(&buffer_code, keywords, &test_function, src);
        
        Assert(compile_result == Error_None);
        clear_rax();
        s32 function_result = ((fn_void_to_s32)test_function.call_function)();
        Assert(function_result == 2);
        
        clear_buffer(&buffer_temp);
        clear_buffer(&buffer_code);
    }
    
    {
        // TODO: Need warning for using ret instead of return.
        String src = create_string(&buffer_temp, "s32 main() {\r\nret\r\n}");
        Function test_function = {};
        Error compile_result = compile(&buffer_code, keywords, &test_function, src);
        
        Assert(compile_result == Error_None);
        clear_rax();
        s32 function_result = ((fn_void_to_s32)test_function.call_function)();
        Assert(function_result == 2);
        
        clear_buffer(&buffer_temp);
        clear_buffer(&buffer_code);
    }
    
    {
        // TODO: Need error for missing close parenthesis.
        String src = create_string(&buffer_temp, "s32 main( {\r\nreturn 2;\r\n}");
        Function test_function = {};
        Error compile_result = compile(&buffer_code, keywords, &test_function, src);
        
        Assert(compile_result == Error_None);
        clear_rax();
        s32 function_result = ((fn_void_to_s32)test_function.call_function)();
        Assert(function_result == 2);
        
        clear_buffer(&buffer_temp);
        clear_buffer(&buffer_code);
    }
    
    {
        String src = create_string(&buffer_temp, "s32 main() {\r\nreturn;\r\n}");
        Function test_function = {};
        Error compile_result = compile(&buffer_code, keywords, &test_function, src);
        
        Assert(compile_result == Error_Return_Value_Required);
        
        clear_buffer(&buffer_temp);
        clear_buffer(&buffer_code);
    }
    
    {
        // TODO: Need to return error on missing close brace.
        String src = create_string(&buffer_temp, "s32 main() {\r\nreturn 2;");
        Function test_function = {};
        Error compile_result = compile(&buffer_code, keywords, &test_function, src);
        
        Assert(compile_result == Error_None);
        clear_rax();
        s32 function_result = ((fn_void_to_s32)test_function.call_function)();
        Assert(function_result == 2);
        
        clear_buffer(&buffer_temp);
        clear_buffer(&buffer_code);
    }
    
    {
        String src = create_string(&buffer_temp, "s32 main() {\r\nreturn 2\r\n}");
        Function test_function = {};
        Error compile_result = compile(&buffer_code, keywords, &test_function, src);
        
        Assert(compile_result == Error_None);
        clear_rax();
        s32 function_result = ((fn_void_to_s32)test_function.call_function)();
        Assert(function_result == 2);
        
        clear_buffer(&buffer_temp);
        clear_buffer(&buffer_code);
    }
    
    {
        // TODO: Need error for not returning a value or having a return statement.
        String src = create_string(&buffer_temp, "s32 main() {\r\nreturn2;\r\n}");
        Function test_function = {};
        Error compile_result = compile(&buffer_code, keywords, &test_function, src);
        
        Assert(compile_result == Error_None);
        clear_rax();
        s32 function_result = ((fn_void_to_s32)test_function.call_function)();
        Assert(function_result == 0);
        
        clear_buffer(&buffer_temp);
        clear_buffer(&buffer_code);
    }
    
    {
        // TODO: Need error for not returning a value or having a return statement.
        String src = create_string(&buffer_temp, "s32 main() {\r\nRETURN 2;\r\n}");
        Function test_function = {};
        Error compile_result = compile(&buffer_code, keywords, &test_function, src);
        
        Assert(compile_result == Error_None);
        clear_rax();
        s32 function_result = ((fn_void_to_s32)test_function.call_function)();
        Assert(function_result == 0);
        
        clear_buffer(&buffer_temp);
        clear_buffer(&buffer_code);
    }
    
    {
        String src = create_string(&buffer_temp, "s32 main() {\r\nreturn 100;\r\n}");
        Function test_function = {};
        Error compile_result = compile(&buffer_code, keywords, &test_function, src);
        
        Assert(compile_result == Error_None);
        clear_rax();
        s32 function_result = ((fn_void_to_s32)test_function.call_function)();
        Assert(function_result == 100);
        
        clear_buffer(&buffer_temp);
        clear_buffer(&buffer_code);
    }
    
    {
        // NOTE: I personally like the return statement and value on the same line.
        String src = create_string(&buffer_temp, "\r\ns32\r\nmain\r\n(\r\n)\r\n{\r\nreturn\r\n2\r\n;\r\n}");
        Function test_function = {};
        Error compile_result = compile(&buffer_code, keywords, &test_function, src);
        
        Assert(compile_result == Error_Return_Value_Required);
        
        clear_buffer(&buffer_temp);
        clear_buffer(&buffer_code);
    }
    
    {
        String src = create_string(&buffer_temp, "s32 main(){return 2;}");
        Function test_function = {};
        Error compile_result = compile(&buffer_code, keywords, &test_function, src);
        
        Assert(compile_result == Error_None);
        clear_rax();
        s32 function_result = ((fn_void_to_s32)test_function.call_function)();
        Assert(function_result == 0);
        
        clear_buffer(&buffer_temp);
        clear_buffer(&buffer_code);
    }
    
    {
        String src = create_string(&buffer_temp, "s32 main() {\r\nreturn 0;\r\n}");
        Function test_function = {};
        Error compile_result = compile(&buffer_code, keywords, &test_function, src);
        
        Assert(compile_result == Error_None);
        clear_rax();
        s32 function_result = ((fn_void_to_s32)test_function.call_function)();
        Assert(function_result == 0);
        
        clear_buffer(&buffer_temp);
        clear_buffer(&buffer_code);
    }
    
    {
        String src = create_string(&buffer_temp, "   s32   main   (   )   {   \r\nreturn   2   ;\r\n   }");
        Function test_function = {};
        Error compile_result = compile(&buffer_code, keywords, &test_function, src);
        
        Assert(compile_result == Error_None);
        clear_rax();
        s32 function_result = ((fn_void_to_s32)test_function.call_function)();
        Assert(function_result == 0);
        
        clear_buffer(&buffer_temp);
        clear_buffer(&buffer_code);
    }
#endif
    
    return(0);
}
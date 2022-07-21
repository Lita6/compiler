#include <windows.h>

#include "win32_compiler.h"

struct Variable
{
    u8 name;
    u8 offset;
};

struct Patch
{
    u8 *location;
    u32 size;
    Variable *var;
};

Patch
create_patch
(Buffer *buffer, u32 size)
{
    Patch result = {};
    result.size = size;
    result.location = buffer->end - size;
    
    return(result);
}

struct Program_Info
{
    Buffer *buffer;
    u8 *ch;
    u8 *chMax;
    u8 stackDynMax;
    u8 stackCurrent;
    Buffer *vars;
    u8 varCount;
    Buffer *patches;
};

void
stackPush
(Program_Info *info)
{
    info->stackCurrent += sizeof(u32);
    
    if(info->stackCurrent > info->stackDynMax)
    {
        info->stackDynMax = info->stackCurrent;
    }
    
    if(info->stackCurrent == 0xff)
    {
        Assert(!"Cannot encode anymore stack offsets with a u8.");
    }
}

void
stackPop
(Program_Info *info)
{
    if(info->stackCurrent == 0)
    {
        Assert(!"Push and pop mismatch.");
    }
    
    info->stackCurrent -= sizeof(u32);
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

void
GetChar
(Program_Info *info)
{
    
    if(info->ch != 0)
    {    
        info->ch++;
        if(info->ch >= info->chMax)
        {
            info->ch = 0;
        }
    }
}

void Expression(Program_Info *info);

void
Factor
(Program_Info *info)
{
    if(*info->ch == '(')
    {
        GetChar(info);
        Expression(info);
        
        if(*info->ch != ')')
        {
            Assert(!"Closing parenthesis expected.");
        }
    }
    else
    {
        if(isDigit(*info->ch))
        {
            u32 num = 0;
            num = (u32)((s32)(*info->ch) - '0');
            
            // mov eax, imm32
            buffer_append_u8(info->buffer, 0xb8);
            buffer_append_u32(info->buffer, num);
        }
        else if(isAlpha(*info->ch))
        {
            Variable *var = 0;
            if(info->varCount > 0)
            {
                Variable *end = (Variable *)(info->vars->memory + (info->varCount * sizeof(Variable)));
                for(Variable *search = (Variable *)info->vars->memory; search < end; search++)
                {
                    if(search->name == *info->ch)
                    {
                        var = search;
                        break;
                    }
                }
            }
            
            if(var == 0)
            {
                var = (Variable *)buffer_allocate(info->vars, sizeof(Variable));
                var->name = *info->ch;
                info->varCount++;
            }
            
            //mov eax, dword ptr[rsp + imm8]
            buffer_append_u8(info->buffer, 0x8b);
            buffer_append_u8(info->buffer, 0x44);
            buffer_append_u8(info->buffer, 0x24);
            buffer_append_u8(info->buffer, 0xcc);
            
            Patch *patch = (Patch *)buffer_allocate(info->patches, sizeof(Patch));
            *patch = create_patch(info->buffer, sizeof(u8));
            patch->var = var;
        }
        else
        {
            Assert(!"This character is not accepted at this time.");
        }
    }
}

void
Multiply
(Program_Info *info)
{
    Factor(info);
    
    // mov ecx, dword ptr[rsp + info->stackCurrent]
    buffer_append_u8(info->buffer, 0x8b);
    buffer_append_u8(info->buffer, 0x4c);
    buffer_append_u8(info->buffer, 0x24);
    buffer_append_u8(info->buffer, info->stackCurrent);
    
    stackPop(info);
    
    // imul eax, ecx
    buffer_append_u8(info->buffer, 0x0f);
    buffer_append_u8(info->buffer, 0xaf);
    buffer_append_u8(info->buffer, 0xc1);
}

void
Divide
(Program_Info *info)
{
    Factor(info);
    
    // xor edx, edx
    buffer_append_u8(info->buffer, 0x31);
    buffer_append_u8(info->buffer, 0xd2);
    
    // mov ecx, eax
    buffer_append_u8(info->buffer, 0x89);
    buffer_append_u8(info->buffer, 0xc1);
    
    // mov eax, dword ptr[rsp + info->stackCurrent
    buffer_append_u8(info->buffer, 0x8b);
    buffer_append_u8(info->buffer, 0x44);
    buffer_append_u8(info->buffer, 0x24);
    buffer_append_u8(info->buffer, info->stackCurrent);
    
    stackPop(info);
    
    // idiv ecx
    buffer_append_u8(info->buffer, 0xf7);
    buffer_append_u8(info->buffer, 0xf9);
}

void
Term
(Program_Info *info)
{
    Factor(info);
    GetChar(info);
    
    while(info->ch != 0 && ((*info->ch == '*') || (*info->ch == '/')))
    {
        
        stackPush(info);
        
        // mov dword ptr[rsp + info->stackCurrent], eax
        buffer_append_u8(info->buffer, 0x89);
        buffer_append_u8(info->buffer, 0x44);
        buffer_append_u8(info->buffer, 0x24);
        buffer_append_u8(info->buffer, info->stackCurrent);
        
        u8 c = *info->ch;
        GetChar(info);
        
        switch(c)
        {
            case '*':
            {
                Multiply(info);
            }break;
            
            case '/':
            {
                Divide(info);
            }break;
            
            default:
            {
                Assert(!"Unsupported operation.");
            };
        }
        
        GetChar(info);
        
    }
}

void
Add
(Program_Info *info)
{
    Term(info);
    
    // mov ecx, dword ptr[rsp + info->stackCurrent
    buffer_append_u8(info->buffer, 0x8b);
    buffer_append_u8(info->buffer, 0x4c);
    buffer_append_u8(info->buffer, 0x24);
    buffer_append_u8(info->buffer, info->stackCurrent);
    
    stackPop(info);
    
    // add eax, ecx
    buffer_append_u8(info->buffer, 0x01);
    buffer_append_u8(info->buffer, 0xc8);
}

void
Sub
(Program_Info *info)
{
    Term(info);
    
    // mov ecx, dword ptr[rsp + info->stackCurrent
    buffer_append_u8(info->buffer, 0x8b);
    buffer_append_u8(info->buffer, 0x4c);
    buffer_append_u8(info->buffer, 0x24);
    buffer_append_u8(info->buffer, info->stackCurrent);
    
    stackPop(info);
    
    // sub eax, ecx
    buffer_append_u8(info->buffer, 0x29);
    buffer_append_u8(info->buffer, 0xc8);
    
    // neg eax
    buffer_append_u8(info->buffer, 0xf7);
    buffer_append_u8(info->buffer, 0xd8);
}

void
Expression
(Program_Info *info)
{
    
    if(*info->ch == '-')
    {
        // xor eax, eax
        buffer_append_u8(info->buffer, 0x31);
        buffer_append_u8(info->buffer, 0xc0);
    }
    else
    {
        Term(info);
    }
    
    while(info->ch != 0 && ((*info->ch == '+') || (*info->ch == '-')))
    {
        
        stackPush(info);
        
        // mov dword ptr[rsp + info->stackCurrent], eax
        buffer_append_u8(info->buffer, 0x89);
        buffer_append_u8(info->buffer, 0x44);
        buffer_append_u8(info->buffer, 0x24);
        buffer_append_u8(info->buffer, info->stackCurrent);
        
        u8 c = *info->ch;
        GetChar(info);
        
        switch(c)
        {
            case '+':
            {
                Add(info);
            }break;
            
            case '-':
            {
                Sub(info);
            }break;
            
            default:
            {
                Assert(!"Unsupported operation.");
            };
        }
    }
}

void
compile
(Program_Info *info)
{
    // Don't overwrite your return address!
    u8 stack_alignment = sizeof(u32);
    
    // mov rax, rsp
    buffer_append_u8(info->buffer, 0x48);
    buffer_append_u8(info->buffer, 0x89);
    buffer_append_u8(info->buffer, 0xe0);
    
    // sub rax, imm8
    buffer_append_u8(info->buffer, 0x48);
    buffer_append_u8(info->buffer, 0x83);
    buffer_append_u8(info->buffer, 0xe8);
    buffer_append_u8(info->buffer, stack_alignment);
    
    // sub rsp, imm8
    buffer_append_u8(info->buffer, 0x48);
    buffer_append_u8(info->buffer, 0x83);
    buffer_append_u8(info->buffer, 0xec);
    buffer_append_u8(info->buffer, 0xcc);
    Patch start_patch = create_patch(info->buffer, sizeof(u8));
    
    // mov rsp, rcx
    buffer_append_u8(info->buffer, 0x48);
    buffer_append_u8(info->buffer, 0x89);
    buffer_append_u8(info->buffer, 0xe1);
    
    u8 *start_loop = info->buffer->end;
    
    // cmp rax, rcx
    buffer_append_u8(info->buffer, 0x48);
    buffer_append_u8(info->buffer, 0x39);
    buffer_append_u8(info->buffer, 0xc8);
    
    // je imm8
    buffer_append_u8(info->buffer, 0x74);
    buffer_append_u8(info->buffer, 0xcc);
    Patch je_patch = create_patch(info->buffer, sizeof(u8));
    u8 *conditional_jump = info->buffer->end;
    
    // mov dword ptr[rax], 0x00
    buffer_append_u8(info->buffer, 0xc7);
    buffer_append_u8(info->buffer, 0x00);
    buffer_append_u32(info->buffer, (u32)0);
    
    // sub rax, imm8
    buffer_append_u8(info->buffer, 0x48);
    buffer_append_u8(info->buffer, 0x83);
    buffer_append_u8(info->buffer, 0xe8);
    buffer_append_u8(info->buffer, sizeof(u32));
    
    // jmp start_loop
    buffer_append_u8(info->buffer, 0xeb);
    buffer_append_u8(info->buffer, 0xcc);
    Patch jmp_patch = create_patch(info->buffer, sizeof(u8));
    u8 *end_loop = info->buffer->end;
    
    *(s8 *)je_patch.location = (s8)(end_loop - conditional_jump);
    *(s8 *)jmp_patch.location = (s8)(start_loop - end_loop);
    
    Expression(info);
    
    Variable *var = (Variable *)info->vars->memory;
    for(u8 i = 0; i < info->varCount; i++)
    {
        var[i].offset = ((i+1) * sizeof(u32)) + info->stackDynMax;
    }
    
    for(Patch *patch = (Patch *)info->patches->memory; patch < (Patch *)info->patches->end; patch++)
    {
        *patch->location = patch->var->offset;
    }
    
    
    s16 stack_total = (s16)(info->stackDynMax + info->varCount * sizeof(u32) + stack_alignment);
    Assert((stack_total > 0) && (stack_total <= MAX_U8));
    
    *start_patch.location = (u8)stack_total;
    
    // add rsp, imm8
    buffer_append_u8(info->buffer, 0x48);
    buffer_append_u8(info->buffer, 0x83);
    buffer_append_u8(info->buffer, 0xc4);
    buffer_append_u8(info->buffer, (u8)stack_total);
    
    // ret
    buffer_append_u8(info->buffer, 0xc3);
    
}

void
test
(Buffer *functions, Buffer *vars, Buffer *patches, String src, s32 expected)
{
    
    Program_Info info = {};
    info.buffer = functions;
    info.ch = src.chars;
    info.chMax = (src.chars + src.len);
    info.vars = vars;
    info.patches = patches;
    
    compile(&info);
    fn_void_to_s32 program = (fn_void_to_s32)functions->memory;
    s32 result = program();
    Assert(result == expected);
    
    clear_buffer(functions);
    clear_buffer(vars);
    clear_buffer(patches);
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
    Buffer buffer_patches = create_buffer(PAGE, PAGE_READWRITE);
    Buffer buffer_functions = create_buffer(PAGE, PAGE_EXECUTE_READWRITE);
    
    {
        String src = create_string(&buffer_strings, "5");
        test(&buffer_functions, &buffer_vars, &buffer_patches, src, 5);
        clear_buffer(&buffer_strings);
    }
    
    {
        String src = create_string(&buffer_strings, "4+7");
        test(&buffer_functions, &buffer_vars, &buffer_patches, src, 11);
        clear_buffer(&buffer_strings);
    }
    
    {
        String src = create_string(&buffer_strings, "2-4");
        test(&buffer_functions, &buffer_vars, &buffer_patches, src, -2);
        clear_buffer(&buffer_strings);
    }
    
    {
        String src = create_string(&buffer_strings, "7+8-9");
        test(&buffer_functions, &buffer_vars, &buffer_patches, src, 6);
        clear_buffer(&buffer_strings);
    }
    
    {
        String src = create_string(&buffer_strings, "2+3*4");
        test(&buffer_functions, &buffer_vars, &buffer_patches, src, 14);
        clear_buffer(&buffer_strings);
    }
    
    {
        String src = create_string(&buffer_strings, "9/3");
        test(&buffer_functions, &buffer_vars, &buffer_patches, src, 3);
        clear_buffer(&buffer_strings);
    }
    
    {
        String src = create_string(&buffer_strings, "1+2*3*4+5/6-7-8-9");
        s32 expected = 1+2*3*4+5/6-7-8-9;
        test(&buffer_functions, &buffer_vars, &buffer_patches, src, expected);
        clear_buffer(&buffer_strings);
    }
    
    {
        String src = create_string(&buffer_strings, "1+2*(3+4)-5*(6-7*(8*9))");
        s32 expected = 1+2*(3+4)-5*(6-7*(8*9));
        test(&buffer_functions, &buffer_vars, &buffer_patches, src, expected);
        clear_buffer(&buffer_strings);
    }
    
    {
        String src = create_string(&buffer_strings, "-8");
        test(&buffer_functions, &buffer_vars, &buffer_patches, src, -8);
        clear_buffer(&buffer_strings);
    }
    
    {
        String src = create_string(&buffer_strings, "a+b");
        test(&buffer_functions, &buffer_vars, &buffer_patches, src, 0);
        clear_buffer(&buffer_strings);
    }
    
    return(0);
}
/* date = October 8th 2022 11:48 pm */

#ifndef WIN64_BUFFER_H
#define WIN64_BUFFER_H

struct Buffer
{
    u8 *memory;
    u8 *end;
    u32 size;
};

Buffer
create_buffer
(u32 size, s32 permission)
{
    Buffer buffer = {};
    buffer.memory = (u8 *)(VirtualAlloc(0, (SIZE_T)size, MEM_COMMIT | MEM_RESERVE, (DWORD)permission));
    Assert(buffer.memory);
    buffer.end = buffer.memory;
    buffer.size = size;
    
    return(buffer);
}

u8 *
buffer_allocate
(Buffer *buffer, u32 amount)
{
    
    Assert((buffer->end + amount) <= (buffer->memory + buffer->size));
    
    u8 *Result = buffer->end;
    buffer->end += amount;
    
    return(Result);
}

void
clear_buffer
(Buffer *buffer)
{
    for(u8 *i = buffer->memory; i < buffer->end; i++)
    {
        *i = 0;
    }
    buffer->end = buffer->memory;
}

#define define_buffer_append(Type) \
inline void \
buffer_append_##Type \
(Buffer *buffer, Type value) \
{ \
Assert((buffer->end + sizeof(Type)) <= (buffer->memory + buffer->size)); \
*(Type *)buffer->end = value; \
buffer->end += sizeof(Type); \
}

define_buffer_append(u8)
define_buffer_append(u16)
define_buffer_append(u32)
define_buffer_append(u64)
#undef define_buffer_append

#endif //WIN64_BUFFER_H

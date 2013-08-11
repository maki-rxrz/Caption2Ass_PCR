//------------------------------------------------------------------------------
// file_reader.h
//------------------------------------------------------------------------------
#ifndef __FILE_READER_H__
#define __FILE_READER_H__

#include <stdint.h>
#include <tchar.h>

typedef enum {
    FR_FILE_ERROR  = -3,
    FR_PARAM_ERROR = -2,
    FR_FAILURE     = -1,
    FR_SUCCESS     =  0,
    FR_EOF         =  1
} fr_return_code_type;

/* I/F class */

class IFileReader
{
public:
    virtual int64_t     get_size( void ) = 0;
    virtual int64_t     ftell   ( void ) = 0;
    virtual int         fread   ( uint8_t *read_buffer, int64_t read_size, int64_t *dest_size ) = 0;
    virtual int         fseek   ( int64_t offset, int origin ) = 0;
    virtual int         open    ( TCHAR *file_name, uint64_t buffer_size ) = 0;
    virtual void        close   ( void ) = 0;
    virtual int         init    ( void ) = 0;
    virtual void        release ( void ) = 0;
};

extern IFileReader * CreateFileReader( void );

#endif // __FILE_READER_H__

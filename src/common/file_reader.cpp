//------------------------------------------------------------------------------
// file_reader.cpp
//------------------------------------------------------------------------------

#include "file_common.h"

#include <stdlib.h>
#include <string.h>
#include <tchar.h>

#include "file_reader.h"

/*============================================================================
 *  Utility functions
 *==========================================================================*/

static int64_t get_file_size( TCHAR *file_name )
{
    int64_t file_size;

    FILE *fp = NULL;
    if (_tfopen_s(&fp, file_name, _T("rb")) || !fp)
        return -1;

    fseeko( fp, 0, SEEK_END );
    file_size = ftello( fp );

    fclose( fp );

    return file_size;
}

/*============================================================================
 *  Class -  File Reader
 *==========================================================================*/

class CFileReader : public IFileReader
{
protected:
    static const uint64_t   buffer_size_min     = (1 << 10);    // (0x0000400)
    static const uint64_t   buffer_size_max     = (1 << 23);    // (0x0800000)
    static const uint64_t   buffer_default_size = (1 << 11);    // (2048)

protected:
    typedef struct {
        FILE       *fp;
        uint64_t    read_pos;
        int64_t     file_size;
        uint64_t    buffer_size;
        struct {
            uint8_t    *buf;
            uint64_t    size;
            uint64_t    pos;
        } cache;
    } file_read_context_t;

    typedef enum {
        FR_STATUS_NOINIT,
        FR_STATUS_CLOSED,
        FR_STATUS_OPENED
    } file_read_staus_type;

    file_read_context_t *   fr_ctx;
    file_read_staus_type    fr_status;

public:
    CFileReader( void );
    ~CFileReader( void );

    int64_t     get_size( void );
    int64_t     ftell   ( void );
    int         fread   ( uint8_t *read_buffer, int64_t read_size, int64_t *dest_size );
    int         fseek   ( int64_t offset, int origin );
    int         open    ( TCHAR *file_name, uint64_t buffer_size );
    void        close   ( void );
    int         init    ( void );
    void        release ( void );
};

/* functions */

int64_t CFileReader::get_size( void )
{
    if( fr_status != FR_STATUS_OPENED )
        return -1;
    return fr_ctx->file_size;
}

int64_t CFileReader::ftell( void )
{
    if( fr_status != FR_STATUS_OPENED )
        return -1;
    return fr_ctx->read_pos + fr_ctx->cache.pos;
}

int CFileReader::fread( uint8_t *read_buffer, int64_t read_size, int64_t *dest_size )
{
    if( fr_status != FR_STATUS_OPENED )
        return FR_FAILURE;

    uint8_t *buf  = read_buffer;
    int64_t  size = read_size;

    if( dest_size )
        *dest_size = 0;

    while( read_size )
    {
        uint64_t rest_size = fr_ctx->cache.size - fr_ctx->cache.pos;

        if( rest_size < (uint64_t)read_size )
        {
            if( rest_size )
            {
                /* Copy the rest data from cache. */
                memcpy( buf, &(fr_ctx->cache.buf[fr_ctx->cache.pos]), (size_t)rest_size );
                buf += rest_size;
                read_size -= rest_size;
                rest_size = 0;
            }

            /* Update the information of file read position. */
            fr_ctx->read_pos += fr_ctx->cache.size;

            /* Read data to cache. */
            uint64_t cache_size = ::fread( fr_ctx->cache.buf, 1, (size_t)(fr_ctx->buffer_size), fr_ctx->fp );
            if( cache_size == 0 )
                goto fail;
            fr_ctx->cache.size = cache_size;
            fr_ctx->cache.pos  = 0;
        }
        else
        {
            /* Copy data from cache. */
            if( buf )
                memcpy( buf, &(fr_ctx->cache.buf[fr_ctx->cache.pos]), (size_t)read_size );
            fr_ctx->cache.pos += read_size;
            read_size = 0;
        }
    }

    if( dest_size )
        *dest_size = size;

    return FR_SUCCESS;

fail:
    if( dest_size )
        *dest_size = size - read_size;

    return FR_EOF;
}

int CFileReader::fseek( int64_t seek_offset, int origin )
{
    if( fr_status != FR_STATUS_OPENED )
        return FR_FAILURE;

    int64_t position = -1;

    /* Check seek position. */
    switch( origin )
    {
        case SEEK_SET :
            position = seek_offset;
            break;
        case SEEK_END :
            position = fr_ctx->file_size - seek_offset;
            break;
        case SEEK_CUR :
         /* position = ::ftell( fr_ctx ) + seek_offset; */
            position = (fr_ctx->read_pos + fr_ctx->cache.pos) + seek_offset;
            break;
        default :
            break;
    }
    if( position < 0 || fr_ctx->file_size < position )
        return FR_FAILURE;

    /* Seek. */
    int64_t offset = position - fr_ctx->read_pos;
    if( 0 <= offset && (uint64_t)offset < fr_ctx->cache.size )
    {
        /* Cache hit. */
        fr_ctx->cache.pos = offset;
    }
    else
    {
        /* No data in cache. */
        int64_t cache_start_pos = position / fr_ctx->buffer_size * fr_ctx->buffer_size;

        fseeko( fr_ctx->fp, cache_start_pos, SEEK_SET );
        fr_ctx->read_pos   = cache_start_pos;
        fr_ctx->cache.size = 0;
        fr_ctx->cache.pos  = 0;

        offset = position - cache_start_pos;
        if( offset )
            fread( NULL, offset, NULL );
    }
    return FR_SUCCESS;
}

int CFileReader::open( TCHAR *file_name, uint64_t buffer_size )
{
    if( fr_status != FR_STATUS_CLOSED )
        return FR_FAILURE;

    int64_t  file_size = 0;
    uint8_t *buffer    = NULL;
    FILE    *fp        = NULL;
    if (_tfopen_s( &fp, file_name, _T("rb") ) || !fp)
        return FR_FILE_ERROR;

    file_size = get_file_size( file_name );

    if( buffer_size == 0 )
        buffer_size = buffer_default_size;
    else if( buffer_size > buffer_size_max )
        buffer_size = buffer_size_max;
    else if( buffer_size < buffer_size_min )
        buffer_size = buffer_size_min;

    buffer = (uint8_t *)malloc( (size_t)buffer_size );
    if( !buffer )
        goto fail;

    /* Set up. */
    memset( fr_ctx, 0, sizeof(file_read_context_t) );
    fr_ctx->fp          = fp;
    fr_ctx->file_size   = file_size;
    fr_ctx->buffer_size = buffer_size;
    fr_ctx->cache.buf   = buffer;
    fr_status = FR_STATUS_OPENED;

    return FR_SUCCESS;

fail:
    fclose( fp );

    return FR_FAILURE;
}

void CFileReader::close( void )
{
    if( fr_status != FR_STATUS_OPENED )
        return;

    if( fr_ctx->cache.buf )
        free( fr_ctx->cache.buf );
    if( fr_ctx->fp )
        fclose( fr_ctx->fp );

    memset( fr_ctx, 0, sizeof(file_read_context_t) );
    fr_status = FR_STATUS_CLOSED;
}

int CFileReader::init( void )
{
    if( fr_ctx )
        return FR_FAILURE;

    file_read_context_t *ctx = static_cast<file_read_context_t *>(malloc( sizeof(file_read_context_t)) );
    if( !ctx )
        return FR_FAILURE;

    memset( ctx, 0, sizeof(file_read_context_t) );
    fr_ctx    = ctx;
    fr_status = FR_STATUS_CLOSED;

    return FR_SUCCESS;
}

void CFileReader::release( void )
{
    if( !fr_ctx )
        return;

    close();

    free( fr_ctx );
    fr_ctx    = NULL;
    fr_status = FR_STATUS_NOINIT;
}

CFileReader::CFileReader( void )
 : fr_ctx(NULL), fr_status(FR_STATUS_NOINIT)
{
}

CFileReader::~CFileReader( void )
{
    release();
}

/* Create function */

extern IFileReader * CreateFileReader( void )
{
    return static_cast<IFileReader *>(new CFileReader());
}

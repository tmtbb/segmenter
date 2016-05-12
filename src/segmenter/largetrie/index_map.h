#ifndef __RELRANK_INDEX_DB_HEADER__
#define __RELRANK_INDEX_DB_HEADER__
#include "trie.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <stdint.h>
#include <vector>
#include <map>
#include <algorithm>
#include <queue>
#include <string>
#include <assert.h>
#include <string.h>

namespace Relrank
{

template <class _Tp>
class IndexMap 
{
public:
    struct dbNode {
        uint32_t _M_d_offset;    /* the max value length can save is 2 ^ 32 * 4 : 16G */
        uint32_t _M_d_cnt_flag;  /* the max value count  can save is the sizeof(_M_d_cnt_flag) */
    };

	struct dbHead {
#define DB_MAGIC	"IndexDBbibo"
		char		_M_magic[12];
		int			_M_size;
		int			_M_n_prop;
		_Tp*	    _M_p_prop;
	};

public:

	IndexMap()
	{
        _M_n_map = 0; 
        _M_n_data = 0;
        _M_p_map = NULL;
        _M_metaNode = NULL;
        _M_p_data = NULL;
        __Initialize();
	}
	
	~IndexMap()
	{
		__clean( ); 
	}

private:
	dbNode*     _M_metaNode;
	dbHead      _M_metadata;
	long        _M_n_map;
	long        _M_n_data;
	char*       _M_p_map;
	float*      _M_p_data;
			
private:
	int  __wstrlen( uint16_t *s )
	{
		if( s == NULL ) return 0;
		uint16_t *p = s; while( *p ) p++;
		return (int)( p - s );
	}

	void __resizeProp( int nIdx )
	{
		if( nIdx >= _M_metadata._M_n_prop ) {
			size_t __nsize = ( nIdx + 127 ) & ~63;
			_M_metadata._M_p_prop = (_Tp *)realloc( _M_metadata._M_p_prop, __nsize * sizeof(_Tp) );
			assert( _M_metadata._M_p_prop != NULL );
			memset( _M_metadata._M_p_prop + _M_metadata._M_n_prop, 0, sizeof( _Tp ) * ( __nsize - _M_metadata._M_n_prop) );
			_M_metadata._M_n_prop = __nsize;
		}
        if ( nIdx > _M_metadata._M_size) _M_metadata._M_size = nIdx;
	}
	
	void __setProp( int n, const _Tp &iProp )
	{
		__resizeProp( n ); 
        memcpy(&_M_metadata._M_p_prop[n], &iProp, sizeof(iProp));
	}

	void __Initialize()
	{
		memset( &_M_metadata, 0, sizeof( dbHead ) );
		memcpy( _M_metadata._M_magic, DB_MAGIC, 12 );
	}

	void __clean( )
	{
		if ( _M_p_map ) {
			munmap( _M_p_map, _M_n_map); 
			_M_p_map = NULL; _M_n_map = 0;
		}
		else {
			if( _M_metadata._M_p_prop) { free( _M_metadata._M_p_prop); }
		}
		memset( &_M_metadata, 0, sizeof( dbHead ) );
	}

private:

	bool __static_load(const char *sIndexName)
	{
		int i,nFD;
		dbHead __daHead;
		uint32_t __flag;

		nFD = open( sIndexName, O_RDONLY, 0666);
		if( nFD < 0 ) return false;

		//HEADER
		i = read( nFD, &__daHead, sizeof( dbHead ) );
		if( i != sizeof( dbHead ) ) { close(nFD); return false; }
		if( memcmp( __daHead._M_magic, DB_MAGIC, 12 ) ) {
			close( nFD ); return false;
		}

		__clean( );

		_M_n_map = lseek( nFD, 0L, SEEK_END );
		lseek ( nFD, 0L, SEEK_SET);

		_M_p_map = (char *)::mmap( NULL, _M_n_map, PROT_READ, MAP_FILE|MAP_SHARED, nFD, 0);
		if( _M_p_map == MAP_FAILED ) {
			close( nFD ); return false;
		}

		/* mark */
		int iStep = 64 * 1024;
		char c[iStep];
		char *p = (char *)_M_p_map;
		for (int64_t i = 0; i < (_M_n_map/iStep); i++) memcpy(c, p + i * iStep, iStep);

		__daHead._M_p_prop = NULL;
		_M_metaNode = (dbNode *)(_M_p_map + sizeof( dbHead ));
		_M_p_data   = (float *)(_M_metaNode + __daHead._M_n_prop);

		memcpy( &_M_metadata, &__daHead, sizeof( dbHead ) );

        _M_n_data = _M_metaNode[__daHead._M_n_prop - 1]._M_d_offset;
        __flag    = _M_metaNode[__daHead._M_n_prop - 1]._M_d_cnt_flag;
        while (__flag) {
            if (__flag & 0x01) {
                _M_n_data++;
            }
            __flag = __flag >> 1;
        }

        if(_M_n_map != (long)((long)sizeof(dbHead) + (long)__daHead._M_n_prop * sizeof(dbNode) + _M_n_data * sizeof(float))) {
            close( nFD );
            __clean();
            return false;
        }

		close( nFD );
		return true;
	}

public:

	void clean( ) { __clean(); }

	void clear( )
	{
		_M_metadata._M_size = 0;
        if (_M_metaNode) {
            memset(_M_metaNode, 0, sizeof(dbNode) * _M_metadata._M_n_prop);
        }
        if (_M_p_data) {
            memset(_M_p_data,   0, sizeof(float)  * _M_n_data);
        }
	}

	int size( ) { return _M_metadata._M_size; }	

    bool set( int nId, const _Tp &iProp)
    {
        if( nId <= 0 ) return false;
	    __setProp( nId, iProp );
        return true;
    }

    bool get(int nId, _Tp &iProp) {
        if (nId <= 0 || nId > _M_metadata._M_size)
            return false;

        uint32_t __offset = _M_metaNode[nId]._M_d_offset;
        uint32_t __flag   = _M_metaNode[nId]._M_d_cnt_flag;
        float*   __value  = (float *)&iProp;
        uint32_t __value_cnt = 0;
        memset(&iProp, 0, sizeof(_Tp));

        while(__flag) {
            if(__flag & 0x01) {
                __value[__value_cnt] = _M_p_data[__offset++];
            } else {
                __value[__value_cnt] = 0;
            }

            __flag = __flag >> 1;
            __value_cnt++;
        }
        return true;
    }

    bool save(const char *sIndexName)
    {
        int64_t i;
        int nFD;
        dbHead __daHead;

        if (_M_metadata._M_size == 0)
            return false;
        fprintf(stderr, "write : %d, magic: %s\n", _M_metadata._M_size, _M_metadata._M_magic);

        nFD = open(sIndexName, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (nFD < 0)
            return false;

        //Write HEADER INFO
        memcpy(&__daHead, &_M_metadata, sizeof(dbHead));
        if (memcmp(__daHead._M_magic, DB_MAGIC, 12))
            fprintf(stderr, "magic err %s\n", __daHead._M_magic);
        else
            fprintf(stderr, "magic good %s\n", __daHead._M_magic);
        __daHead._M_n_prop = (__daHead._M_size > __daHead._M_n_prop) ? __daHead._M_n_prop : __daHead._M_size + 1;

        //HEADER
        i = write(nFD, &__daHead, sizeof(dbHead));
        assert(i == sizeof(dbHead));

        dbNode *__node = (dbNode *) malloc(__daHead._M_n_prop * sizeof(dbNode));
        if (!__node) {
            //fprintf(stderr, "malloc dbNode error. Error No. %d\n", errno);
            return false;
        }

        uint32_t __offset = 0;
        for (i = 0; i < __daHead._M_n_prop; i++) {
            int va_cnt = sizeof(_Tp) / sizeof(int);
            float *value = reinterpret_cast<float *>(&__daHead._M_p_prop[i]);

            va_cnt = va_cnt > 32 ? 32 : va_cnt;
            __node[i]._M_d_offset = __offset;
            __node[i]._M_d_cnt_flag = 0;
            for (int j = 0; j < va_cnt; j++) {
                if (value[j] != 0.0) {
                    __node[i]._M_d_cnt_flag |= (1 << j);
                    __offset++;
                }
            }
        }

        int64_t wr_size = static_cast<int64_t>(__daHead._M_n_prop) * sizeof(dbNode);
        char* pwrtbuf = reinterpret_cast<char *>(__node);
        int64_t wblock_sz = 0;
        for (i = 0; i < wr_size; i += wblock_sz) {
            wblock_sz = write(nFD, pwrtbuf + i, wr_size - i);
            if (wblock_sz < 0) {
                //fprintf(stderr, "Write to file error. Error NO. %d\n", errno);
                break;
            }
        }
        if (i != wr_size) {
            fprintf(stderr, "Write dbnode Error: %ld != %ld\n", i, wr_size);
        }
        free(__node);
        for (i = 0; i < __daHead._M_n_prop; i++) {
            int va_cnt = sizeof(_Tp) / sizeof(int);
            float *value = reinterpret_cast<float *>(&__daHead._M_p_prop[i]);
            for (int j = 0; j < va_cnt; j++) {
                if (value[j] != 0.0) {
                    int64_t wblock_sz = write(nFD, &value[j], sizeof(float));
                    if (wblock_sz < 0) {
                        //fprintf(stderr, "Write to file error. Error NO. %d\n", errno);
                        break;
                    }
                }
            }
        }
        close(nFD);
        clean();
        __static_load(sIndexName);

        return true;
    }

    bool load(const char *sIndexName, bool bRdOnly = true)
    { 
         return __static_load( sIndexName );
    }

};

}

#endif

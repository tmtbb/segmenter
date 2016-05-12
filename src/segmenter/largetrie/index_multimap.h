#ifndef __RELRANK_INDEX_MULTI_MAP_HEADER__
#define __RELRANK_INDEX_MULTI_MAP_HEADER__
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <vector>
#include <map>
#include <algorithm>
#include <queue>
#include <string>
#include <assert.h>
#include "base.h"

namespace Relrank
{
using namespace std;

template <class _Tp>
class IndexMultiMap 
{
public:
	struct dbHead {
#define MULTI_MAGIC	"IndexMultiMapbibo"
		char		_M_magic[18];
		int			_M_size;
        int         _M_data_size;
        int*        _M_offset;
		char*	    _M_data;
	};

public:

	IndexMultiMap()
	{
        _M_n_map        = 0; 
        _M_p_map        = NULL;
        __Initialize();
	}
	
	~IndexMultiMap()
	{
		__clean( ); 
	}

private:
	dbHead      _M_metadata;
	int         _M_n_map;
	char*       _M_p_map;

    map<int, iVal_t<_Tp> > _M_mp;

private:
	Val_t<_Tp> *__getProp(int n)
	{
		if (n >= _M_metadata._M_size || n < 0) return NULL;
        Val_t<_Tp> *pres = reinterpret_cast<Val_t<_Tp>*>(_M_metadata._M_data + _M_metadata._M_offset[n]);
        //printf("find: %d, %d, %p, %d\n", n, _M_metadata._M_size, _M_metadata._M_data, _M_metadata._M_offset[n]);

        return pres;
	}

	void __Initialize()
	{
		memset( &_M_metadata, 0, sizeof( dbHead ) );
		memcpy( _M_metadata._M_magic, MULTI_MAGIC, 18 );
	}

	void __clean( )
	{
		if ( _M_p_map ) {
			munmap( _M_p_map, _M_n_map); 
			_M_p_map = NULL; _M_n_map = 0;
		}
		else {
			if(_M_metadata._M_offset) { free(_M_metadata._M_offset); }
			if(_M_metadata._M_data) { free(_M_metadata._M_data); }
		}
		memset( &_M_metadata, 0, sizeof( dbHead ) );
	}

    int __dynac_load(const char *sIndexName)
    {
		int i,nFD;
		dbHead __daHead;

		nFD = open( sIndexName, O_RDONLY, 0666);
		if( nFD < 0 ) return false;

		//HEADER
		i = read( nFD, &__daHead, sizeof( dbHead ) );
		if( i != sizeof( dbHead ) ) return false;
		if( memcmp( __daHead._M_magic, MULTI_MAGIC, 18 ) ) {
			close( nFD ); return false;
		}

		__clean( );
		__daHead._M_offset = (int *)malloc( sizeof( int ) * __daHead._M_size );
		assert( __daHead._M_offset != NULL );
		//_Tp
		i = read( nFD, __daHead._M_offset, sizeof( int ) * __daHead._M_size );
		if( i != (int)(sizeof( int ) * __daHead._M_size ) ) return false;
        //data
		__daHead._M_data = (char *)malloc( __daHead._M_data_size );
		assert( __daHead._M_data != NULL );
		i = read( nFD, __daHead._M_data, __daHead._M_data_size );
		if( i !=  __daHead._M_data_size ) return false;

		memcpy( &_M_metadata, &__daHead, sizeof( dbHead ) );

		close(nFD);
		return true;
	}
    int __static_load(const char *sIndexName)
	{
		int i,nFD;
		dbHead __daHead;

		nFD = open( sIndexName, O_RDONLY, 0666);
		if( nFD < 0 ) return false;

		//HEADER
		i = read( nFD, &__daHead, sizeof( dbHead ) );
		if( i != sizeof( dbHead ) ) { close(nFD); return false; }
		if( memcmp( __daHead._M_magic, MULTI_MAGIC, 18 ) ) {
			close( nFD ); return false;
		}

        //printf("%s: size: %d, %p, data_size: %d, %p\n",  __daHead._M_magic, __daHead._M_size, __daHead._M_offset, __daHead._M_data_size, __daHead._M_data);
		__clean( );

		_M_n_map = lseek( nFD, 0L, SEEK_END );
		lseek ( nFD, 0L, SEEK_SET);

		_M_p_map = (char *)::mmap( NULL, _M_n_map, PROT_READ, MAP_FILE|MAP_SHARED, nFD, 0);
		if( _M_p_map == MAP_FAILED ) {
			close( nFD ); return false;
		}
		int iStep = 64 * 1024;
		char c[iStep];
		char *p = (char *)_M_p_map;
		for (int i = 0; i < (_M_n_map/iStep); i++) memcpy(c, p + i * iStep, iStep);
		__daHead._M_offset  = (int *) ( _M_p_map + sizeof( dbHead ) );
        __daHead._M_data    = (char *)( _M_p_map + sizeof( dbHead ) + __daHead._M_size * sizeof( int ));

        //printf("%s: size: %d, %p, data_size: %d, %p\n",  __daHead._M_magic, __daHead._M_size, __daHead._M_offset, __daHead._M_data_size, __daHead._M_data);
		memcpy( &_M_metadata, &__daHead, sizeof( dbHead ) );

		close( nFD );
		return true;
	}

    int __calc_node_size(int size)
    {
        Val_t<_Tp> tmp;
        if (size == 0) size = 1;
        int bytenum = sizeof(tmp.size) + sizeof(_Tp) * size;
        int packnum = 4 - bytenum % 4;
        if (packnum == 4) packnum = 0;
        bytenum += packnum;

        return bytenum;
    }

public:

	void clean( ) { __clean(); }

	int size( ) { return _M_metadata._M_size; }	

    bool insert(int key, const _Tp &val)
    {
        typename map<int, iVal_t<_Tp> >::iterator it = _M_mp.find(key);
        if (it == _M_mp.end())
        {
            _M_mp[key] = iVal_t<_Tp>();
            it = _M_mp.find(key);
        }

        it->second.insert(val);
        return true;
    }

    bool set(int key, const _Tp *tval, int len)
    {
        typename map<int, iVal_t<_Tp> >::iterator it = _M_mp.find(key);
        if (it == _M_mp.end())
        {
            _M_mp[key] = iVal_t<_Tp>();
            it = _M_mp.find(key);
        }

        it->second.set(tval, len);
        return true;
    }

    Val_t<_Tp> *get( int nId)
    {
        return __getProp(nId);
    }

    bool build()
    {
        //fprintf(stderr, "sizeof Val_t<_Tp>: %d, sizeof _Tp: %d\n", sizeof(Val_t<_Tp>), sizeof(_Tp));
        typename map<int, iVal_t<_Tp> >::const_iterator it = _M_mp.begin();
        _M_metadata._M_data_size    = sizeof(Val_t<_Tp>);
        _M_metadata._M_size         = 0;
        for (; it != _M_mp.end(); ++ it)
        {
            _M_metadata._M_data_size        += __calc_node_size(it->second.size);
            //_M_metadata._M_data_size        += sizeof(Val_t<_Tp>);
            //if (it->second.size > 1)
            //    _M_metadata._M_data_size    += (it->second.size - 1) * sizeof(_Tp);

            if (it->first > _M_metadata._M_size)  _M_metadata._M_size = it->first;
        }
        _M_metadata._M_size ++;

        if(_M_metadata._M_offset) { free(_M_metadata._M_offset); }
        if(_M_metadata._M_data) { free(_M_metadata._M_data); }
        _M_metadata._M_offset = (int*)malloc(sizeof(int) * _M_metadata._M_size);
        memset(_M_metadata._M_offset, 0, sizeof(int) * _M_metadata._M_size); 
        _M_metadata._M_data = (char*)malloc(_M_metadata._M_data_size);
        memset(_M_metadata._M_data, 0, _M_metadata._M_data_size);

        int offset = 0;
        //memset(_M_metadata._M_data, 0, sizeof(Val_t<_Tp>));
        offset  += sizeof(Val_t<_Tp>);
        for (it = _M_mp.begin(); it != _M_mp.end(); ++ it)
        {
            _M_metadata._M_offset[it->first]    = offset;
            memcpy(_M_metadata._M_data + offset, &(it->second.size), sizeof(it->second.size));
            if (it->second.size > 0)
            {
                memcpy(_M_metadata._M_data + offset + sizeof(it->second.size), it->second.pval, sizeof(_Tp) * it->second.size);
            }
            offset += __calc_node_size(it->second.size);
        }
        //fprintf(stderr, "%d, %d\n", offset, _M_metadata._M_data_size);
        assert(offset == _M_metadata._M_data_size);

        return true;
    }

    bool save(const char *sIndexName)
    {
        int i,nFD;
        dbHead __daHead;

        if( _M_metadata._M_size == 0 ) return false;	

        nFD = open( sIndexName, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if( nFD < 0 ) return false;

        //Write HEADER INFO
        memcpy( &__daHead, &_M_metadata, sizeof( dbHead ) );
        __daHead._M_offset  = 0;
        __daHead._M_data    = 0;

        //HEADER
        i = write( nFD, &__daHead, sizeof( dbHead ) );
        assert( i == sizeof( dbHead ) );
        //_Tp
        i = write( nFD, _M_metadata._M_offset, sizeof( int ) * _M_metadata._M_size );
        assert( (size_t)i == sizeof( int ) * _M_metadata._M_size );
        //data
        i = write( nFD, _M_metadata._M_data, _M_metadata._M_data_size);
        assert( i == _M_metadata._M_data_size);

        close( nFD );

        return true;
    }

    bool load(const char *sIndexName, bool bRdOnly = true)
    {
        if( bRdOnly ) 
            return __static_load( sIndexName );
        else
            return __dynac_load( sIndexName );

    }
};

}

#endif

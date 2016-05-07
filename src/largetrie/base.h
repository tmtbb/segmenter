#ifndef __RELRANK_BASE_HEADER__
#define __RELRANK_BASE_HEADER__
#include <unistd.h>
#include <sys/types.h>

namespace Relrank
{

template <class _Tval>
struct Val_t
{
    int     size;
    _Tval   pval[1];

    int     find(_Tval v)
    {
        int i = 0;
        for (; i < size; i ++)
        {
            if (pval[i] == v) return i;
        }
        return -1;
    }

    Val_t()
        : size(0)
    {}
};

template <class _Tval>
struct iVal_t
{
    int     size;
    _Tval*  pval;

    iVal_t()
        : size(0), pval(NULL)
    {
    }

    int insert(const _Tval &val)
    {
        int i = 0;
        for (i = 0; i < size; i ++)
        {
            if (pval[i] == val) break;
        }

        if (i == size)
        {
            __inc_size();
            pval[size-1]    = val;
        }

        return size;
    }

    int set(const _Tval* tval, int len)
    {
        size    = len;
        pval = (_Tval *)realloc(pval, size*sizeof(_Tval));
        assert( pval != NULL);
        for (int i = 0; i < size; i ++)
            pval[i] = tval[i];

        return size;
    }

	void __inc_size()
    {
        size ++;
        pval = (_Tval *)realloc( pval, size * sizeof(_Tval) );
        assert( pval != NULL );
    }
};

}

#endif

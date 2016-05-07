#ifndef _RELRANK_TRIE_MAP_HEADER_
#define _RELRANK_TRIE_MAP_HEADER_

#include "trie.h"

#define QSS_MAX_ELEM_COUNT      256

#define DICT_FORMAT_DATRIE      1
#define DICT_FORMAT_BUFFERTRIE  2

namespace Relrank
{



        typedef struct 
        {
            int     pos1;
            int     len1;
            int     pos2;
            int     len2;
        } TriePair;

template<class _TP>
class TrieMap
{
    public:
        TrieMap(){}
        virtual ~TrieMap(){}


        //static TrieMap* dynamic_load(const char* filename);
        virtual TrieNode<_TP>* get_root() = 0;

        virtual bool save        (const char* filename) = 0;
        virtual bool load        (const char* filename) = 0;
        virtual int  build       (Trie<_TP> &trie) = 0;
        virtual int  build       () = 0;

        virtual int reserver     (int size) = 0;
        virtual int size         () = 0;
		virtual int get_id(const _TP *pkey, int len) = 0;
        //  return number of keys
        virtual int insert_sorted (const _TP *pkey, int len, int child_size) {return 0;}
        virtual int insert_sorted (const _TP *pkey, int len, int child_size, float value) {return 0;}
        virtual int insert       (const _TP *pkey, int len, int id) = 0;
        virtual int insert       (const _TP *pkey, int id) = 0;
        //  return true if found
        virtual int nfind        (const _TP *pkey, int len) = 0;
        virtual float nget        (const _TP *pkey, int len) = 0;
        virtual int find         (const _TP *pkey) = 0;
        //  return length of maximum match
        virtual int  max_nmatch  (const _TP *pkey, int len, int &id) = 0;
        virtual int  max_match   (const _TP *pkey, int &id) = 0;
        //  return number of key has been found
        virtual int  multi_nmatch(const _TP *pkey, int len, int* pid, int* plen, int size) = 0;
        virtual int  multi_nmatch(const _TP *pkey, int len, float* pval, int* plen, int size) = 0;
        virtual int  multi_nmatch(const wchar_t *pkey, int len, float* pval, int* plen, int size) = 0;
        virtual int  multi_match (const _TP *pkey, int* pid, int* plen, int size) = 0;
        virtual int  suffix_nmatch(const _TP *pkey, int len, const _TP *psuffix, int size, int* pid) = 0;
        virtual int  pairwise_nmatch(const _TP pkey[QSS_MAX_ELEM_COUNT][2], int cnt[2], int pid[], int idx[][2], int size) = 0;

    private:
        
};

}
#endif

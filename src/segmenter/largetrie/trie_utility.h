#ifndef _RELRANK_TRIE_MAP_UTILITY_HEADER_
#define _RELRANK_TRIE_MAP_UTILITY_HEADER_

#include "bufferTrie.h"

namespace Relrank
{

template<class _TP>
class TrieUtility
{
    public:
    static TrieMap<_TP>* dynamic_load(const char* filename)
    {

        Relrank::BufferTrie<_TP>* pbuftrie = new Relrank::BufferTrie<_TP>;
        bool ret = pbuftrie->load(filename);
        if (ret) return reinterpret_cast<TrieMap<_TP>* >(pbuftrie);
        delete pbuftrie;
        pbuftrie = NULL;

        return NULL;
    }
};

}

#endif

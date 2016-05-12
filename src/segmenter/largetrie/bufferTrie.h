#ifndef __RELRANK_BUFFER_TRIE_HEADER__
#define __RELRANK_BUFFER_TRIE_HEADER__
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <assert.h>
#include <queue>
#include "trie.h"
#include "trie_map.h"

namespace Relrank
{

template <class _TP>
class BufferTrie : public TrieMap<_TP>
{
    public:
        template<class _TKEY>
        struct TrieElm
        {
            _TKEY       key;
            union {
                int         offset;
                float       value;
            };
            int         size;
        };

        template<class _TT>
        struct TrieHead {
            #define BUFFER_TRIE_MAGIC "BUFFER__TRIE"
            char		    _M_magic[12];
            char		    _M_case;
            int			    _M_size;
            int			    _M_n_elm;
            TrieElm<_TT>*	_M_p_elm;
        };

        BufferTrie()
        {
            __initialize();
            _M_trie = new Trie<_TP>;
			_M_p_map = 0;
			_M_n_map = 0;
            _offset = 1;
            _cur = 0;
        }

        ~BufferTrie()
        {
            __clean();
            delete  _M_trie;
            _M_trie = NULL;
        }

        TrieNode<_TP>* get_root()
        {
            return _M_trie->get_root();
        }

        bool save        (const char *filename)
        {
            uint64_t i;
            int nFD;
            TrieHead<_TP> __head;

            if (_M_bufTrie._M_size == 0) return false;

            nFD = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0666);
            if (nFD < 0) return false;

            //write header info
            memcpy(&__head, &_M_bufTrie, sizeof(TrieHead<_TP>));
            //write elm
            for (i = _M_bufTrie._M_n_elm - 1; i >= 0; i --) {
                if (_M_bufTrie._M_p_elm[i].key == 0 && _M_bufTrie._M_p_elm[i].offset == 0) break;
            }
            __head._M_n_elm = i + 1;

            //header
            uint64_t wr_size = sizeof(TrieHead<_TP>);
            i = write(nFD, &__head, wr_size);
            if (i != wr_size)
            {
                fprintf(stderr, "Write BufferTrie Header Error: %ld != %ld\n", i, wr_size);
            }
            //elm
            i = 0;
            int64_t write_len = 0;
            wr_size = static_cast<uint64_t>(__head._M_n_elm)*sizeof(TrieElm<_TP>);
            while (i != wr_size) {
                write_len = write(nFD, (uint8_t *) __head._M_p_elm + i, wr_size - i);
                if (write_len < 0) {
                    //fprintf(stderr, "Write BufferTrie elm Data Error, errno: %d\n", errno);
                    break;
                }
                i += write_len;
                if (i > wr_size) {
                    fprintf(stderr, "Write BufferTrie elm Data Error: %ld != %ld\n", i, wr_size);
                    break;
                }
            }

            if (i != wr_size)
            {
                fprintf(stderr, "Write BufferTrie elm Data Error: %ld != %ld\n", i, wr_size);
            }

            close(nFD);

            return true;
        }

        bool load        (const char *filename)
        {
            return __static_load(filename);
        }

        int build       (Trie<_TP> &trie)
        {
            std::queue<pair<TrieNode<_TP>*, int> > q;
            q.push(make_pair(trie.get_root(), 0));
            int offset = 1;
            while (q.size() > 0)
            {
                pair<TrieNode<_TP>*, int> top    = q.front();
                q.pop();
                TrieNode<_TP>* trienode  = top.first;
                int       cur       = top.second;

                __resizeElm(cur+1);
                if (trienode->child.size() == 0)
                {
                    _M_bufTrie._M_p_elm[cur].key    = trienode->ch;
                    _M_bufTrie._M_p_elm[cur].offset = trienode->id;
                    _M_bufTrie._M_p_elm[cur].size   = 0;
                    _M_bufTrie._M_size ++;
                    continue;
                }

                _M_bufTrie._M_p_elm[cur].key    = trienode->ch;
                _M_bufTrie._M_p_elm[cur].offset = offset;
                _M_bufTrie._M_p_elm[cur].size   = trienode->child.size();

                typename std::map<_TP, TrieNode<_TP>* >::iterator it = trienode->child.begin();
                for (; it != trienode->child.end(); ++ it)
                {
                    q.push(make_pair(it->second, offset));
                    offset ++;
                }
            }

            return _M_bufTrie._M_size;
        }

        int build       ()
        {
            return build(*_M_trie);
        }

        int reserver    (int size)
        {
            __resizeElm(size);
            return _M_bufTrie._M_size;
        }

        int size        ()
        {
            return _M_bufTrie._M_size;
        }
		
        int get_id(const _TP *pkey, int len)
        {
            return _M_trie->find(pkey, len);
        }

        /*
        int insert_sorted      (const _TP *pkey, int len, int child_size, int val_size, float *vals)
        {
            _TP key = (len == 0 ? 0 : pkey[len - 1]);
            if (child_size > 0) {
                __resizeElm(_cur + 1);
                _M_bufTrie._M_p_elm[_cur].key    = key;
                _M_bufTrie._M_p_elm[_cur].offset = _offset;
                _M_bufTrie._M_p_elm[_cur].size   = child_size;
                if (val_size > 0) {
                    __resizeElm(_offset + 1);
                    _M_bufTrie._M_p_elm[_offset].key    = 0;
                    _M_bufTrie._M_p_elm[_offset].offset = val_size;
                    _M_bufTrie._M_p_elm[_offset].size   = 0;
                    _cur ++;
                }
                _offset += child_size;
            }
            else if (child_size == 0) {
                if (val_size > 0) {
                    __resizeElm(_cur + 1);
                    _M_bufTrie._M_p_elm[_cur].key    = key;
                    _M_bufTrie._M_p_elm[_cur].offset = val_size;
                    _M_bufTrie._M_p_elm[_cur].size   = 0;
                    _cur ++;
                    int u_size = (val_size * sizeof(float) + sizeof(irieEli<_TP>) - 1) / sizeof(TrieElm<_TP>);
                    __resizeElm(_cur + u_size);
                    float *buf = (float*)&_M_bufTrie._M_p_elm[_cur];
                    for (int i = 0; i < val_size; i ++) {
                        buf[i] = vals[i];
                    }
                    _cur += u_size;
                }
                else {
                    _cur ++;
                }
                _M_bufTrie._M_size ++;
            }
            return _M_bufTrie._M_size;
        }
        */

        int insert_sorted      (const _TP *pkey, int len, int child_size, float value)
        {
            _TP key = (len == 0 ? 0 : pkey[len - 1]);
            if (child_size > 0) {
                __resizeElm(_cur + 1);
                _M_bufTrie._M_p_elm[_cur].key    = key;
                _M_bufTrie._M_p_elm[_cur].offset = _offset;
                _M_bufTrie._M_p_elm[_cur].size   = child_size;
                __resizeElm(_offset + 1);
                _M_bufTrie._M_p_elm[_offset].key    = 0;
                _M_bufTrie._M_p_elm[_offset].value = value;
                _M_bufTrie._M_p_elm[_offset].size   = 0;
                _offset += child_size;
                _cur ++;
            }
            else if (child_size == 0) {
                __resizeElm(_cur + 1);
                _M_bufTrie._M_p_elm[_cur].key    = key;
                _M_bufTrie._M_p_elm[_cur].value = value;
                _M_bufTrie._M_p_elm[_cur].size   = 0;
                _M_bufTrie._M_size ++;
                _cur ++;
            }
            return _M_bufTrie._M_size;
        }

        int insert_sorted      (const _TP *pkey, int len, int child_size)
        {
            _TP key = (len == 0 ? 0 : pkey[len - 1]);
            if (child_size > 0) {
                __resizeElm(_cur + 1);
                _M_bufTrie._M_p_elm[_cur].key    = key;
                _M_bufTrie._M_p_elm[_cur].offset = _offset;
                _M_bufTrie._M_p_elm[_cur].size   = child_size;
                _offset += child_size;
                _cur ++;
            }
            else if (child_size == 0) {
                _M_bufTrie._M_size ++;
                _cur ++;
            }
            return _M_bufTrie._M_size;
        }

        //  return number of keys
        int insert      (const _TP *pkey, int len, int id)
        {
            _M_trie->add(pkey, len, id);
            return _M_trie->size();
        }

        int insert      (const _TP *pkey, int id)
        {
            int len = 0;
            for (; pkey[len] != 0; len ++);
            _M_trie->add(pkey, len, id);
            return _M_trie->size();
        }

        //  return true if found
        int nfind(const _TP *pkey, int len)
        {
            int cur = 0; const _TP* pend = pkey + len;
            for (const _TP* p = pkey; p != pend; p ++)
            {
                if (_M_bufTrie._M_p_elm[cur].size == 0)
                    return -1;
                int istart  = _M_bufTrie._M_p_elm[cur].offset;
                int iend    = _M_bufTrie._M_p_elm[cur].offset + _M_bufTrie._M_p_elm[cur].size - 1;
                int next    = __binary_search(_M_bufTrie._M_p_elm, istart, iend, *p);
                if (next < 0) {  
                    return -1;
                }
                cur         = next;
            }
            if (_M_bufTrie._M_p_elm[cur].size == 0)
            {
                return _M_bufTrie._M_p_elm[cur].offset;
            }
            cur = _M_bufTrie._M_p_elm[cur].offset;
            if (_M_bufTrie._M_p_elm[cur].size == 0)
            {
                return _M_bufTrie._M_p_elm[cur].offset;
            }

            return -1;
        }

        float nget(const _TP *pkey, int len)
        {
            int cur = 0; const _TP* pend = pkey + len;
            for (const _TP* p = pkey; p != pend; p ++)
            {
                if (_M_bufTrie._M_p_elm[cur].size == 0)
                    return -1;
                int istart  = _M_bufTrie._M_p_elm[cur].offset;
                int iend    = _M_bufTrie._M_p_elm[cur].offset + _M_bufTrie._M_p_elm[cur].size - 1;
                int next    = __binary_search(_M_bufTrie._M_p_elm, istart, iend, *p);
                if (next < 0) {  
                    return -1;
                }
                cur         = next;
            }
            if (_M_bufTrie._M_p_elm[cur].size == 0)
            {
                return _M_bufTrie._M_p_elm[cur].value;
            }
            cur = _M_bufTrie._M_p_elm[cur].offset;
            if (_M_bufTrie._M_p_elm[cur].size == 0)
            {
                return _M_bufTrie._M_p_elm[cur].value;
            }

            return -1;
        }

        int find(const _TP *pkey)
        {
            int cur = 0;
            for (const _TP* p = pkey; *p != 0; p ++)
            {
                if (_M_bufTrie._M_p_elm[cur].size == 0)
                    return -1;
                int istart  = _M_bufTrie._M_p_elm[cur].offset;
                int iend    = istart + _M_bufTrie._M_p_elm[cur].size - 1;
                int next    = __binary_search(_M_bufTrie._M_p_elm, istart, iend, *p);
                if (next < 0)   return -1;
                cur         = next;
            }
            cur = _M_bufTrie._M_p_elm[cur].offset;
            if (_M_bufTrie._M_p_elm[cur].key == 0 && _M_bufTrie._M_p_elm[cur].size == 0)
            {
                return _M_bufTrie._M_p_elm[cur].offset;
            }

            return -1;
        }

        //  return length of maximum match
        int  max_nmatch  (const _TP *pkey, int len, int &id)
        {
            int cur = 0; const _TP* pend = pkey + len; int accepted = 0; const _TP* pok = pkey;
            for (const _TP* p = pkey; p != pend; p ++)
            {
                if (_M_bufTrie._M_p_elm[cur].size == 0)
                    return -1;
                int istart  = _M_bufTrie._M_p_elm[cur].offset;
                int iend    = istart + _M_bufTrie._M_p_elm[cur].size - 1;
                int next    = __binary_search(_M_bufTrie._M_p_elm, istart, iend, *p);
                if (next < 0) break;
                cur         = next;
                int pval    = _M_bufTrie._M_p_elm[cur].offset;
                if (_M_bufTrie._M_p_elm[pval].key == 0 && _M_bufTrie._M_p_elm[pval].size == 0)
                {
                    accepted    = pval;
                    pok         = p + 1;
                }
            }
            if (accepted == 0) return 0;

            id  = _M_bufTrie._M_p_elm[accepted].offset;
            return static_cast<int>(pok - pkey);
        }

        int  max_match  (const _TP *pkey, int &id)
        {
            int cur = 0; int accepted = 0; const _TP* pok = pkey;
            for (const _TP* p = pkey; *p != 0; p ++)
            {
                if (_M_bufTrie._M_p_elm[cur].size == 0)
                    return -1;
                int istart  = _M_bufTrie._M_p_elm[cur].offset;
                int iend    = istart + _M_bufTrie._M_p_elm[cur].size - 1;
                int next    = __binary_search(_M_bufTrie._M_p_elm, istart, iend, *p);
                if (next < 0) break;
                cur         = next;
                int pval    = _M_bufTrie._M_p_elm[cur].offset;
                if (_M_bufTrie._M_p_elm[pval].key == 0 && _M_bufTrie._M_p_elm[pval].size == 0)
                {
                    accepted    = pval;
                    pok         = p + 1;
                }
            }
            if (accepted == 0) return 0;

            id  = _M_bufTrie._M_p_elm[accepted].offset;
            return static_cast<int>(pok - pkey);
        }

        int  pairwise_nmatch(const _TP pkey[QSS_MAX_ELEM_COUNT][2], int cnt[2], int pid[], int idx[][2], int size)
        {
            int cur_cnt = 1, next_cnt = 0;
            int cur[size], next[size], cidx[size], nidx[size][2];
            cur[0] = 0;
            int accepted_num = 0;
            for (int i = 0; i < 2; i ++)
            {
                next_cnt = 0;
                for (int j = 0; j < cur_cnt; j ++)
                {
                    if (_M_bufTrie._M_p_elm[cur[j]].size == 0)
                        continue;
                    int istart  = _M_bufTrie._M_p_elm[cur[j]].offset;
                    int iend    = istart + _M_bufTrie._M_p_elm[cur[j]].size - 1;
                    for (int n = 0; n < cnt[i] && next_cnt < size; n ++)
                    {
                        next[next_cnt] = __binary_search(_M_bufTrie._M_p_elm, istart, iend, pkey[n][i]);
                        if (i > 0) 
                        {
                            nidx[next_cnt][0] = cidx[j];
                            nidx[next_cnt][1] = n;
                        }
                        else 
                        {
                            cidx[next_cnt] = n;
                        }
                        if (next[next_cnt] > 0) {
                            //printf("from: %d to %d, key %d\n", cur[j], next[next_cnt], pkey[n][i]);
                            next_cnt ++;
                        }
                    }
                }
                if (next_cnt == 0) break;
                if (i == 2 - 1)
                {
                    for (int i = 0; i < next_cnt && accepted_num < size; i ++)
                    {
                        int pval;
                        if (_M_bufTrie._M_p_elm[next[i]].size == 0)
                            pval = next[i];
                        else
                            pval = _M_bufTrie._M_p_elm[next[i]].offset;
                        if (_M_bufTrie._M_p_elm[pval].size != 0)
                            continue;
                        pid[accepted_num] = _M_bufTrie._M_p_elm[pval].offset;
                        idx[accepted_num][0] = nidx[i][0];
                        idx[accepted_num][1] = nidx[i][1];
                        //ppair[accepted_num] = pair[i];
                        accepted_num ++;
                    }
                }
                else 
                {
                    for (int j = 0; j < next_cnt; j ++)
                    {
                        cur[j] = next[j];
                    }
                    cur_cnt = next_cnt;
                }
            }
            return accepted_num;
        }

        //  return number of key has been found
        int  multi_nmatch(const wchar_t *pkey, int len, float* pval, int* plen, int size)
        {
            int cur = 0; const wchar_t* pend = pkey + len; int accepted = 0; const wchar_t* pok = pkey; int accepted_num = 0;
            for (const wchar_t* p = pkey; p != pend && _M_bufTrie._M_p_elm[cur].size > 0; p ++)
            {
                if (_M_bufTrie._M_p_elm[cur].size == 0)
                    return -1;
                int istart  = _M_bufTrie._M_p_elm[cur].offset;
                int iend    = istart + _M_bufTrie._M_p_elm[cur].size - 1;
                int next    = __binary_search(_M_bufTrie._M_p_elm, istart, iend, (_TP)(*p));
                if (next < 0) break;
                cur         = next;
                int curval;
                if (_M_bufTrie._M_p_elm[cur].size == 0)
                    curval = cur;
                else
                    curval = _M_bufTrie._M_p_elm[cur].offset;
                if (_M_bufTrie._M_p_elm[curval].size == 0)
                {
                    accepted    = curval;
                    pok         = p + 1;
                    if (accepted_num < size)
                    {
                        pval[accepted_num]   = _M_bufTrie._M_p_elm[accepted].value;
                        plen[accepted_num]  = static_cast<int>(pok - pkey);
                        accepted_num ++;
                    }
                }
            }

            return accepted_num;
        }

        //  return number of key has been found
        int  multi_nmatch(const _TP *pkey, int len, float* pval, int* plen, int size)
        {
            int cur = 0; const _TP* pend = pkey + len; int accepted = 0; const _TP* pok = pkey; int accepted_num = 0;
            for (const _TP* p = pkey; p != pend && _M_bufTrie._M_p_elm[cur].size > 0; p ++)
            {
                if (_M_bufTrie._M_p_elm[cur].size == 0)
                    return -1;
                int istart  = _M_bufTrie._M_p_elm[cur].offset;
                int iend    = istart + _M_bufTrie._M_p_elm[cur].size - 1;
                int next    = __binary_search(_M_bufTrie._M_p_elm, istart, iend, *p);
                if (next < 0) break;
                cur         = next;
                int curval;
                if (_M_bufTrie._M_p_elm[cur].size == 0)
                    curval = cur;
                else
                    curval = _M_bufTrie._M_p_elm[cur].offset;
                if (_M_bufTrie._M_p_elm[curval].size == 0)
                {
                    accepted    = curval;
                    pok         = p + 1;
                    if (accepted_num < size)
                    {
                        pval[accepted_num]   = _M_bufTrie._M_p_elm[accepted].value;
                        plen[accepted_num]  = static_cast<int>(pok - pkey);
                        accepted_num ++;
                    }
                }
            }

            return accepted_num;
        }

        //  return number of key has been found
        int  multi_nmatch(const _TP *pkey, int len, int* pid, int* plen, int size)
        {
            int cur = 0; const _TP* pend = pkey + len; int accepted = 0; const _TP* pok = pkey; int accepted_num = 0;
            for (const _TP* p = pkey; p != pend && _M_bufTrie._M_p_elm[cur].size > 0; p ++)
            {
                if (_M_bufTrie._M_p_elm[cur].size == 0)
                    return -1;
                int istart  = _M_bufTrie._M_p_elm[cur].offset;
                int iend    = istart + _M_bufTrie._M_p_elm[cur].size - 1;
                int next    = __binary_search(_M_bufTrie._M_p_elm, istart, iend, *p);
                if (next < 0) break;
                cur         = next;
                int pval;
                if (_M_bufTrie._M_p_elm[cur].size == 0)
                    pval = cur;
                else
                    pval = _M_bufTrie._M_p_elm[cur].offset;
                if (_M_bufTrie._M_p_elm[pval].size == 0)
                {
                    accepted    = pval;
                    pok         = p + 1;
                    if (accepted_num < size)
                    {
                        pid[accepted_num]   = _M_bufTrie._M_p_elm[accepted].offset;
                        plen[accepted_num]  = static_cast<int>(pok - pkey);
                        accepted_num ++;
                    }
                }
            }

            return accepted_num;
        }

        int  multi_match(const _TP *pkey, int* pid, int* plen, int size)
        {
            int cur = 0; int accepted = 0; const _TP* pok = pkey; int accepted_num = 0;
            for (const _TP* p = pkey; *p != 0; p ++)
            {
                if (_M_bufTrie._M_p_elm[cur].size == 0)
                    return -1;
                int istart  = _M_bufTrie._M_p_elm[cur].offset;
                int iend    = istart + _M_bufTrie._M_p_elm[cur].size - 1;
                int next    = __binary_search(_M_bufTrie._M_p_elm, istart, iend, *p);
                if (next < 0) break;
                cur         = next;
                int pval    = _M_bufTrie._M_p_elm[cur].offset;
                if (_M_bufTrie._M_p_elm[pval].key == 0 && _M_bufTrie._M_p_elm[pval].size == 0)
                {
                    accepted    = pval;
                    pok         = p + 1;
                    if (accepted_num < size)
                    {
                        pid[accepted_num]   = _M_bufTrie._M_p_elm[accepted].offset;
                        plen[accepted_num]  = static_cast<int>(pok - pkey);
                        accepted_num ++;
                    }
                }
            }

            return accepted_num;
        }

        int  suffix_nmatch(const _TP *pkey, int len, const _TP *psuffix, int size, int* pid)
        {
            int cur = 0; const _TP* pend = pkey + len;
            for (const _TP* p = pkey; p != pend; p ++)
            {
                if (_M_bufTrie._M_p_elm[cur].size == 0)
                    return -1;
                int istart  = _M_bufTrie._M_p_elm[cur].offset;
                int iend    = istart + _M_bufTrie._M_p_elm[cur].size - 1;
                int next    = __binary_search(_M_bufTrie._M_p_elm, istart, iend, *p);
                if (next < 0)   return 0;
                cur         = next;
            }
            cur = _M_bufTrie._M_p_elm[cur].offset;

            int suf_cur = cur; int acceptnum = 0;
            for (int i = 0; i < size; i ++)
            {
                if (_M_bufTrie._M_p_elm[cur].size == 0)
                    return -1;
                int istart  = _M_bufTrie._M_p_elm[suf_cur].offset;
                int iend    = istart + _M_bufTrie._M_p_elm[suf_cur].size - 1;
                int next    = __binary_search(_M_bufTrie._M_p_elm, istart, iend, psuffix[i]);
                if (next < 0)
                {
                    pid[i]  = -1;
                    continue;
                }
                cur         = next;
                if (_M_bufTrie._M_p_elm[cur].key == 0 && _M_bufTrie._M_p_elm[cur].size == 0)
                {
                    pid[i]  = _M_bufTrie._M_p_elm[cur].offset;
                    acceptnum ++;
                }
            }

            return acceptnum;
        }

    private:
        void    __clean()
        {
            if (_M_p_map) {
                munmap(_M_p_map, _M_n_map);
                _M_p_map = NULL; _M_n_map = 0;
            }
            else {
                if (_M_bufTrie._M_p_elm) { free(_M_bufTrie._M_p_elm); }
            }
            memset(&_M_bufTrie, 0, sizeof(TrieHead<_TP>));
        }

        void    __resizeElm(int nIdx)
        {
            if (nIdx >= _M_bufTrie._M_n_elm) {
                size_t __nsize = ((nIdx + 511) & ~511) * 3 / 2;
                TrieElm<_TP> *p = (TrieElm<_TP> *)realloc(_M_bufTrie._M_p_elm, __nsize * sizeof(TrieElm<_TP>));
                if(p == NULL) {
                    __nsize = (nIdx + 1023) & ~511;
                    p = (TrieElm<_TP> *)realloc(_M_bufTrie._M_p_elm, __nsize * sizeof(TrieElm<_TP>));
                    assert (p != NULL);
                }
                _M_bufTrie._M_p_elm = p;
                memset(_M_bufTrie._M_p_elm + _M_bufTrie._M_n_elm, 0, (__nsize - _M_bufTrie._M_n_elm) * sizeof(TrieElm<_TP>));

                _M_bufTrie._M_n_elm = __nsize;
            }
        }

        int     __binary_search(TrieElm<_TP>* pbuf, int istart, int iend, _TP key)
        {
            if (istart == iend)
            {
                if (pbuf[istart].key == key)    return istart;
                else                            return -1;
            }
            int imid    = (int)(((long)istart + (long)iend) / 2);
            if (pbuf[imid].key == key)   
                return imid;
            else if (pbuf[imid].key > key)
                return __binary_search(pbuf, istart, imid, key);
            else
                return __binary_search(pbuf, imid + 1, iend, key);
        }

        bool     __dynac_load(const char *sIndexName)
        {
            int i, nFD;
            TrieHead<_TP>    __head;

            nFD = open(sIndexName, O_RDONLY, 0666);
            if (nFD < 0) return false;

            //header
            i = read(nFD, &__head, sizeof(TrieHead<_TP>));
            if (i != sizeof(TrieHead<_TP>)) return false;
            if (memcmp(__head._M_magic, BUFFER_TRIE_MAGIC, 12)) {
                close(nFD); return false;
            }
            __clean();
            __head._M_p_elm = (TrieElm<_TP> *)malloc(sizeof(TrieElm<_TP>) * __head._M_n_elm);
            assert(__head._M_p_elm != NULL);
            //elm
            i = read(nFD, __head._M_p_elm, sizeof(TrieElm<_TP>) * __head._M_n_elm);
            if (i != (int)(sizeof(TrieElm<_TP>) * __head._M_n_elm)) return false;

            memcpy(&_M_bufTrie, &__head, sizeof(TrieHead<_TP>));

            close(nFD);
            return true;
        }

        bool     __static_load(const char *sIndexName)
        {
            int i, nFD;
            TrieHead<_TP> __head;

            nFD = open(sIndexName, O_RDONLY, 0666);
            if (nFD < 0) return false;

            i = read(nFD, &__head, sizeof(TrieHead<_TP>));
            if (i != sizeof(TrieHead<_TP>)) { close(nFD); return false; }
            if (memcmp(__head._M_magic, BUFFER_TRIE_MAGIC, 12)) {
                close(nFD); return false;
            }
            __clean();

            _M_n_map = lseek64(nFD, 0L, SEEK_END);
            lseek64(nFD, 0L, SEEK_SET);

            _M_p_map = (char *)::mmap(NULL, _M_n_map, PROT_READ, MAP_FILE|MAP_SHARED, nFD, 0);
            if (_M_p_map == MAP_FAILED) {
                close(nFD); return false;
            }

            int iStep = 64 * 1024;
            char c[iStep];
            char *p = (char *)_M_p_map;
            for (int64_t i = 0; i < (_M_n_map/iStep); i ++) memcpy(c, p + i * iStep, iStep);
            __head._M_p_elm = (TrieElm<_TP> *) (_M_p_map + sizeof(TrieHead<_TP>));

            memcpy(&_M_bufTrie, &__head, sizeof(TrieHead<_TP>));

            close(nFD);
            return true;
        }

        void    __initialize()
        {
            memset(&_M_bufTrie, 0, sizeof(TrieHead<_TP>));
            memcpy(_M_bufTrie._M_magic, BUFFER_TRIE_MAGIC, 12);
        }

        TrieHead<_TP>   _M_bufTrie;
        long            _M_n_map;
        char*           _M_p_map;
        Trie<_TP>*      _M_trie;

        long            _offset;
        long            _cur;
};

}

#endif

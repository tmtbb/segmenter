#ifndef __RELRANK_XBB_TRIE_HEADER__
#define __RELRANK_XBB_TRIE_HEADER__
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <map>
#include <vector>
#include <algorithm>

using namespace std;

namespace Relrank
{

template <class _TKEY>
struct TrieNode
{
        _TKEY       ch;
        int         id;
        TrieNode*   parent;
        map<_TKEY, TrieNode* > child;
        TrieNode()
        {
            ch      = 0;
            id      = -1;
            parent  = NULL;
        }
        int end_val()
        {
            typename map<_TKEY, TrieNode<_TKEY>* >::iterator fit = child.find(0);
            if (fit != child.end())
                return fit->second->id;

            return -1;
        }
        int get_keys(vector<_TKEY> &keys, _TKEY *keymap = NULL)
        {
            int cnt = 0;
            typename map<_TKEY, TrieNode<_TKEY>* >::iterator it = child.begin();
            for (cnt = 0; it != child.end(); ++ it, cnt ++)
                keys.push_back(translate(keymap, it->first));

            sort(keys.begin(), keys.end());

            return cnt;
        }
        int translate(_TKEY* keymap, _TKEY key)
        {
            if (keymap == NULL) return key;
            return keymap[key];
        }
};

template <class _TP>
class Trie
{
    public:
        Trie(bool btrans = false)
        {
            _root       = new TrieNode<_TP>;
            _node_num   = 1;
            _size       = 0;
            _btrans     = btrans;
        };

        ~Trie()
        {
            clear(_root);
            delete _root;
            _root   = NULL;
        };

        TrieNode<_TP>* get_root()
        {
            return _root;
        }

        int  get_node_num()
        {
            return _node_num;
        }
        
        uint32_t size()
        {
            return _size;
        }

        bool  has_transcode()
        {
            return _btrans;
        }

        bool build_transcode(_TP *buf)
        {
            uint64_t sz = 1;
            sz <<= 8*sizeof(_TP);
            for (uint64_t i = 0; i < sz; i ++)
                buf[i] = i;
            if (!_btrans) {
                fprintf(stderr, "will not build transcode\n");
                return false;
            }

            vector<pair<int, _TP> >   tmpvec(_data.size());
            typename map<_TP, int>::iterator it = _data.begin();
            for (; it != _data.end(); ++ it)
                tmpvec.push_back(make_pair(it->second, it->first));

            sort(tmpvec.begin(), tmpvec.end(), greater<pair<int, _TP> >());
            _TP i = 0;
            for (i = 0; static_cast<size_t>(i) < _data.size(); i ++)
            {
                buf[tmpvec[i].second] = i+1;
#ifdef  DEBUG
                //printf("transcode: %d => %d\n", tmpvec[i].second, i+1);
#endif
            }
            for (uint64_t key = 1; key < sz; key ++)
                if (_data.find(key) == _data.end())
                    buf[key]    = i++;

            return true;
        }

        bool clear(TrieNode<_TP>* cur)
        {
            typename map<_TP, TrieNode<_TP>* >::iterator fit = cur->child.begin();
            for (; fit != cur->child.end(); ++ fit)
            {
                clear(fit->second);
                delete fit->second;
            }
            cur->child.clear();

            return true;
        };

        bool add(const _TP *pword, int len, int id)
        {
            TrieNode<_TP>* cur = _root;
            for (int i = 0; i < len+1; i ++)
            {
                typename map<_TP, TrieNode<_TP>* >::iterator fit = cur->child.find(pword[i]);
                if (fit == cur->child.end())
                {
                    cur->child[pword[i]] =  new TrieNode<_TP>;
                    fit = cur->child.find(pword[i]);
                    _key_statistic(pword[i]);
                    _node_num ++;
                }

                fit->second->ch     = pword[i];
                fit->second->parent = cur;
                cur                 = fit->second;
            }
            if (cur->id == -1)  _size ++;
            cur->id   = id;
            if (cur->ch != 0)
            {
                fprintf(stderr, "ch: %d, id:%d \n", cur->ch, cur->id);
                return false;
            }

            return true;
        };

        int find(const _TP *pword, int len)
        {
            TrieNode<_TP>* cur = _root;
            int i = 0;
            for (i = 0; i < len+1; i ++)
            {
                typename map<_TP, TrieNode<_TP>* >::iterator fit = cur->child.find(pword[i]);
                if (fit == cur->child.end())
                    break;
                cur = fit->second;
            }
            return cur->id;
        };

        TrieNode<_TP>* walk(TrieNode<_TP>* cur, _TP ch)
        {
            typename map<_TP, TrieNode<_TP>* >::iterator fit = cur->child.find(ch);
            if (fit == cur->child.end())
                return NULL;

            return fit->second;
        };

    private:
        TrieNode<_TP>*       _root;
        map<_TP, int>   _data;
        int             _node_num;
        uint32_t        _size;
        bool            _btrans;

        void            _key_statistic(_TP key)
        {
            if (_btrans && key > 0)
            {
                typename map<_TP, int>::iterator ffit = _data.find(key);
                if (ffit == _data.end()) 
                    _data[key] = 1;
                else
                    _data[key] = ffit->second + 1;
            }
        }
};

}

#endif

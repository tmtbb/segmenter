#include "dfa.h"
#include <cstdio>
#include <string>
#include <math.h>
#include <fstream>
#include "exception.h"
//for debug
#include <iostream>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

#include "type_const.h"

namespace qss {
namespace segmenter {

using namespace std;

Dfa::Dfa() : mmap_pattern_(false)
        , fd_map_(-1)
        , map_(0)
        , map_size_(0)
        , node_count_(0)
        , node_type_(0)
        , level1_(0)
        , level2_(0) {
}

Dfa::~Dfa() {
  Clear();
}

// init from mmap file of dfa
void Dfa::InitFromMmapFile(const char *dfa_file) {
  if (mmap_pattern_) {
    ClearMmap();
  }
  fd_map_ = open(dfa_file, O_RDONLY, 0666);
  if (fd_map_ == -1) {
    throw RegularExpressionException(string("io exception in Dfa::InitFromMmapFile():\
         open file failed") + dfa_file);
    return;
  }
  UINT32 map_size_in_byte = sizeof(INT32) * map_size_;
  if (map_size_in_byte == 0) {
    map_size_in_byte = lseek(fd_map_, 0L, SEEK_END);
    map_size_ = map_size_in_byte / sizeof(INT32);
    lseek(fd_map_, 0L, SEEK_SET);
  }
  map_ = reinterpret_cast<INT32 *>(mmap(0, map_size_in_byte, PROT_READ, MAP_FILE | MAP_SHARED, fd_map_, 0));
  if (map_ == MAP_FAILED) {
    close(fd_map_);
    throw RegularExpressionException(
        string(
            "io exception in Dfa::InitFromMmapFile():\
        call mmap on map file failed:")
            + dfa_file);
  }
  node_count_ = map_[0];
  node_type_ = map_ + 1;
  level1_ = node_type_ + node_count_;
  level2_ = level1_ + 256 * node_count_;
  mmap_pattern_ = true;
}

// init from serialization file of dfa
void Dfa::InitFromFile(const char *dfa_file) {
  Clear();
  try {
    char buffer[4];
    const INT32 kInt16Size = 2;
    const INT32 kInt32Size = 4;
    ifstream ifs(dfa_file, ifstream::binary);
    //// get size of file
    //ifs.seekg(0, ifstream::end);
    //INT32 file_size = ifs.tellg();
    //ifs.seekg(0);
    // init accept_types_
    if (ifs.bad() || ifs.fail()) {
      throw RegularExpressionException(
          string("io exception in Dfa::InitFromFile(): open file failed"));
    }
    ifs.read(buffer, kInt32Size);
    INT32 size = *(reinterpret_cast<INT32 *>(&(buffer[0])));
    for (INT32 i = 0; i < size; ++i) {
      ifs.read(buffer, kInt32Size);
      accept_types_.insert(*(reinterpret_cast<INT32 *>(&(buffer[0]))));
    }
    // init nodes_
    ifs.read(buffer, kInt32Size);
    size = *(reinterpret_cast<INT32 *>(&(buffer[0])));
    for (INT32 i = 0; i < size; ++i) {
      DfaNode *node = new DfaNode;
      UINT16 symbol;
      INT32 node_id;
      try {
        //ifs.read(buffer, kInt32Size);
        //node->node_id = *(reinterpret_cast<INT32 *>(&(buffer[0])));
        node->node_id = i;
        ifs.read(buffer, kInt32Size);
        node->type = *(reinterpret_cast<INT32 *>(&(buffer[0])));
        ifs.read(buffer, kInt32Size);
        INT32 succeed_count = *(reinterpret_cast<INT32 *>(&(buffer[0])));
        for (INT32 j = 0; j < succeed_count; ++j) {
          ifs.read(buffer, kInt16Size);
          symbol = *(reinterpret_cast<UINT16 *>(&(buffer[0])));
          ifs.read(buffer, kInt32Size);
          node_id = *(reinterpret_cast<INT32 *>(&(buffer[0])));
          node->succeed_nodes[symbol] = node_id;
        }
      } catch (exception &e) {
        delete node;
        node = 0;
        throw RegularExpressionException(
            string("read exception in Dfa::InitFromFile():") +
                e.what());
      }
      if (node)
        nodes_.push_back(node);
    }
    ifs.close();
  } catch (exception &e) {
    throw RegularExpressionException(
        string("io exception in Dfa::InitFromFile():") +
            e.what());
  }
  mmap_pattern_ = false;
}

// store nodes_ and accept_types_ to file;
// file format: the data stored in host order(little-endian on X86 machine)
// accept_types_.size()(INT32)), member of accept_types_(INT32)
// nodes_.size()(INT32),DfaNode[node_id, type, succeed_nodes.size(), succeed_nodes ids], DfaNode
void Dfa::Serialization(const char *dfa_file) {
  if (mmap_pattern_)
    throw RegularExpressionException(
        string(
            "exception in Dfa::Serialization(): the DFA is in mmap pattern"));
  try {
    const INT32 kInt16Size = 2;
    const INT32 kInt32Size = 4;
    ofstream ofs(dfa_file, ofstream::binary);
    INT32 size = accept_types_.size();
    //cout << "accept size:" << size << endl;
    ofs.write(reinterpret_cast<char *>(&size), kInt32Size);
    for (set<INT32>::iterator i = accept_types_.begin();
        i != accept_types_.end(); ++i) {
      //cout << "accept member:" << *i << endl;
      ofs.write(reinterpret_cast<const char *>(&(*i)), kInt32Size);
    }
    size = nodes_.size();
    //cout << "nodes_ size:" << size << endl;
    ofs.write(reinterpret_cast<char *>(&size), kInt32Size);
    for (INT32 i = 0; i < size; ++i) {
      DfaNode &node = *(nodes_[i]);
      //ofs.write(reinterpret_cast<char *>(&(node.node_id)), kInt32Size);
      ofs.write(reinterpret_cast<char *>(&(node.type)), kInt32Size);
      INT32 succeed_count = node.succeed_nodes.size();
      ofs.write(reinterpret_cast<char *>(&succeed_count), kInt32Size);
      for (std::map<UINT16, INT32>::iterator j = node.succeed_nodes.begin();
          j != node.succeed_nodes.end(); ++j) {
        ofs.write(reinterpret_cast<const char *>(&(j->first)), kInt16Size);
        ofs.write(reinterpret_cast<const char *>(&(j->second)), kInt32Size);
      }
    }
    ofs.close();
  } catch (exception &e) {
    throw RegularExpressionException(
        string("io exception in Dfa::Serialization():") +
            e.what());
  }
}

// store the content of dfa to a file(the file will be used for mmap load)
void Dfa::StoreToMmap(const char *dfa_file) {
  try {
    //const INT32 kInt16Size = 2;
    const INT32 kInt32Size = 4;
    if (mmap_pattern_) {
      //write three section into file
      ofstream ofs(dfa_file, ofstream::binary);
      ofs.write(reinterpret_cast<char *>(&node_count_), kInt32Size);
      for (UINT32 i = 0; i < map_size_; ++i) {
        ofs.write(reinterpret_cast<char *>(map_ + i), kInt32Size);
      }
      ofs.close();
    } else {
      // prepare three section: node type section, level-1 section, level-2 section
      INT32 node_count = nodes_.size();
      INT32 *type_section = new INT32[node_count];
      for (int i = 0; i < node_count; ++i)
        type_section[i] = -1;
      INT32 *level1_section = new INT32[node_count * 256];
      for (int i = 0; i < node_count * 256; ++i)
        level1_section[i] = -1;
      std::vector<INT32 *> level2_section;
      INT32 level2_size = level2_section.size();

      // fill content to three sections
      for (INT32 i = 0; i < node_count; ++i) {
        DfaNode &node = *(nodes_[i]);
        type_section[i] = node.type;
        INT32 *level1_block = level1_section + i * 256;
        //INT32 succeed_count = node.succeed_nodes.size();
        for (std::map<UINT16, INT32>::iterator j = node.succeed_nodes.begin();
            j != node.succeed_nodes.end(); ++j) {
          // little-endian as default
          UINT8 character_low = *(reinterpret_cast<const UINT8 *>(&(j->first)));
          UINT8 character_high = *(reinterpret_cast<const UINT8 *>(&(j->first)) + 1);
          if (level1_block[character_high] == -1) {
            INT32 *level2_block = new INT32[256];
            for (int i = 0; i < 256; ++i) {
              level2_block[i] = -1;
            }
            level2_section.push_back(level2_block);
            level1_block[character_high] = level2_size;
            ++level2_size;
          }
          level2_section[level1_block[character_high]][character_low] =
              j->second;
          // debug
          //if (character_low == 115 && character_high == 0 && i == 0) {
          //  cout << "state:" << i <<"\t symbol:" << character_low << "\tNextState:" << j->second <<endl;
          //  cout << "write in to :\tlevel1:" << level1_block[character_high] <<endl;
          //  cout <<"\tlevel2:" << level2_section[level1_block[character_high]][character_low] <<endl;
          //}
        }
      }

      //write three section into file
      ofstream ofs(dfa_file, ofstream::binary);
      ofs.write(reinterpret_cast<char *>(&node_count), kInt32Size);
      for (INT32 i = 0; i < node_count; ++i) {
        ofs.write(reinterpret_cast<char *>(type_section + i), kInt32Size);
      }
      for (INT32 i = 0; i < node_count * 256; ++i) {
        ofs.write(reinterpret_cast<char *>(level1_section + i), kInt32Size);
      }
      for (INT32 i = 0; i < level2_size; ++i) {
        INT32 *level2_block = level2_section[i];
        for (INT32 j = 0; j < 256; ++j) {
          ofs.write(reinterpret_cast<char *>(level2_block + j), kInt32Size);
        }
      }
      ofs.close();

      //release memory
      delete[] type_section;
      delete[] level1_section;
      for (INT32 i = 0; i < level2_size; ++i) {
        delete[] level2_section[i];
        level2_section[i] = 0;
      }
    }
  } catch (exception &e) {
    throw RegularExpressionException(
        string("io exception in Dfa::StoreToMmap():") +
            e.what());
  }
}

void Dfa::Clear() {
  ClearStdMemory();
  ClearMmap();
}

void Dfa::ClearMmap() {
  // unmap
  if (map_) {
    munmap(map_, sizeof(INT32) * map_size_);
    map_ = 0;
    map_size_ = 0;
  }
  if (fd_map_ >= 0)
    close(fd_map_);
  node_count_ = 0;
  mmap_pattern_ = false;
  node_type_ = 0;
  level1_ = 0;
  level2_ = 0;
}

void Dfa::ClearStdMemory() {
  for (vector<DfaNode *>::iterator i = nodes_.begin();
      i != nodes_.end();
      ++i) {
    delete (*i);
  }
  nodes_.clear();
  accept_types_.clear();
  state_exists_.clear();
  ClearIdMap();
}

void Dfa::ClearIdMap() {
  for (map<INT32, std::set<INT32> *>::iterator i = id_map_.begin();
      i != id_map_.end();
      ++i) {
    delete i->second;
  }
  id_map_.clear();
}

void Dfa::InitFromNfa(const Nfa &nfa) {
  if (nfa.nfa_stack_.empty()) {
    throw RegularExpressionException(
        "input Nfa instance is empty while init Dfa");
  }
  Clear();
  set<INT32> dfa_mark; //mark the processed DFA node
  deque<INT32> dfa_queue; //for travel all DFA nodes
  set<INT32> nfa_start_set;
  set<INT32> nfa_move_set;
  // symbol set, store the symbols appear in NFA
  const set<UINT16> &symbols = nfa.symbols_;
  NfaTable *nfa_table = nfa.nfa_stack_.top();
  INT32 start_nfa_node_id = (*nfa_table)[0]->node_id;
  nfa_start_set.insert(start_nfa_node_id);
  INT32 eclosure_size = EpsilonClosure(nfa, &nfa_start_set);
  INT32 node_id = CreateDfaNode(nfa, &nfa_start_set);
  dfa_queue.push_back(node_id);
  /*
   cout << "dfa_queue:[";
   for (deque<INT32>::iterator i = dfa_queue.begin();
   i != dfa_queue.end();
   ++i) {
   cout << *i << ',';
   }
   cout << ']' <<endl;
   */
  while (!dfa_queue.empty()) {
    INT32 start_id = dfa_queue.front();
    if (dfa_mark.find(start_id) == dfa_mark.end()) {
      dfa_mark.insert(start_id);
      set<INT32> *start_set = id_map_[start_id];
      for (set<UINT16>::const_iterator i = symbols.begin();
          i != symbols.end();
          ++i) {
        //cout << "Init(): Move(" <<start_id<<','<< *i <<')'
        //     <<Move(nfa, *start_set, *i, &nfa_move_set)<<endl;
        if (Move(nfa, *start_set, *i, &nfa_move_set) > 0) {
          eclosure_size = EpsilonClosure(nfa, &nfa_move_set);
          INT32 move_id = CreateDfaNode(nfa, &nfa_move_set);
          nodes_[start_id]->succeed_nodes[*i] = move_id; //add translate edge
          // for debug, output
          //cout << "Move(" <<start_id<<"["<<nodes_[start_id]->type<<"],\t"<< *i <<")=\t"<<move_id<<endl;
          if (dfa_mark.find(move_id) == dfa_mark.end()) {
            dfa_queue.push_back(move_id);
          }
        }
      }
    }
    dfa_queue.pop_front();
  }
  // clear memory
  state_exists_.clear();
  ClearIdMap();
  mmap_pattern_ = false;
}

INT32 Dfa::EpsilonClosure(const Nfa &nfa, set<INT32> *nfa_set) {
  const std::vector<NfaNode *> &nfa_nodes = nfa.nfa_nodes_;
  deque<INT32> queue;
  set<INT32>::iterator it = nfa_set->begin();
  while (it != nfa_set->end()) {
    queue.push_back(*it);
    ++it;
  }
  while (queue.size() != 0) {
    /*
     cout << "EpsilonClosure():queue:[";
     for (deque<INT32>::iterator i = queue.begin();
     i != queue.end();
     ++i) {
     cout << *i << ',';
     }
     cout << ']' <<endl;
     */

    INT32 node_id = queue[0];
    const vector<INT32> &succeeds = nfa_nodes[node_id]->succeed_nodes;
    INT32 size = succeeds.size();
    for (INT32 i = 0; i < size; ++i) {
      INT32 succeed_id = succeeds[i];
      if (nfa_nodes[succeed_id]->symbol == 0) {
        // check to avoid infinite loop
        if (nfa_set->find(succeed_id) == nfa_set->end()) {
          nfa_set->insert(succeed_id);
          //cout << "EpsilonClosure():" << succeed_id <<endl;
          queue.push_back(succeed_id);
        }
      }
    }
    queue.pop_front();
  }
  return nfa_set->size();
}

INT32 Dfa::Move(const Nfa &nfa, const std::set<INT32> &start_set,
    UINT16 symbol, std::set<INT32> *move_set) {
  /*
   cout << "Move():start_set:[";
   for (set<INT32>::const_iterator i = start_set.begin();
   i != start_set.end();
   ++i) {
   cout << *i << ',';
   }
   cout << ']' <<endl;
   */
  move_set->clear();
  const std::vector<NfaNode *> &nfa_nodes = nfa.nfa_nodes_;
  set<INT32>::const_iterator it = start_set.begin();
  while (it != start_set.end()) {
    INT32 node_id = *it;
    const vector<INT32> &succeeds = nfa_nodes[node_id]->succeed_nodes;
    INT32 size = succeeds.size();
    for (INT32 i = 0; i < size; ++i) {
      INT32 succeed_id = succeeds[i];
      if (nfa_nodes[succeed_id]->symbol == symbol) {
        // check to avoid infinite loop
        if (move_set->find(succeed_id) == move_set->end()) {
          move_set->insert(succeed_id);
        }
      }
    }
    ++it;
  }
  /*
   cout << "Move():move_set:[";
   for (set<INT32>::iterator i = move_set->begin();
   i != move_set->end();
   ++i) {
   cout << *i << ',';
   }
   cout << ']' <<endl;
   */
  return move_set->size();
}

// convert the nfa_set to be a string,
// use the state_map_ to store the map relationship
INT32 Dfa::CreateDfaNode(const Nfa &nfa, const std::set<INT32> *nfa_set) {
  // check the whether relative DFA node has existed
  string nfa_ids("");
  char buffer[18];
  //INT32 is_final_state = 0; //whether the DFA node should be a final state
  INT32 node_type = -1; // store the DFA node type
  const std::vector<NfaNode *> &nfa_nodes = nfa.nfa_nodes_;
  // the set has sort its members by default; so a resort operation is omitted
  for (set<INT32>::const_iterator i = nfa_set->begin();
      i != nfa_set->end();
      ++i) {
    if (nfa_nodes[*i]->type > 0) {
      node_type = nfa_nodes[*i]->type;
    }
    sprintf(buffer, "%d", *i);
    nfa_ids += buffer;
    nfa_ids += " ";
  }
  map<string, INT32>::iterator it = state_exists_.find(nfa_ids);
  if (it != state_exists_.end()) {
    return it->second; // the DFA node has existed
  }
  DfaNode *dfa_node = new DfaNode;
  dfa_node->node_id = nodes_.size();
  dfa_node->type = node_type;
  nodes_.push_back(dfa_node);
  // update accepte_types_
  if (node_type > 0) {
    accept_types_.insert(node_type);
  }
  // mark the node exists
  state_exists_[nfa_ids] = dfa_node->node_id;
  // add the map relation to id_map_
  set<INT32> *map_set = new set<INT32>(*nfa_set);
  id_map_[dfa_node->node_id] = map_set;
  return dfa_node->node_id;
}

// match until the next character could not reach another state(dfa node)
INT32 Dfa::GreedyMatch(const UINT16 *buffer, INT32 len,
    std::vector<RegularMatchResult> *result) const {
  result->clear();
  if (mmap_pattern_)
    return GreedyMatchInMmapPattern(buffer, len, result);
  else
    return GreedyMatchInStdPattern(buffer, len, result);
}

// match until the next character could not reach another state(dfa node)
INT32 Dfa::LongestAnyMatch(const UINT16 *buffer, INT32 len,
    std::vector<RegularMatchResult> *result) const {
  result->clear();
  if (mmap_pattern_)
    return LongestAnyMatchInMmapPattern(buffer, len, result);
  else
    return LongestAnyMatchInStdPattern(buffer, len, result);
}

// for the sam type, longest match, if diff type output the pre-matched result
INT32 Dfa::LongestAnyMatch(const UINT16 *buffer, INT32 len,
      RegularMatchResult *result_buf, INT32 buf_len) const {
  if (mmap_pattern_)
    return LongestAnyMatchInMmapPattern(buffer, len, result_buf, buf_len);
  else
    return LongestAnyMatchInStdPattern(buffer, len, result_buf, buf_len);                         
}


// match until the next character could not reach another state(dfa node)
INT32 Dfa::GreedyMatchInStdPattern(const UINT16 *buffer, INT32 len,
    std::vector<RegularMatchResult> *result) const {
  //cout << "GreedyMatch() start" <<endl;
  //result->clear();
  if (nodes_.size() == 0)
    throw RegularExpressionException("match using an empty Dfa instance");
  if (nodes_[0]->type > 0)
    throw RegularExpressionException("the match can't stop on the Dfa");
  INT32 start_state = 0;
  INT32 current_state = start_state;
  // store the pre match result
  INT32 pre_match_pos = -1;
  INT32 pre_match_type = -1;
  for (INT32 i = 0; i < len;) {
    //INT32 match_type = -1; //the type matched now; -1 for no match type
    current_state = 0;
    INT32 j = i;
    pre_match_pos = -1;
    pre_match_type = -1;
    while (j <= len) {
      DfaNode *current_node = nodes_[current_state];
      //cout <<"GreedMatch():" << i <<"," << j
      //    <<"\tstate:" << current_state <<"\tsymbol:"<<buffer[j]<< endl;
      //cout <<"\tcurrent_type:" << current_node->type <<endl;
      if (current_node->type > 0) {
        pre_match_pos = j;
        pre_match_type = current_node->type;
      }
      if (j >= len)
        break;
      // test the next state
      UINT16 symbol = buffer[j];
      map<UINT16, INT32>::iterator it =
          current_node->succeed_nodes.find(symbol);
      // break if no path; else go to next state
      if (it == current_node->succeed_nodes.end()) {
        break;
      } else {
        current_state = it->second;
        ++j;
      }
      //cout <<"\tnext_state:" << it->second <<endl;
    }
    if (pre_match_type > 0) {
      RegularMatchResult mr;
      mr.begin = i;
      mr.end = pre_match_pos;
      mr.type = pre_match_type;
      result->push_back(mr);
      i = mr.end;
    } else {
      //i = j + 1;//i=i+1 is right
      ++i;
    }
  }
  return result->size();
}

// match until the next character could not reach another state(dfa node)
INT32 Dfa::LongestAnyMatchInStdPattern(const UINT16 *buffer, INT32 len,
    std::vector<RegularMatchResult> *result) const {
  //cout << "GreedyMatch() start" <<endl;
  //result->clear();
  if (nodes_.size() == 0)
    throw RegularExpressionException("match using an empty Dfa instance");
  if (nodes_[0]->type > 0)
    throw RegularExpressionException("the match can't stop on the Dfa");
  INT32 start_state = 0;
  INT32 current_state = start_state;
  // store the pre match result
  INT32 pre_match_pos = -1;
  INT32 pre_match_type = -1;
  RegularMatchResult mr;
  for (INT32 i = 0; i < len;) {
    //INT32 match_type = -1; //the type matched now; -1 for no match type
    current_state = 0;
    INT32 j = i;
    pre_match_pos = -1;
    pre_match_type = -1;
    while (j <= len) {
      DfaNode *current_node = nodes_[current_state];
      //cout <<"GreedMatch():" << i <<"," << j
      //    <<"\tstate:" << current_state <<"\tsymbol:"<<buffer[j]<< endl;
      //cout <<"\tcurrent_type:" << current_node->type <<endl;
      if (current_node->type > 0) {
        if (pre_match_type > 0 && pre_match_type != current_node->type) {
          mr.begin = i;
          mr.end = pre_match_pos;
          mr.type = pre_match_type;
          result->push_back(mr);
        }
        pre_match_pos = j;
        pre_match_type = current_node->type;
      }
      if (j >= len)
        break;
      // test the next state
      UINT16 symbol = buffer[j];
      map<UINT16, INT32>::iterator it =
          current_node->succeed_nodes.find(symbol);
      // break if no path; else go to next state
      if (it == current_node->succeed_nodes.end()) {
        break;
      } else {
        current_state = it->second;
        ++j;
      }
    }
    if (pre_match_type > 0) {
      mr.begin = i;
      mr.end = pre_match_pos;
      mr.type = pre_match_type;
      result->push_back(mr);
      i = mr.end;
    } else {
      //i = j + 1;//i=i+1 is right
      ++i;
    }
  }
  return result->size();
}


// match until the next character could not reach another state(dfa node)
INT32 Dfa::GreedyMatchInMmapPattern(const UINT16 *buffer, INT32 len,
    std::vector<RegularMatchResult> *result) const {
  //cout << "GreedyMatch() start" <<endl;
  //result->clear();
  if (node_count_ == 0)
    throw RegularExpressionException("match using an empty Dfa instance");
  if (node_type_[0] > 0)
    throw RegularExpressionException("the match can't stop on the Dfa");
  INT32 start_state = 0;
  INT32 current_state = start_state;
  // store the pre match result
  INT32 pre_match_pos = -1;
  INT32 pre_match_type = -1;
  UINT8 character_low = 0;
  UINT8 character_high = 0;
  INT32 *level1_block = 0;
  INT32 next_state = -1;
  RegularMatchResult mr;
  for (INT32 i = 0; i < len;) {
    //INT32 match_type = -1; //the type matched now; -1 for no match type
    current_state = 0;
    INT32 j = i;
    pre_match_pos = -1;
    pre_match_type = -1;
    while (j <= len) {
      //INT32 current_node = current_state;
      level1_block = level1_ + 256 * current_state;
      //cout <<"GreedyMatchInMmapPattern():" << i <<"," << j
      //    <<"\tstate:" << current_state <<"\tsymbol:"<<buffer[j]<< endl;
      //cout <<"\tcurrent_type:" << node_type_[current_state] <<endl;
      if (node_type_[current_state] > 0) {
        pre_match_pos = j;
        pre_match_type = node_type_[current_state];
      }
      if (j >= len)
        break;
      // test the next state
      // UINT16 symbol = buffer[j];
      // little-endian as default
      character_low = *(reinterpret_cast<const UINT8 *>(buffer+j));
      character_high = *(reinterpret_cast<const UINT8 *>(buffer+j) + 1);
      //cout <<"\tcharacter_high:" <<(int)character_high << "\tcharacter_low:" <<(int)character_low <<endl;
      next_state = -1;
      //cout <<"\tlevel1_block[character_high]:" <<level1_block[character_high] <<endl;
      if (level1_block[character_high] >= 0) {
        next_state = *(level2_ + 256 * level1_block[character_high] + character_low);
        //cout <<"\tnext_state:" <<next_state <<endl;
      }
      // break if no path; else go to next state
      if (next_state == -1) {
        break;
      } else {
        current_state = next_state;
        ++j;
      }
    }
    if (pre_match_type > 0) {
      mr.begin = i;
      mr.end = pre_match_pos;
      mr.type = pre_match_type;
      result->push_back(mr);
      i = mr.end;
    } else {
      //i = j + 1;//i=i+1 is right
      ++i;
    }
  }
  return result->size();
}

float Dfa::GetScore(INT32 type, int len) const {
    if ((type & qss::segmenter::k_type_date)
            || type & qss::segmenter::k_type_time
            || type & qss::segmenter::k_type_telephone
            || type & qss::segmenter::k_type_digit_number
            || type & qss::segmenter::k_type_email
            || type & qss::segmenter::k_type_ip
            || type & qss::segmenter::k_type_chinese_numeric_str
            || type & qss::segmenter::k_type_special_word
            || type & qss::segmenter::k_type_chinese_number
            || type & qss::segmenter::k_type_version_number
            || type & qss::segmenter::k_type_percent
       ) {
        return 23.0;
    }
    else if (type & qss::segmenter::k_type_eng_number) {
        return 30 + -2.0 * len;
    }
    return 10.0 + -3.3 * len;
}

INT32 Dfa::MatchInMmapPattern(const wchar_t *buffer, INT32 len, float* pvals, int* plens, int bufsize) const {
    int cnt = 0;
    if (node_count_ == 0)
        throw RegularExpressionException("match using an empty Dfa instance");
    if (node_type_[0] > 0)
        throw RegularExpressionException("the match can't stop on the Dfa");
    INT32 start_state = 0;
    INT32 current_state = start_state;
    INT32 pre_match_pos = -1;
    INT32 pre_match_type = -1;
    UINT8 character_low = 0;
    UINT8 character_high = 0;
    INT32 *level1_block = 0;
    INT32 next_state = -1;
    current_state = 0;
    INT32 j = 0;
    pre_match_pos = -1;
    pre_match_type = -1;
    while (j <= len) {
        level1_block = level1_ + 256 * current_state;
        if (node_type_[current_state] > 0) {
            if (pre_match_type > 0 && pre_match_type != node_type_[current_state] && cnt < bufsize) {
                pvals[cnt] = GetScore(pre_match_type, pre_match_pos);
                plens[cnt] = pre_match_pos;
                cnt ++;
            }
            pre_match_pos = j;
            pre_match_type = node_type_[current_state];
        }
        if (j >= len)
            break;
        character_low = *(reinterpret_cast<const UINT8 *>(buffer+j));
        character_high = *(reinterpret_cast<const UINT8 *>(buffer+j) + 1);
        next_state = -1;
        if (level1_block[character_high] >= 0) {
            next_state = *(level2_ + 256 * level1_block[character_high] + character_low);
        }
        if (next_state == -1) {
            break;
        } else {
            current_state = next_state;
            ++j;
        }
    }
    if (pre_match_type > 0 && cnt < bufsize) {
        pvals[cnt] = GetScore(pre_match_type, pre_match_pos);
        plens[cnt] = pre_match_pos;
        cnt ++;
    }
    return cnt;
}

INT32 Dfa::MatchInMmapPattern(const UINT16 *buffer, INT32 len, float* pvals, int* plens, int bufsize) const {
    int cnt = 0;
    if (node_count_ == 0)
        throw RegularExpressionException("match using an empty Dfa instance");
    if (node_type_[0] > 0)
        throw RegularExpressionException("the match can't stop on the Dfa");
    INT32 start_state = 0;
    INT32 current_state = start_state;
    INT32 pre_match_pos = -1;
    INT32 pre_match_type = -1;
    UINT8 character_low = 0;
    UINT8 character_high = 0;
    INT32 *level1_block = 0;
    INT32 next_state = -1;
    current_state = 0;
    INT32 j = 0;
    pre_match_pos = -1;
    pre_match_type = -1;
    while (j <= len) {
        level1_block = level1_ + 256 * current_state;
        if (node_type_[current_state] > 0) {
            if (pre_match_type > 0 && pre_match_type != node_type_[current_state]) {
                pvals[cnt] = GetScore(pre_match_type, pre_match_pos);
                plens[cnt] = pre_match_pos;
                //std::cout<<cnt<<" "<<plens[cnt]<<std::endl;
                //printf("%p %d %d\n", buffer, cnt, plens[cnt]);
                cnt ++;
                if (cnt >= bufsize)
                    break;
            }
            pre_match_pos = j;
            pre_match_type = node_type_[current_state];
        }
        if (j >= len)
            break;
        character_low = *(reinterpret_cast<const UINT8 *>(buffer+j));
        character_high = *(reinterpret_cast<const UINT8 *>(buffer+j) + 1);
        next_state = -1;
        if (level1_block[character_high] >= 0) {
            next_state = *(level2_ + 256 * level1_block[character_high] + character_low);
        }
        if (next_state == -1) {
            break;
        } else {
            current_state = next_state;
            ++j;
        }
    }
    if (pre_match_type > 0 && cnt < bufsize) {
        pvals[cnt] = GetScore(pre_match_type, pre_match_pos);
        plens[cnt] = pre_match_pos;
        //std::cout<<cnt<<" "<<plens[cnt]<<std::endl;
        //printf("%p %d %d\n", buffer, cnt, plens[cnt]);
        cnt ++;
    }
    return cnt;
}

// match until the next character could not reach another state(dfa node)
INT32 Dfa::LongestAnyMatchInMmapPattern(const UINT16 *buffer, INT32 len,
        std::vector<RegularMatchResult> *result) const {
    //cout << "GreedyMatch() start" <<endl;
    //result->clear();
    if (node_count_ == 0)
        throw RegularExpressionException("match using an empty Dfa instance");
    if (node_type_[0] > 0)
        throw RegularExpressionException("the match can't stop on the Dfa");
    INT32 start_state = 0;
    INT32 current_state = start_state;
    // store the pre match result
    INT32 pre_match_pos = -1;
    INT32 pre_match_type = -1;
    UINT8 character_low = 0;
    UINT8 character_high = 0;
    INT32 *level1_block = 0;
    INT32 next_state = -1;
    RegularMatchResult mr;
    //for (INT32 i = 0; i < len;) {
    for (INT32 i = 0; i < len;) {
        //INT32 match_type = -1; //the type matched now; -1 for no match type
        current_state = 0;
        INT32 j = i;
        pre_match_pos = -1;
        pre_match_type = -1;
        while (j <= len) {
            //DfaNode *current_node = nodes_[current_state];
            //INT32 current_node = current_state;
            level1_block = level1_ + 256 * current_state;
            //cout <<"GreedMatch():" << i <<"," << j
            //    <<"\tstate:" << current_state <<"\tsymbol:"<<buffer[j]<< endl;
            //cout <<"\tcurrent_type:" << current_node->type <<endl;
            if (node_type_[current_state] > 0) {
                if (pre_match_type > 0 && pre_match_type != node_type_[current_state]) {
                    mr.begin = i;
                    mr.end = pre_match_pos;
                    mr.type = pre_match_type;
                    result->push_back(mr);
                }
                pre_match_pos = j;
                pre_match_type = node_type_[current_state];
            }
            if (j >= len)
                break;
            // test the next state
            // little-endian as default
            character_low = *(reinterpret_cast<const UINT8 *>(buffer+j));
            character_high = *(reinterpret_cast<const UINT8 *>(buffer+j) + 1);
            //cout <<"\tcharacter_high:" <<(int)character_high << "\tcharacter_low:" <<(int)character_low <<endl;
            next_state = -1;
            //cout <<"\tlevel1_block[character_high]:" <<level1_block[character_high] <<endl;
            if (level1_block[character_high] >= 0) {
                next_state = *(level2_ + 256 * level1_block[character_high] + character_low);
                //cout <<"\tnext_state:" <<next_state <<endl;
            }
            // break if no path; else go to next state
            if (next_state == -1) {
                break;
            } else {
                current_state = next_state;
                ++j;
            }
        }
        if (pre_match_type > 0) {
            mr.begin = i;
            mr.end = pre_match_pos;
            mr.type = pre_match_type;
            result->push_back(mr);
            i = mr.end;
        } else {
            //i = j + 1;//i=i+1 is right
            ++i;
        }
    }
    return result->size();
}

// match until the next character could not reach another state(dfa node)
INT32 Dfa::LongestAnyMatchInMmapPattern(const UINT16 *buffer, INT32 len,
        RegularMatchResult *result_buf, INT32 buf_len) const {
    //cout << "GreedyMatch() start" <<endl;
    //result->clear();
    if (node_count_ == 0)
        throw RegularExpressionException("match using an empty Dfa instance");
    if (node_type_[0] > 0)
        throw RegularExpressionException("the match can't stop on the Dfa");
    INT32 start_state = 0;
    INT32 current_state = start_state;
    // store the pre match result
    INT32 pre_match_pos = -1;
    INT32 pre_match_type = -1;
    UINT8 character_low = 0;
    UINT8 character_high = 0;
    INT32 *level1_block = 0;
    INT32 next_state = -1;
    //RegularMatchResult mr;
    INT32 buf_offset = 0;
    for (INT32 i = 0; i < len && buf_offset < buf_len;) {
        //INT32 match_type = -1; //the type matched now; -1 for no match type
        current_state = 0;
        INT32 j = i;
        pre_match_pos = -1;
        pre_match_type = -1;
        while (j <= len && buf_offset < buf_len) {
            level1_block = level1_ + 256 * current_state;
            if (node_type_[current_state] > 0) {
                if (pre_match_type > 0 && pre_match_type != node_type_[current_state]) {
                    //mr.begin = i;
                    //mr.end = pre_match_pos;
                    //mr.type = pre_match_type;
                    //result->push_back(mr);
                    result_buf[buf_offset].begin = i;
                    result_buf[buf_offset].end = pre_match_pos;
                    result_buf[buf_offset].type = pre_match_type;
                    ++buf_offset;
                }
                pre_match_pos = j;
                pre_match_type = node_type_[current_state];
            }
            if (j >= len)
                break;
            // test the next state
            // little-endian as default
            character_low = *(reinterpret_cast<const UINT8 *>(buffer+j));
            character_high = *(reinterpret_cast<const UINT8 *>(buffer+j) + 1);
            //cout <<"\tcharacter_high:" <<(int)character_high << "\tcharacter_low:" <<(int)character_low <<endl;
            next_state = -1;
            //cout <<"\tlevel1_block[character_high]:" <<level1_block[character_high] <<endl;
            if (level1_block[character_high] >= 0) {
                next_state = *(level2_ + 256 * level1_block[character_high] + character_low);
                //cout <<"\tnext_state:" <<next_state <<endl;
            }
            // break if no path; else go to next state
            if (next_state == -1) {
                break;
            } else {
                current_state = next_state;
                ++j;
            }
        }
        if (pre_match_type > 0 && buf_offset < buf_len) {
            result_buf[buf_offset].begin = i;
            result_buf[buf_offset].end = pre_match_pos;
            result_buf[buf_offset].type = pre_match_type;
            ++buf_offset;
            i = pre_match_pos;
        } else {
            //i = j + 1;//i=i+1 is right
            ++i;
        }
    }
    return buf_offset;
}

// match until the next character could not reach another state(dfa node)
INT32 Dfa::LongestAnyMatchInStdPattern(const UINT16 *buffer, INT32 len,
        RegularMatchResult *result_buf, INT32 buf_len) const {
    //cout << "GreedyMatch() start" <<endl;
    //result->clear();
    if (nodes_.size() == 0)
        throw RegularExpressionException("match using an empty Dfa instance");
    if (nodes_[0]->type > 0)
        throw RegularExpressionException("the match can't stop on the Dfa");
    INT32 start_state = 0;
    INT32 current_state = start_state;
    // store the pre match result
    INT32 pre_match_pos = -1;
    INT32 pre_match_type = -1;
    //RegularMatchResult mr;
    INT32 buf_offset = 0;
    for (INT32 i = 0; i < len && buf_offset < buf_len;) {
        //INT32 match_type = -1; //the type matched now; -1 for no match type
        current_state = 0;
        INT32 j = i;
        pre_match_pos = -1;
        pre_match_type = -1;
        while (j <= len) {
            DfaNode *current_node = nodes_[current_state];
            //cout <<"GreedMatch():" << i <<"," << j
            //    <<"\tstate:" << current_state <<"\tsymbol:"<<buffer[j]<< endl;
            //cout <<"\tcurrent_type:" << current_node->type <<endl;
            if (current_node->type > 0) {
                if (pre_match_type > 0 && pre_match_type != current_node->type && buf_offset < buf_len) {
                    result_buf[buf_offset].begin = i;
                    result_buf[buf_offset].end = pre_match_pos;
                    result_buf[buf_offset].type = pre_match_type;
                    ++buf_offset;
                }
                pre_match_pos = j;
                pre_match_type = current_node->type;
            }
            if (j >= len)
                break;
            // test the next state
            UINT16 symbol = buffer[j];
            map<UINT16, INT32>::iterator it =
                current_node->succeed_nodes.find(symbol);
            // break if no path; else go to next state
            if (it == current_node->succeed_nodes.end()) {
                break;
            } else {
                current_state = it->second;
                ++j;
            }
        }
        if (pre_match_type > 0 && buf_offset < buf_len) {
            result_buf[buf_offset].begin = i;
            result_buf[buf_offset].end = pre_match_pos;
            result_buf[buf_offset].type = pre_match_type;
            ++buf_offset;
            i = pre_match_pos;
        } else {
            //i = j + 1;//i=i+1 is right
            ++i;
        }
    }
    return buf_offset;
}

INT32 Dfa::AnyMatch(const UINT16 *buffer, INT32 len,
        std::vector<RegularMatchResult> *result) const {
    result->clear();
    if (mmap_pattern_)
        return AnyMatchInMmapPattern(buffer, len, result);
    else
        return AnyMatchInStdPattern(buffer, len, result);
}

// all possible match. use map_
INT32 Dfa::AnyMatchInMmapPattern(const UINT16 *buffer, INT32 len,
        std::vector<RegularMatchResult> *result) const {
    if (node_count_ == 0)
        throw RegularExpressionException("match using an empty Dfa instance");
    if (node_type_[0] > 0)
        throw RegularExpressionException("the match can't stop on the Dfa");
    INT32 start_state = 0;
    INT32 current_state = start_state;
    // store the pre match result
    INT32 pre_match_pos = -1;
    INT32 pre_match_type = -1;
    UINT8 character_low = 0;
    UINT8 character_high = 0;
    INT32 *level1_block = 0;
    INT32 next_state = -1;
    RegularMatchResult mr;
    std::deque<int> start_positions(16);
    int match_start = 0;
    start_positions.push_back(match_start);
    while (!start_positions.empty()) {
        start_state = 0;
        current_state = start_state;
        pre_match_pos = -1;
        pre_match_type = -1;
        character_low = 0;
        character_high = 0;
        level1_block = 0;
        next_state = -1;
        match_start = start_positions.front();
        start_positions.pop_front();
        for (INT32 i = match_start; i < len;) {
            //INT32 match_type = -1; //the type matched now; -1 for no match type
            current_state = 0;
            INT32 j = i;
            pre_match_pos = i-1;
            pre_match_type = -1;
            while (j <= len) {
                //DfaNode *current_node = nodes_[current_state];
                //INT32 current_node = current_state;
                level1_block = level1_ + 256 * current_state;
                //cout <<"GreedMatch():" << i <<"," << j
                //    <<"\tstate:" << current_state <<"\tsymbol:"<<buffer[j]<< endl;
                //cout <<"\tcurrent_type:" << current_node->type <<endl;
                if (node_type_[current_state] > 0) {
                    if (pre_match_type > 0 && pre_match_type != node_type_[current_state]) {
                        mr.begin = i;
                        mr.end = pre_match_pos;
                        mr.type = pre_match_type;
                        result->push_back(mr);
                        start_positions.push_back(pre_match_pos);
                    }
                    pre_match_pos = j;
                    pre_match_type = node_type_[current_state];
                }
                if (j >= len)
                    break;
                // test the next state
                // little-endian as default
                character_low = *(reinterpret_cast<const UINT8 *>(buffer+j));
                character_high = *(reinterpret_cast<const UINT8 *>(buffer+j) + 1);
                //cout <<"\tcharacter_high:" <<(int)character_high << "\tcharacter_low:" <<(int)character_low <<endl;
                next_state = -1;
                //cout <<"\tlevel1_block[character_high]:" <<level1_block[character_high] <<endl;
                if (level1_block[character_high] >= 0) {
                    next_state = *(level2_ + 256 * level1_block[character_high] + character_low);
                    //cout <<"\tnext_state:" <<next_state <<endl;
                }
                // break if no path; else go to next state
                if (next_state == -1) {
                    break;
                } else {
                    current_state = next_state;
                    ++j;
                }
            }
            if (pre_match_type > 0) {
                mr.begin = i;
                mr.end = pre_match_pos;
                mr.type = pre_match_type;
                result->push_back(mr);
                i = mr.end;
            } else {
                //i = j + 1;//i=i+1 is right
                ++i;
            }
        }
    }
    return result->size();  

}

// all possible match. use nodes_
INT32 Dfa::AnyMatchInStdPattern(const UINT16 *buffer, INT32 len,
                       std::vector<RegularMatchResult> *result) const {
  return 0;
}
}
}

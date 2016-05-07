#ifndef __QSS_SEGMENTER_REGULAR_EXPRESSION_TYPES_H__
#define __QSS_SEGMENTER_REGULAR_EXPRESSION_TYPES_H__
#include <deque>
#include <string>
#include <vector>
#include <stack>
#include "common_def.h"

namespace qss {
namespace segmenter {

typedef std::basic_string<UINT16> w16string;

// the type of token in regular expression
enum TokenType {
  kOperator = 0, // the operator in the expression, such as '(', '|','[','<', '{', '?', '*', '-', '+'; 
  kTerminator, // the alphabeta, digital or other symbol which is not operator in the expression
  //kVariable,   // the variable started by '$', such as $Telnumber
  //kEqualMark,  // the '=' in the regular expression, only one per expression
  kUnknown,
};

// one node in an expression
struct TokenNode {
  TokenType type;
  UINT16 symbol; // the content of the node. wchar for the terminator and operator
  TokenNode(TokenType _type, UINT16 _symbol) 
  	: type(_type), symbol(_symbol) {}
};

// the regular expression stored in a queue of TokenNode
// it could be used to store both infix expression and suffix expression
typedef std::deque<TokenNode> TokenizedExpression;
// the stack to store token
// it could be used to convert the infix expression to suffix expression
// it also could be used to convert the suffix expression to be NFA
typedef std::stack<TokenNode> TokenStack;

// store one NFA node 
struct NfaNode {
  INT32 node_id;
  UINT16 symbol; // the unicode value of the edge symbol
  INT32 type; //-1: original node; >=1:final node; 
  std::vector<INT32> succeed_nodes; //the indexes of nodes could be arrived from this node by one edge
};

// store the match result; record the begin offset, end offset, and the type matched
struct RegularMatchResult {
  INT32 begin;
  INT32 end;
  INT32 type;
};

// store one NFA machine, the nodes' instances are stored in nfa_nodes_
// the head is the start node, and the tail is the final node
// there is only one final node for each NfaTable
typedef std::deque<NfaNode *> NfaTable;
}
}
#endif

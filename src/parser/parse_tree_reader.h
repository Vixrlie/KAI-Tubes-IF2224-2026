#ifndef PARSE_TREE_READER_H
#define PARSE_TREE_READER_H

#include <memory>
#include <string>

#include "parse_tree_node.h"

class ParseTreeReader
{
public:
    static bool looksLikeParseTree(const std::string &text);
    static std::unique_ptr<ParseTreeNode> read(const std::string &text);
};

#endif

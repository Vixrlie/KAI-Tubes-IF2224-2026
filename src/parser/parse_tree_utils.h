#ifndef PARSE_TREE_UTILS_H
#define PARSE_TREE_UTILS_H

#include "parse_tree_node.h"

#include <functional>
#include <vector>

namespace ParseTreeUtils
{

    bool isLeaf(const ParseTreeNode *node);

    bool isTerminal(const ParseTreeNode *node);

    std::size_t countChildren(const ParseTreeNode *node);

    ParseTreeNode *getChildAt(ParseTreeNode *node, std::size_t index);

    const ParseTreeNode *getChildAt(const ParseTreeNode *node, std::size_t index);

    int getDepth(const ParseTreeNode *node);

    int getHeight(const ParseTreeNode *node);

    std::vector<ParseTreeNode *> collectAllNodes(ParseTreeNode *node);

    std::vector<const ParseTreeNode *> collectAllNodes(const ParseTreeNode *node);

    std::vector<ParseTreeNode *> collectTerminalNodes(ParseTreeNode *node);

    std::vector<const ParseTreeNode *> collectTerminalNodes(const ParseTreeNode *node);

    std::vector<ParseTreeNode *> collectNodesByName(ParseTreeNode *node, const std::string &name);

    std::vector<const ParseTreeNode *> collectNodesByName(const ParseTreeNode *node, const std::string &name);

    ParseTreeNode *findFirstNode(ParseTreeNode *node, const std::function<bool(ParseTreeNode *)> &predicate);

    const ParseTreeNode *findFirstNode(const ParseTreeNode *node, const std::function<bool(const ParseTreeNode *)> &predicate);

    std::vector<ParseTreeNode *> findAllNodes(ParseTreeNode *node, const std::function<bool(ParseTreeNode *)> &predicate);

    std::vector<const ParseTreeNode *> findAllNodes(const ParseTreeNode *node, const std::function<bool(const ParseTreeNode *)> &predicate);

    int countNodesByName(const ParseTreeNode *node, const std::string &name);

    std::size_t countTotalChildren(const ParseTreeNode *node);

    bool hasChild(const ParseTreeNode *node, const std::string &childName);

}

#endif

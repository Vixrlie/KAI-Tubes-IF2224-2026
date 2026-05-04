#include "parse_tree_utils.h"

#include <algorithm>

// This file provides small traversal helpers so tree queries stay out of parser code.
namespace ParseTreeUtils
{

    // This checks whether a node has no parent and is therefore the tree root.
    bool isRoot(const ParseTreeNode *node)
    {
        return node != nullptr && node->parent() == nullptr;
    }

    // This checks whether a node has no children and therefore ends a branch.
    bool isLeaf(const ParseTreeNode *node)
    {
        if (!node)
            return false;
        return node->children().empty();
    }

    // This checks whether a node is the first child under its parent.
    bool isLeft(const ParseTreeNode *node)
    {
        if (!node || !node->parent())
            return false;

        const auto &siblings = node->parent()->children();
        return !siblings.empty() && siblings.front().get() == node;
    }

    // This checks whether a node is the last child under its parent.
    bool isRight(const ParseTreeNode *node)
    {
        if (!node || !node->parent())
            return false;

        const auto &siblings = node->parent()->children();
        return !siblings.empty() && siblings.back().get() == node;
    }

    // This checks whether a node is a terminal leaf rather than a grammar node.
    bool isTerminal(const ParseTreeNode *node)
    {
        if (!node)
            return false;
        return dynamic_cast<const ParseTreeTerminalNode *>(node) != nullptr;
    }

    // This returns the number of direct children without descending into the subtree.
    std::size_t countChildren(const ParseTreeNode *node)
    {
        if (!node)
            return 0;
        return node->children().size();
    }

    // This safely returns the mutable parent pointer or null when absent.
    ParseTreeNode *getParent(ParseTreeNode *node)
    {
        if (!node)
            return nullptr;
        return node->parent();
    }

    // This safely returns the const parent pointer or null when absent.
    const ParseTreeNode *getParent(const ParseTreeNode *node)
    {
        if (!node)
            return nullptr;
        return node->parent();
    }

    // This safely returns one mutable child by index when it exists.
    ParseTreeNode *getChildAt(ParseTreeNode *node, std::size_t index)
    {
        if (!node || index >= node->children().size())
            return nullptr;
        return node->children()[index].get();
    }

    // This safely returns one const child by index when it exists.
    const ParseTreeNode *getChildAt(const ParseTreeNode *node, std::size_t index)
    {
        if (!node || index >= node->children().size())
            return nullptr;
        return node->children()[index].get();
    }

    // This computes the deepest distance from the node down to any leaf.
    int getDepth(const ParseTreeNode *node)
    {
        if (!node)
            return -1;
        int maxDepth = -1;
        for (const auto &child : node->children())
        {
            int childDepth = getDepth(child.get());
            if (childDepth > maxDepth)
                maxDepth = childDepth;
        }
        return maxDepth + 1;
    }

    // This aliases depth so callers can use the more familiar tree term.
    int getHeight(const ParseTreeNode *node)
    {
        return getDepth(node);
    }

    // This collects every mutable node in the subtree with a preorder walk.
    std::vector<ParseTreeNode *> collectAllNodes(ParseTreeNode *node)
    {
        std::vector<ParseTreeNode *> result;
        if (!node)
            return result;

        result.push_back(node);
        for (auto &child : node->children())
        {
            auto childNodes = collectAllNodes(child.get());
            result.insert(result.end(), childNodes.begin(), childNodes.end());
        }
        return result;
    }

    // This collects every const node in the subtree with a preorder walk.
    std::vector<const ParseTreeNode *> collectAllNodes(const ParseTreeNode *node)
    {
        std::vector<const ParseTreeNode *> result;
        if (!node)
            return result;

        result.push_back(node);
        for (const auto &child : node->children())
        {
            auto childNodes = collectAllNodes(child.get());
            result.insert(result.end(), childNodes.begin(), childNodes.end());
        }
        return result;
    }

    // This collects only mutable terminal nodes from the subtree.
    std::vector<ParseTreeNode *> collectTerminalNodes(ParseTreeNode *node)
    {
        std::vector<ParseTreeNode *> result;
        if (!node)
            return result;

        if (isTerminal(node))
        {
            result.push_back(node);
        }

        for (auto &child : node->children())
        {
            auto childTerminals = collectTerminalNodes(child.get());
            result.insert(result.end(), childTerminals.begin(), childTerminals.end());
        }
        return result;
    }

    // This collects only const terminal nodes from the subtree.
    std::vector<const ParseTreeNode *> collectTerminalNodes(const ParseTreeNode *node)
    {
        std::vector<const ParseTreeNode *> result;
        if (!node)
            return result;

        if (isTerminal(node))
        {
            result.push_back(node);
        }

        for (const auto &child : node->children())
        {
            auto childTerminals = collectTerminalNodes(child.get());
            result.insert(result.end(), childTerminals.begin(), childTerminals.end());
        }
        return result;
    }

    // This returns every mutable node whose printed grammar name matches the query.
    std::vector<ParseTreeNode *> collectNodesByName(ParseTreeNode *node, const std::string &name)
    {
        std::vector<ParseTreeNode *> result;
        if (!node)
            return result;

        if (node->name() == name)
        {
            result.push_back(node);
        }

        for (auto &child : node->children())
        {
            auto childMatches = collectNodesByName(child.get(), name);
            result.insert(result.end(), childMatches.begin(), childMatches.end());
        }
        return result;
    }

    // This returns every const node whose printed grammar name matches the query.
    std::vector<const ParseTreeNode *> collectNodesByName(const ParseTreeNode *node, const std::string &name)
    {
        std::vector<const ParseTreeNode *> result;
        if (!node)
            return result;

        if (node->name() == name)
        {
            result.push_back(node);
        }

        for (const auto &child : node->children())
        {
            auto childMatches = collectNodesByName(child.get(), name);
            result.insert(result.end(), childMatches.begin(), childMatches.end());
        }
        return result;
    }

    // This stops at the first mutable node that satisfies the caller predicate.
    ParseTreeNode *findFirstNode(ParseTreeNode *node, const std::function<bool(ParseTreeNode *)> &predicate)
    {
        if (!node)
            return nullptr;

        if (predicate(node))
            return node;

        for (auto &child : node->children())
        {
            ParseTreeNode *found = findFirstNode(child.get(), predicate);
            if (found)
                return found;
        }
        return nullptr;
    }

    // This stops at the first const node that satisfies the caller predicate.
    const ParseTreeNode *findFirstNode(const ParseTreeNode *node, const std::function<bool(const ParseTreeNode *)> &predicate)
    {
        if (!node)
            return nullptr;

        if (predicate(node))
            return node;

        for (const auto &child : node->children())
        {
            const ParseTreeNode *found = findFirstNode(static_cast<const ParseTreeNode *>(child.get()), predicate);
            if (found)
                return found;
        }
        return nullptr;
    }

    // This collects every mutable node that matches the caller predicate.
    std::vector<ParseTreeNode *> findAllNodes(ParseTreeNode *node, const std::function<bool(ParseTreeNode *)> &predicate)
    {
        std::vector<ParseTreeNode *> result;
        if (!node)
            return result;

        if (predicate(node))
            result.push_back(node);

        for (auto &child : node->children())
        {
            auto childMatches = findAllNodes(child.get(), predicate);
            result.insert(result.end(), childMatches.begin(), childMatches.end());
        }
        return result;
    }

    // This collects every const node that matches the caller predicate.
    std::vector<const ParseTreeNode *> findAllNodes(const ParseTreeNode *node, const std::function<bool(const ParseTreeNode *)> &predicate)
    {
        std::vector<const ParseTreeNode *> result;
        if (!node)
            return result;

        if (predicate(node))
            result.push_back(node);

        for (const auto &child : node->children())
        {
            auto childMatches = findAllNodes(static_cast<const ParseTreeNode *>(child.get()), predicate);
            result.insert(result.end(), childMatches.begin(), childMatches.end());
        }
        return result;
    }

    // This counts how many nodes in the subtree share the same grammar label.
    int countNodesByName(const ParseTreeNode *node, const std::string &name)
    {
        auto matches = collectNodesByName(node, name);
        return static_cast<int>(matches.size());
    }

    // This counts every descendant edge below the node for quick size checks.
    std::size_t countTotalChildren(const ParseTreeNode *node)
    {
        if (!node)
            return 0;
        std::size_t count = node->children().size();
        for (const auto &child : node->children())
        {
            count += countTotalChildren(child.get());
        }
        return count;
    }

    // This checks whether the node has a direct child with the requested label.
    bool hasChild(const ParseTreeNode *node, const std::string &childName)
    {
        if (!node)
            return false;

        for (const auto &child : node->children())
        {
            if (child->name() == childName)
                return true;
        }
        return false;
    }

}

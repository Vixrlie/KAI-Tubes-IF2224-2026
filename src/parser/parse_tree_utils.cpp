#include "parse_tree_utils.h"

#include <algorithm>

namespace ParseTreeUtils
{

    bool isLeaf(const ParseTreeNode *node)
    {
        if (!node)
            return false;
        return node->children().empty();
    }

    bool isTerminal(const ParseTreeNode *node)
    {
        if (!node)
            return false;
        return dynamic_cast<const ParseTreeTerminalNode *>(node) != nullptr;
    }

    std::size_t countChildren(const ParseTreeNode *node)
    {
        if (!node)
            return 0;
        return node->children().size();
    }

    ParseTreeNode *getChildAt(ParseTreeNode *node, std::size_t index)
    {
        if (!node || index >= node->children().size())
            return nullptr;
        return node->children()[index].get();
    }

    const ParseTreeNode *getChildAt(const ParseTreeNode *node, std::size_t index)
    {
        if (!node || index >= node->children().size())
            return nullptr;
        return node->children()[index].get();
    }

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

    int getHeight(const ParseTreeNode *node)
    {
        return getDepth(node);
    }

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

    int countNodesByName(const ParseTreeNode *node, const std::string &name)
    {
        auto matches = collectNodesByName(node, name);
        return static_cast<int>(matches.size());
    }

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

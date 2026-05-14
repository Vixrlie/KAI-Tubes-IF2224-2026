#ifndef AST_FORMATTER_H
#define AST_FORMATTER_H

#include <ostream>
#include <string>

#include "ast_node.h"

// Formats the Decorated AST for human-readable output.
// Displays the tree structure with annotations (type, tab_index, level).

namespace AST
{
    class ASTFormatter
    {
    public:
        // Print the entire decorated AST to the given output stream
        void print(std::ostream &out, const ASTNode *root) const;

    private:
        // Print root's children (special case: no prefix for first level)
        void printRootChildren(std::ostream &out, const ASTNode *root) const;

        // Recursive print with indentation
        void printNode(std::ostream &out, const ASTNode *node,
                       const std::string &prefix, bool isLast) const;

        // Format annotation info for a node
        std::string formatAnnotation(const ASTNode *node) const;

        // Format the node label (type + key info)
        std::string formatLabel(const ASTNode *node) const;

        // Print children of compound/container nodes
        void printChildren(std::ostream &out, const std::vector<std::unique_ptr<ASTNode>> &children,
                           const std::string &prefix) const;

        // Helper to print a single child with proper tree drawing
        void printChild(std::ostream &out, const ASTNode *child,
                        const std::string &prefix, bool isLast) const;
    };

} // namespace AST

#endif

#include "ast_node.h"

// AST node implementations.
// Most logic lives in the header as inline methods since nodes are primarily
// data containers. This file exists for linkage and future extensions.

namespace AST
{
    // Currently all node methods are defined inline in the header.
    // This translation unit ensures the vtables are emitted in one place.
}

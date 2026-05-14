#include "ast_formatter.h"

#include <sstream>

namespace AST
{
    void ASTFormatter::print(std::ostream &out, const ASTNode *root) const
    {
        if (!root)
        {
            out << "(empty AST)" << std::endl;
            return;
        }

        // Print root label
        out << formatLabel(root);
        std::string ann = formatAnnotation(root);
        if (!ann.empty())
        {
            out << "  " << ann;
        }
        out << "\n";

        // Print root's children with tree connectors
        printRootChildren(out, root);
    }

    void ASTFormatter::printRootChildren(std::ostream &out, const ASTNode *root) const
    {
        if (auto *prog = dynamic_cast<const ProgramNode *>(root))
        {
            size_t totalChildren = prog->declarations.size() + (prog->body ? 1 : 0);
            size_t idx = 0;
            for (auto &decl : prog->declarations)
            {
                printNode(out, decl.get(), "", ++idx == totalChildren);
            }
            if (prog->body)
            {
                printNode(out, prog->body.get(), "", true);
            }
        }
        else if (auto *compound = dynamic_cast<const CompoundNode *>(root))
        {
            for (size_t i = 0; i < compound->statements.size(); ++i)
            {
                printNode(out, compound->statements[i].get(), "", i + 1 == compound->statements.size());
            }
        }
    }

    void ASTFormatter::printNode(std::ostream &out, const ASTNode *node,
                                  const std::string &prefix, bool isLast) const
    {
        if (!node) return;

        out << prefix;
        out << (isLast ? "└── " : "├── ");
        out << formatLabel(node);

        std::string ann = formatAnnotation(node);
        if (!ann.empty())
        {
            out << "  " << ann;
        }
        out << "\n";

        std::string childPrefix = prefix + (isLast ? "    " : "│   ");

        // Dispatch to specific node types to print their children
        if (auto *prog = dynamic_cast<const ProgramNode *>(node))
        {
            // Print declarations
            size_t totalChildren = prog->declarations.size() + (prog->body ? 1 : 0);
            size_t idx = 0;
            for (auto &decl : prog->declarations)
            {
                printChild(out, decl.get(), childPrefix, ++idx == totalChildren);
            }
            if (prog->body)
            {
                printChild(out, prog->body.get(), childPrefix, true);
            }
        }
        else if (auto *compound = dynamic_cast<const CompoundNode *>(node))
        {
            printChildren(out, compound->statements, childPrefix);
        }
        else if (auto *varDecl = dynamic_cast<const VarDeclNode *>(node))
        {
            if (varDecl->typeSpec)
            {
                printChild(out, varDecl->typeSpec.get(), childPrefix, true);
            }
        }
        else if (auto *constDecl = dynamic_cast<const ConstDeclNode *>(node))
        {
            if (constDecl->value)
            {
                printChild(out, constDecl->value.get(), childPrefix, true);
            }
        }
        else if (auto *typeDecl = dynamic_cast<const TypeDeclNode *>(node))
        {
            if (typeDecl->typeSpec)
            {
                printChild(out, typeDecl->typeSpec.get(), childPrefix, true);
            }
        }
        else if (auto *arrayType = dynamic_cast<const ArrayTypeNode *>(node))
        {
            if (arrayType->indexType)
            {
                printChild(out, arrayType->indexType.get(), childPrefix, !arrayType->elementType);
            }
            if (arrayType->elementType)
            {
                printChild(out, arrayType->elementType.get(), childPrefix, true);
            }
        }
        else if (auto *recordType = dynamic_cast<const RecordTypeNode *>(node))
        {
            size_t idx = 0;
            for (auto &field : recordType->fields)
            {
                printChild(out, field.get(), childPrefix, ++idx == recordType->fields.size());
            }
        }
        else if (auto *subrange = dynamic_cast<const SubrangeTypeNode *>(node))
        {
            if (subrange->low)
            {
                printChild(out, subrange->low.get(), childPrefix, !subrange->high);
            }
            if (subrange->high)
            {
                printChild(out, subrange->high.get(), childPrefix, true);
            }
        }
        else if (auto *procDecl = dynamic_cast<const ProcedureDeclNode *>(node))
        {
            size_t total = procDecl->params.size() + procDecl->localDecls.size() + (procDecl->body ? 1 : 0);
            size_t idx = 0;
            for (auto &p : procDecl->params)
            {
                printChild(out, p.get(), childPrefix, ++idx == total);
            }
            for (auto &d : procDecl->localDecls)
            {
                printChild(out, d.get(), childPrefix, ++idx == total);
            }
            if (procDecl->body)
            {
                printChild(out, procDecl->body.get(), childPrefix, true);
            }
        }
        else if (auto *funcDecl = dynamic_cast<const FunctionDeclNode *>(node))
        {
            size_t total = funcDecl->params.size() + funcDecl->localDecls.size() + (funcDecl->body ? 1 : 0);
            size_t idx = 0;
            for (auto &p : funcDecl->params)
            {
                printChild(out, p.get(), childPrefix, ++idx == total);
            }
            for (auto &d : funcDecl->localDecls)
            {
                printChild(out, d.get(), childPrefix, ++idx == total);
            }
            if (funcDecl->body)
            {
                printChild(out, funcDecl->body.get(), childPrefix, true);
            }
        }
        else if (auto *assign = dynamic_cast<const AssignNode *>(node))
        {
            if (assign->target)
            {
                printChild(out, assign->target.get(), childPrefix, !assign->value);
            }
            if (assign->value)
            {
                printChild(out, assign->value.get(), childPrefix, true);
            }
        }
        else if (auto *binOp = dynamic_cast<const BinOpNode *>(node))
        {
            if (binOp->left)
            {
                printChild(out, binOp->left.get(), childPrefix, !binOp->right);
            }
            if (binOp->right)
            {
                printChild(out, binOp->right.get(), childPrefix, true);
            }
        }
        else if (auto *unaryOp = dynamic_cast<const UnaryOpNode *>(node))
        {
            if (unaryOp->operand)
            {
                printChild(out, unaryOp->operand.get(), childPrefix, true);
            }
        }
        else if (auto *arrayAccess = dynamic_cast<const ArrayAccessNode *>(node))
        {
            size_t total = (arrayAccess->base ? 1 : 0) + arrayAccess->indices.size();
            size_t idx = 0;
            if (arrayAccess->base)
            {
                printChild(out, arrayAccess->base.get(), childPrefix, ++idx == total);
            }
            for (auto &index : arrayAccess->indices)
            {
                printChild(out, index.get(), childPrefix, ++idx == total);
            }
        }
        else if (auto *fieldAccess = dynamic_cast<const FieldAccessNode *>(node))
        {
            if (fieldAccess->base)
            {
                printChild(out, fieldAccess->base.get(), childPrefix, true);
            }
        }
        else if (auto *procCall = dynamic_cast<const ProcCallNode *>(node))
        {
            printChildren(out, procCall->args, childPrefix);
        }
        else if (auto *funcCall = dynamic_cast<const FuncCallNode *>(node))
        {
            printChildren(out, funcCall->args, childPrefix);
        }
        else if (auto *ifStmt = dynamic_cast<const IfNode *>(node))
        {
            size_t total = (ifStmt->condition ? 1 : 0) + (ifStmt->thenBranch ? 1 : 0) + (ifStmt->elseBranch ? 1 : 0);
            size_t idx = 0;
            if (ifStmt->condition)
            {
                printChild(out, ifStmt->condition.get(), childPrefix, ++idx == total);
            }
            if (ifStmt->thenBranch)
            {
                printChild(out, ifStmt->thenBranch.get(), childPrefix, ++idx == total);
            }
            if (ifStmt->elseBranch)
            {
                printChild(out, ifStmt->elseBranch.get(), childPrefix, true);
            }
        }
        else if (auto *whileStmt = dynamic_cast<const WhileNode *>(node))
        {
            if (whileStmt->condition)
            {
                printChild(out, whileStmt->condition.get(), childPrefix, !whileStmt->body);
            }
            if (whileStmt->body)
            {
                printChild(out, whileStmt->body.get(), childPrefix, true);
            }
        }
        else if (auto *repeatStmt = dynamic_cast<const RepeatNode *>(node))
        {
            size_t total = repeatStmt->statements.size() + (repeatStmt->condition ? 1 : 0);
            size_t idx = 0;
            for (auto &s : repeatStmt->statements)
            {
                printChild(out, s.get(), childPrefix, ++idx == total);
            }
            if (repeatStmt->condition)
            {
                printChild(out, repeatStmt->condition.get(), childPrefix, true);
            }
        }
        else if (auto *forStmt = dynamic_cast<const ForNode *>(node))
        {
            size_t total = (forStmt->startExpr ? 1 : 0) + (forStmt->endExpr ? 1 : 0) + (forStmt->body ? 1 : 0);
            size_t idx = 0;
            if (forStmt->startExpr)
            {
                printChild(out, forStmt->startExpr.get(), childPrefix, ++idx == total);
            }
            if (forStmt->endExpr)
            {
                printChild(out, forStmt->endExpr.get(), childPrefix, ++idx == total);
            }
            if (forStmt->body)
            {
                printChild(out, forStmt->body.get(), childPrefix, true);
            }
        }
        else if (auto *caseStmt = dynamic_cast<const CaseNode *>(node))
        {
            size_t total = (caseStmt->selector ? 1 : 0) + caseStmt->branches.size();
            size_t idx = 0;
            if (caseStmt->selector)
            {
                printChild(out, caseStmt->selector.get(), childPrefix, ++idx == total);
            }
            for (auto &branch : caseStmt->branches)
            {
                printChild(out, branch.get(), childPrefix, ++idx == total);
            }
        }
        else if (auto *caseBranch = dynamic_cast<const CaseBranchNode *>(node))
        {
            size_t total = caseBranch->labels.size() + (caseBranch->body ? 1 : 0);
            size_t idx = 0;
            for (auto &label : caseBranch->labels)
            {
                printChild(out, label.get(), childPrefix, ++idx == total);
            }
            if (caseBranch->body)
            {
                printChild(out, caseBranch->body.get(), childPrefix, true);
            }
        }
        // Leaf nodes (Number, Real, Char, String, Boolean, VarRef, Empty, etc.)
        // have no children to print
    }

    std::string ASTFormatter::formatLabel(const ASTNode *node) const
    {
        if (!node) return "(null)";

        if (auto *prog = dynamic_cast<const ProgramNode *>(node))
        {
            return "Program('" + prog->name + "')";
        }
        if (dynamic_cast<const CompoundNode *>(node))
        {
            return "Block";
        }
        if (auto *varDecl = dynamic_cast<const VarDeclNode *>(node))
        {
            return "VarDecl('" + varDecl->name + "')";
        }
        if (auto *constDecl = dynamic_cast<const ConstDeclNode *>(node))
        {
            return "ConstDecl('" + constDecl->name + "')";
        }
        if (auto *typeDecl = dynamic_cast<const TypeDeclNode *>(node))
        {
            return "TypeDecl('" + typeDecl->name + "')";
        }
        if (auto *named = dynamic_cast<const NamedTypeNode *>(node))
        {
            return "Type('" + named->name + "')";
        }
        if (dynamic_cast<const ArrayTypeNode *>(node))
        {
            return "ArrayType";
        }
        if (dynamic_cast<const RecordTypeNode *>(node))
        {
            return "RecordType";
        }
        if (dynamic_cast<const SubrangeTypeNode *>(node))
        {
            return "Subrange";
        }
        if (auto *enumType = dynamic_cast<const EnumeratedTypeNode *>(node))
        {
            std::string result = "Enumerated(";
            for (size_t i = 0; i < enumType->identifiers.size(); ++i)
            {
                if (i > 0) result += ", ";
                result += enumType->identifiers[i];
            }
            result += ")";
            return result;
        }
        if (auto *procDecl = dynamic_cast<const ProcedureDeclNode *>(node))
        {
            return "ProcedureDecl('" + procDecl->name + "')";
        }
        if (auto *funcDecl = dynamic_cast<const FunctionDeclNode *>(node))
        {
            return "FunctionDecl('" + funcDecl->name + "', returns: '" + funcDecl->returnTypeName + "')";
        }
        if (auto *param = dynamic_cast<const ParamNode *>(node))
        {
            std::string label = "Param('" + param->name + "': " + param->typeName;
            if (param->isVar) label += ", var";
            label += ")";
            return label;
        }
        if (dynamic_cast<const AssignNode *>(node))
        {
            return "Assign";
        }
        if (auto *binOp = dynamic_cast<const BinOpNode *>(node))
        {
            return "BinOp('" + binOp->op + "')";
        }
        if (auto *unaryOp = dynamic_cast<const UnaryOpNode *>(node))
        {
            return "UnaryOp('" + unaryOp->op + "')";
        }
        if (auto *num = dynamic_cast<const NumberNode *>(node))
        {
            return "Num(" + std::to_string(num->value) + ")";
        }
        if (auto *real = dynamic_cast<const RealNode *>(node))
        {
            std::ostringstream oss;
            oss << "Real(" << real->value << ")";
            return oss.str();
        }
        if (auto *ch = dynamic_cast<const CharNode *>(node))
        {
            return std::string("Char('") + ch->value + "')";
        }
        if (auto *str = dynamic_cast<const StringNode *>(node))
        {
            // Strip surrounding quotes if present for cleaner display
            std::string display = str->value;
            if (display.size() >= 2 && display.front() == '\'' && display.back() == '\'')
            {
                display = display.substr(1, display.size() - 2);
            }
            return "String('" + display + "')";
        }
        if (auto *boolNode = dynamic_cast<const BooleanNode *>(node))
        {
            return std::string("Boolean(") + (boolNode->value ? "true" : "false") + ")";
        }
        if (auto *var = dynamic_cast<const VarRefNode *>(node))
        {
            return "Var('" + var->name + "')";
        }
        if (dynamic_cast<const ArrayAccessNode *>(node))
        {
            return "ArrayAccess";
        }
        if (auto *field = dynamic_cast<const FieldAccessNode *>(node))
        {
            return "FieldAccess(." + field->fieldName + ")";
        }
        if (auto *procCall = dynamic_cast<const ProcCallNode *>(node))
        {
            return "ProcCall('" + procCall->name + "')";
        }
        if (auto *funcCall = dynamic_cast<const FuncCallNode *>(node))
        {
            return "FuncCall('" + funcCall->name + "')";
        }
        if (dynamic_cast<const IfNode *>(node))
        {
            return "If";
        }
        if (dynamic_cast<const WhileNode *>(node))
        {
            return "While";
        }
        if (dynamic_cast<const RepeatNode *>(node))
        {
            return "Repeat";
        }
        if (auto *forStmt = dynamic_cast<const ForNode *>(node))
        {
            return std::string("For('") + forStmt->varName + "', " +
                   (forStmt->isDownTo ? "downto" : "to") + ")";
        }
        if (dynamic_cast<const CaseNode *>(node))
        {
            return "Case";
        }
        if (dynamic_cast<const CaseBranchNode *>(node))
        {
            return "CaseBranch";
        }
        if (dynamic_cast<const EmptyNode *>(node))
        {
            return "Empty";
        }

        return node->nodeType();
    }

    std::string ASTFormatter::formatAnnotation(const ASTNode *node) const
    {
        if (!node) return "";

        const Annotation &ann = node->annotation;

        // Only show annotation if something meaningful is set
        if (ann.tabIndex < 0 && ann.typeName.empty() && ann.level < 0)
        {
            return "";
        }

        std::string result = "→ ";
        bool first = true;

        if (ann.tabIndex >= 0)
        {
            result += "tab_index:" + std::to_string(ann.tabIndex);
            first = false;
        }

        if (!ann.typeName.empty())
        {
            if (!first) result += ", ";
            result += "type:" + ann.typeName;
            first = false;
        }

        if (ann.level >= 0)
        {
            if (!first) result += ", ";
            result += "lev:" + std::to_string(ann.level);
        }

        if (ann.blockIndex >= 0)
        {
            result += ", block:" + std::to_string(ann.blockIndex);
        }

        return result;
    }

    void ASTFormatter::printChildren(std::ostream &out,
                                      const std::vector<std::unique_ptr<ASTNode>> &children,
                                      const std::string &prefix) const
    {
        for (size_t i = 0; i < children.size(); ++i)
        {
            printChild(out, children[i].get(), prefix, i + 1 == children.size());
        }
    }

    void ASTFormatter::printChild(std::ostream &out, const ASTNode *child,
                                   const std::string &prefix, bool isLast) const
    {
        printNode(out, child, prefix, isLast);
    }

} // namespace AST

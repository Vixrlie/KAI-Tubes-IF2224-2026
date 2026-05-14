#include "ast_builder.h"

#include <stdexcept>

namespace AST
{
    // Entry point: build AST from parse tree root
    std::unique_ptr<ProgramNode> ASTBuilder::build(const ParseTreeNode *parseTree)
    {
        if (!parseTree || parseTree->name() != "<program>")
        {
            throw std::runtime_error("ASTBuilder: expected <program> root node");
        }
        return visitProgram(parseTree);
    }

    // <program> -> <program-header> <declaration-part> <compound-statement> PERIOD
    std::unique_ptr<ProgramNode> ASTBuilder::visitProgram(const ParseTreeNode *node)
    {
        auto program = std::make_unique<ProgramNode>();

        const ParseTreeNode *header = findChild(node, "<program-header>");
        if (header)
        {
            program->name = visitProgramHeader(header);
        }

        const ParseTreeNode *declPart = findChild(node, "<declaration-part>");
        if (declPart)
        {
            program->declarations = visitDeclarationPart(declPart);
        }

        const ParseTreeNode *compound = findChild(node, "<compound-statement>");
        if (compound)
        {
            program->body = visitCompoundStatement(compound);
        }

        return program;
    }

    // <program-header> -> PROGRAMSY IDENT SEMICOLON
    std::string ASTBuilder::visitProgramHeader(const ParseTreeNode *node)
    {
        auto terminals = getTerminals(node);
        for (auto *term : terminals)
        {
            if (term->token().type == TokenType::IDENT)
            {
                return term->token().value;
            }
        }
        return "";
    }

    // <declaration-part> -> const-decl* type-decl* var-decl* subprogram-decl*
    std::vector<std::unique_ptr<ASTNode>> ASTBuilder::visitDeclarationPart(const ParseTreeNode *node)
    {
        std::vector<std::unique_ptr<ASTNode>> decls;

        for (auto &child : node->children())
        {
            if (child->name() == "<const-declaration>")
            {
                auto constDecls = visitConstDeclaration(child.get());
                for (auto &d : constDecls)
                {
                    decls.push_back(std::move(d));
                }
            }
            else if (child->name() == "<type-declaration>")
            {
                auto typeDecls = visitTypeDeclaration(child.get());
                for (auto &d : typeDecls)
                {
                    decls.push_back(std::move(d));
                }
            }
            else if (child->name() == "<var-declaration>")
            {
                auto varDecls = visitVarDeclaration(child.get());
                for (auto &d : varDecls)
                {
                    decls.push_back(std::move(d));
                }
            }
            else if (child->name() == "<subprogram-declaration>")
            {
                auto subDecl = visitSubprogramDeclaration(child.get());
                if (subDecl)
                {
                    decls.push_back(std::move(subDecl));
                }
            }
        }

        return decls;
    }

    // <const-declaration> -> CONSTSY (IDENT EQL <constant> SEMICOLON)+
    std::vector<std::unique_ptr<ASTNode>> ASTBuilder::visitConstDeclaration(const ParseTreeNode *node)
    {
        std::vector<std::unique_ptr<ASTNode>> decls;
        std::string currentName;

        for (size_t i = 0; i < node->children().size(); ++i)
        {
            const auto &child = node->children()[i];
            const auto *terminal = dynamic_cast<const ParseTreeTerminalNode *>(child.get());

            if (terminal && terminal->token().type == TokenType::IDENT)
            {
                currentName = terminal->token().value;
            }
            else if (child->name() == "<constant>" && !currentName.empty())
            {
                auto constDecl = std::make_unique<ConstDeclNode>();
                constDecl->name = currentName;
                constDecl->value = visitConstant(child.get());
                decls.push_back(std::move(constDecl));
                currentName.clear();
            }
        }

        return decls;
    }

    // <constant> -> [+|-] (IDENT | INTCON | REALCON) | CHARCON | STRING
    std::unique_ptr<ASTNode> ASTBuilder::visitConstant(const ParseTreeNode *node)
    {
        bool negative = false;
        const Token *valueToken = nullptr;

        for (auto &child : node->children())
        {
            const auto *terminal = dynamic_cast<const ParseTreeTerminalNode *>(child.get());
            if (!terminal) continue;

            if (terminal->token().type == TokenType::MINUS)
            {
                negative = true;
            }
            else if (terminal->token().type == TokenType::PLUS)
            {
                negative = false;
            }
            else
            {
                valueToken = &terminal->token();
            }
        }

        if (!valueToken)
        {
            return std::make_unique<NumberNode>();
        }

        switch (valueToken->type)
        {
        case TokenType::INTCON:
        {
            auto num = std::make_unique<NumberNode>();
            num->value = std::stoi(valueToken->value) * (negative ? -1 : 1);
            return num;
        }
        case TokenType::REALCON:
        {
            auto real = std::make_unique<RealNode>();
            real->value = std::stod(valueToken->value) * (negative ? -1.0 : 1.0);
            return real;
        }
        case TokenType::CHARCON:
        {
            auto ch = std::make_unique<CharNode>();
            ch->value = valueToken->value.empty() ? '\0' : valueToken->value[0];
            return ch;
        }
        case TokenType::STRING:
        {
            auto str = std::make_unique<StringNode>();
            str->value = valueToken->value;
            return str;
        }
        case TokenType::IDENT:
        {
            // Handle boolean constant identifiers
            if (valueToken->value == "True" || valueToken->value == "true")
            {
                auto boolNode = std::make_unique<BooleanNode>();
                boolNode->value = true;
                return boolNode;
            }
            if (valueToken->value == "False" || valueToken->value == "false")
            {
                auto boolNode = std::make_unique<BooleanNode>();
                boolNode->value = false;
                return boolNode;
            }
            auto var = std::make_unique<VarRefNode>();
            var->name = valueToken->value;
            return var;
        }
        default:
        {
            auto num = std::make_unique<NumberNode>();
            return num;
        }
        }
    }

    // <type-declaration> -> TYPESY (IDENT EQL <type> SEMICOLON)+
    std::vector<std::unique_ptr<ASTNode>> ASTBuilder::visitTypeDeclaration(const ParseTreeNode *node)
    {
        std::vector<std::unique_ptr<ASTNode>> decls;
        std::string currentName;

        for (size_t i = 0; i < node->children().size(); ++i)
        {
            const auto &child = node->children()[i];
            const auto *terminal = dynamic_cast<const ParseTreeTerminalNode *>(child.get());

            if (terminal && terminal->token().type == TokenType::IDENT)
            {
                currentName = terminal->token().value;
            }
            else if (child->name() == "<type>" && !currentName.empty())
            {
                auto typeDecl = std::make_unique<TypeDeclNode>();
                typeDecl->name = currentName;
                typeDecl->typeSpec = visitType(child.get());
                decls.push_back(std::move(typeDecl));
                currentName.clear();
            }
        }

        return decls;
    }

    // <var-declaration> -> VARSY (identifier-list COLON type SEMICOLON)+
    std::vector<std::unique_ptr<ASTNode>> ASTBuilder::visitVarDeclaration(const ParseTreeNode *node)
    {
        std::vector<std::unique_ptr<ASTNode>> decls;

        // Process groups: each group is identifier-list + type
        std::vector<std::string> currentIdents;
        const ParseTreeNode *currentType = nullptr;

        for (auto &child : node->children())
        {
            if (child->name() == "<identifier-list>")
            {
                currentIdents = visitIdentifierList(child.get());
            }
            else if (child->name() == "<type>")
            {
                currentType = child.get();
            }
            else
            {
                const auto *terminal = dynamic_cast<const ParseTreeTerminalNode *>(child.get());
                if (terminal && terminal->token().type == TokenType::SEMICOLON && currentType)
                {
                    // Emit one VarDeclNode per identifier in the list
                    for (const auto &ident : currentIdents)
                    {
                        auto varDecl = std::make_unique<VarDeclNode>();
                        varDecl->name = ident;
                        varDecl->typeSpec = visitType(currentType);
                        decls.push_back(std::move(varDecl));
                    }
                    currentIdents.clear();
                    currentType = nullptr;
                }
            }
        }

        return decls;
    }

    // <identifier-list> -> IDENT (COMMA IDENT)*
    std::vector<std::string> ASTBuilder::visitIdentifierList(const ParseTreeNode *node)
    {
        std::vector<std::string> idents;
        for (auto &child : node->children())
        {
            const auto *terminal = dynamic_cast<const ParseTreeTerminalNode *>(child.get());
            if (terminal && terminal->token().type == TokenType::IDENT)
            {
                idents.push_back(terminal->token().value);
            }
        }
        return idents;
    }

    // <type> -> <array-type> | <enumerated> | <record-type> | <range> | IDENT
    std::unique_ptr<ASTNode> ASTBuilder::visitType(const ParseTreeNode *node)
    {
        for (auto &child : node->children())
        {
            if (child->name() == "<array-type>")
            {
                return visitArrayType(child.get());
            }
            if (child->name() == "<enumerated>")
            {
                return visitEnumerated(child.get());
            }
            if (child->name() == "<record-type>")
            {
                return visitRecordType(child.get());
            }
            if (child->name() == "<range>")
            {
                return visitRange(child.get());
            }

            const auto *terminal = dynamic_cast<const ParseTreeTerminalNode *>(child.get());
            if (terminal && terminal->token().type == TokenType::IDENT)
            {
                auto named = std::make_unique<NamedTypeNode>();
                named->name = terminal->token().value;
                return named;
            }
        }

        auto unknown = std::make_unique<NamedTypeNode>();
        unknown->name = "unknown";
        return unknown;
    }

    // <array-type> -> ARRAYSY LBRACK (range|IDENT) RBRACK OFSY type
    std::unique_ptr<ArrayTypeNode> ASTBuilder::visitArrayType(const ParseTreeNode *node)
    {
        auto arrayType = std::make_unique<ArrayTypeNode>();

        for (auto &child : node->children())
        {
            if (child->name() == "<range>")
            {
                arrayType->indexType = visitRange(child.get());
            }
            else if (child->name() == "<type>")
            {
                arrayType->elementType = visitType(child.get());
            }
            else
            {
                const auto *terminal = dynamic_cast<const ParseTreeTerminalNode *>(child.get());
                if (terminal && terminal->token().type == TokenType::IDENT && !arrayType->indexType)
                {
                    auto named = std::make_unique<NamedTypeNode>();
                    named->name = terminal->token().value;
                    arrayType->indexType = std::move(named);
                }
            }
        }

        return arrayType;
    }

    // <range> -> constant PERIOD PERIOD constant
    std::unique_ptr<SubrangeTypeNode> ASTBuilder::visitRange(const ParseTreeNode *node)
    {
        auto range = std::make_unique<SubrangeTypeNode>();
        std::vector<const ParseTreeNode *> constants;

        for (auto &child : node->children())
        {
            if (child->name() == "<constant>")
            {
                constants.push_back(child.get());
            }
        }

        if (constants.size() >= 2)
        {
            range->low = visitConstant(constants[0]);
            range->high = visitConstant(constants[1]);
        }

        return range;
    }

    // <enumerated> -> LPARENT IDENT (COMMA IDENT)* RPARENT
    std::unique_ptr<EnumeratedTypeNode> ASTBuilder::visitEnumerated(const ParseTreeNode *node)
    {
        auto enumType = std::make_unique<EnumeratedTypeNode>();

        for (auto &child : node->children())
        {
            const auto *terminal = dynamic_cast<const ParseTreeTerminalNode *>(child.get());
            if (terminal && terminal->token().type == TokenType::IDENT)
            {
                enumType->identifiers.push_back(terminal->token().value);
            }
        }

        return enumType;
    }

    // <record-type> -> RECORDSY <field-list> ENDSY
    std::unique_ptr<RecordTypeNode> ASTBuilder::visitRecordType(const ParseTreeNode *node)
    {
        auto record = std::make_unique<RecordTypeNode>();

        const ParseTreeNode *fieldList = findChild(node, "<field-list>");
        if (fieldList)
        {
            record->fields = visitFieldList(fieldList);
        }

        return record;
    }

    // <field-list> -> <field-part> (SEMICOLON <field-part>)*
    std::vector<std::unique_ptr<VarDeclNode>> ASTBuilder::visitFieldList(const ParseTreeNode *node)
    {
        std::vector<std::unique_ptr<VarDeclNode>> fields;

        for (auto &child : node->children())
        {
            if (child->name() == "<field-part>")
            {
                auto partFields = visitFieldPart(child.get());
                for (auto &f : partFields)
                {
                    fields.push_back(std::move(f));
                }
            }
        }

        return fields;
    }

    // <field-part> -> identifier-list COLON type
    std::vector<std::unique_ptr<VarDeclNode>> ASTBuilder::visitFieldPart(const ParseTreeNode *node)
    {
        std::vector<std::unique_ptr<VarDeclNode>> fields;
        std::vector<std::string> idents;
        const ParseTreeNode *typeNode = nullptr;

        for (auto &child : node->children())
        {
            if (child->name() == "<identifier-list>")
            {
                idents = visitIdentifierList(child.get());
            }
            else if (child->name() == "<type>")
            {
                typeNode = child.get();
            }
        }

        if (typeNode)
        {
            for (const auto &ident : idents)
            {
                auto field = std::make_unique<VarDeclNode>();
                field->name = ident;
                field->typeSpec = visitType(typeNode);
                fields.push_back(std::move(field));
            }
        }

        return fields;
    }

    // <subprogram-declaration> -> <procedure-declaration> | <function-declaration>
    std::unique_ptr<ASTNode> ASTBuilder::visitSubprogramDeclaration(const ParseTreeNode *node)
    {
        for (auto &child : node->children())
        {
            if (child->name() == "<procedure-declaration>")
            {
                return visitProcedureDeclaration(child.get());
            }
            if (child->name() == "<function-declaration>")
            {
                return visitFunctionDeclaration(child.get());
            }
        }
        return nullptr;
    }

    // <procedure-declaration> -> PROCEDURESY IDENT [formal-params] SEMICOLON block SEMICOLON
    std::unique_ptr<ProcedureDeclNode> ASTBuilder::visitProcedureDeclaration(const ParseTreeNode *node)
    {
        auto proc = std::make_unique<ProcedureDeclNode>();

        for (auto &child : node->children())
        {
            const auto *terminal = dynamic_cast<const ParseTreeTerminalNode *>(child.get());
            if (terminal && terminal->token().type == TokenType::IDENT)
            {
                if (proc->name.empty())
                {
                    proc->name = terminal->token().value;
                }
            }
            else if (child->name() == "<formal-parameter-list>")
            {
                proc->params = visitFormalParameterList(child.get());
            }
            else if (child->name() == "<block>")
            {
                // Block contains declaration-part and compound-statement
                const ParseTreeNode *declPart = findChild(child.get(), "<declaration-part>");
                if (declPart)
                {
                    proc->localDecls = visitDeclarationPart(declPart);
                }
                const ParseTreeNode *compound = findChild(child.get(), "<compound-statement>");
                if (compound)
                {
                    proc->body = visitCompoundStatement(compound);
                }
            }
        }

        return proc;
    }

    // <function-declaration> -> FUNCTIONSY IDENT [formal-params] COLON IDENT SEMICOLON block SEMICOLON
    std::unique_ptr<FunctionDeclNode> ASTBuilder::visitFunctionDeclaration(const ParseTreeNode *node)
    {
        auto func = std::make_unique<FunctionDeclNode>();
        bool foundName = false;
        bool foundColon = false;

        for (auto &child : node->children())
        {
            const auto *terminal = dynamic_cast<const ParseTreeTerminalNode *>(child.get());
            if (terminal)
            {
                if (terminal->token().type == TokenType::IDENT)
                {
                    if (!foundName)
                    {
                        func->name = terminal->token().value;
                        foundName = true;
                    }
                    else if (foundColon)
                    {
                        func->returnTypeName = terminal->token().value;
                    }
                }
                else if (terminal->token().type == TokenType::COLON)
                {
                    foundColon = true;
                }
            }
            else if (child->name() == "<formal-parameter-list>")
            {
                func->params = visitFormalParameterList(child.get());
            }
            else if (child->name() == "<block>")
            {
                const ParseTreeNode *declPart = findChild(child.get(), "<declaration-part>");
                if (declPart)
                {
                    func->localDecls = visitDeclarationPart(declPart);
                }
                const ParseTreeNode *compound = findChild(child.get(), "<compound-statement>");
                if (compound)
                {
                    func->body = visitCompoundStatement(compound);
                }
            }
        }

        return func;
    }

    // <formal-parameter-list> -> LPARENT param-group (SEMICOLON param-group)* RPARENT
    std::vector<std::unique_ptr<ParamNode>> ASTBuilder::visitFormalParameterList(const ParseTreeNode *node)
    {
        std::vector<std::unique_ptr<ParamNode>> params;

        for (auto &child : node->children())
        {
            if (child->name() == "<parameter-group>")
            {
                auto groupParams = visitParameterGroup(child.get());
                for (auto &p : groupParams)
                {
                    params.push_back(std::move(p));
                }
            }
        }

        return params;
    }

    // <parameter-group> -> identifier-list COLON (IDENT | array-type)
    std::vector<std::unique_ptr<ParamNode>> ASTBuilder::visitParameterGroup(const ParseTreeNode *node)
    {
        std::vector<std::unique_ptr<ParamNode>> params;
        std::vector<std::string> idents;
        std::string typeName;

        for (auto &child : node->children())
        {
            if (child->name() == "<identifier-list>")
            {
                idents = visitIdentifierList(child.get());
            }
            else
            {
                const auto *terminal = dynamic_cast<const ParseTreeTerminalNode *>(child.get());
                if (terminal && terminal->token().type == TokenType::IDENT)
                {
                    typeName = terminal->token().value;
                }
            }
        }

        for (const auto &ident : idents)
        {
            auto param = std::make_unique<ParamNode>();
            param->name = ident;
            param->typeName = typeName;
            params.push_back(std::move(param));
        }

        return params;
    }

    // <compound-statement> -> BEGINSY <statement-list> ENDSY
    std::unique_ptr<CompoundNode> ASTBuilder::visitCompoundStatement(const ParseTreeNode *node)
    {
        auto compound = std::make_unique<CompoundNode>();

        const ParseTreeNode *stmtList = findChild(node, "<statement-list>");
        if (stmtList)
        {
            compound->statements = visitStatementList(stmtList);
        }

        return compound;
    }

    // <statement-list> -> statement (SEMICOLON statement)*
    std::vector<std::unique_ptr<ASTNode>> ASTBuilder::visitStatementList(const ParseTreeNode *node)
    {
        std::vector<std::unique_ptr<ASTNode>> stmts;

        for (auto &child : node->children())
        {
            if (child->name() == "<statement>")
            {
                auto stmt = visitStatement(child.get());
                if (stmt)
                {
                    stmts.push_back(std::move(stmt));
                }
            }
        }

        return stmts;
    }

    // <statement> -> assignment | if | case | while | repeat | for | proc-call | compound | empty
    std::unique_ptr<ASTNode> ASTBuilder::visitStatement(const ParseTreeNode *node)
    {
        for (auto &child : node->children())
        {
            if (child->name() == "<assignment-statement>")
            {
                return visitAssignmentStatement(child.get());
            }
            if (child->name() == "<if-statement>")
            {
                return visitIfStatement(child.get());
            }
            if (child->name() == "<case-statement>")
            {
                return visitCaseStatement(child.get());
            }
            if (child->name() == "<while-statement>")
            {
                return visitWhileStatement(child.get());
            }
            if (child->name() == "<repeat-statement>")
            {
                return visitRepeatStatement(child.get());
            }
            if (child->name() == "<for-statement>")
            {
                return visitForStatement(child.get());
            }
            if (child->name() == "<procedure/function-call>")
            {
                return visitProcedureFunctionCall(child.get());
            }
            if (child->name() == "<compound-statement>")
            {
                return visitCompoundStatement(child.get());
            }
            if (child->name() == "<empty-statement>")
            {
                return std::make_unique<EmptyNode>();
            }
        }

        return std::make_unique<EmptyNode>();
    }

    // <variable> -> IDENT | <component-variable>
    std::unique_ptr<ASTNode> ASTBuilder::visitVariable(const ParseTreeNode *node)
    {
        // Check if this variable has a component-variable child (array/field access)
        const ParseTreeNode *compVar = findChild(node, "<component-variable>");
        if (compVar)
        {
            // Component variable wraps a base variable + access
            const ParseTreeNode *baseVar = findChild(compVar, "<variable>");
            std::unique_ptr<ASTNode> base;

            if (baseVar)
            {
                base = visitVariable(baseVar);
            }
            else
            {
                // Fallback: look for IDENT in the component variable
                auto terminals = getTerminals(compVar);
                for (auto *t : terminals)
                {
                    if (t->token().type == TokenType::IDENT)
                    {
                        auto var = std::make_unique<VarRefNode>();
                        var->name = t->token().value;
                        base = std::move(var);
                        break;
                    }
                }
            }

            // Determine if it's array access or field access
            const ParseTreeNode *indexList = findChild(compVar, "<index-list>");
            if (indexList)
            {
                auto access = std::make_unique<ArrayAccessNode>();
                access->base = std::move(base);

                // Collect index expressions from index-list
                for (auto &indexChild : indexList->children())
                {
                    const auto *terminal = dynamic_cast<const ParseTreeTerminalNode *>(indexChild.get());
                    if (terminal && terminal->token().type != TokenType::COMMA)
                    {
                        if (terminal->token().type == TokenType::INTCON)
                        {
                            auto num = std::make_unique<NumberNode>();
                            num->value = std::stoi(terminal->token().value);
                            access->indices.push_back(std::move(num));
                        }
                        else if (terminal->token().type == TokenType::CHARCON)
                        {
                            auto ch = std::make_unique<CharNode>();
                            ch->value = terminal->token().value.empty() ? '\0' : terminal->token().value[0];
                            access->indices.push_back(std::move(ch));
                        }
                        else if (terminal->token().type == TokenType::IDENT)
                        {
                            auto var = std::make_unique<VarRefNode>();
                            var->name = terminal->token().value;
                            access->indices.push_back(std::move(var));
                        }
                    }
                }

                return access;
            }
            else
            {
                // Field access: look for PERIOD followed by IDENT
                auto access = std::make_unique<FieldAccessNode>();
                access->base = std::move(base);

                auto terminals = getTerminals(compVar);
                bool foundPeriod = false;
                for (auto *t : terminals)
                {
                    if (t->token().type == TokenType::PERIOD)
                    {
                        foundPeriod = true;
                    }
                    else if (foundPeriod && t->token().type == TokenType::IDENT)
                    {
                        access->fieldName = t->token().value;
                        break;
                    }
                }

                return access;
            }
        }

        // Simple variable: just an IDENT
        auto terminals = getTerminals(node);
        for (auto *t : terminals)
        {
            if (t->token().type == TokenType::IDENT)
            {
                auto var = std::make_unique<VarRefNode>();
                var->name = t->token().value;
                return var;
            }
        }

        auto var = std::make_unique<VarRefNode>();
        var->name = "";
        return var;
    }

    // <assignment-statement> -> variable BECOMES expression
    std::unique_ptr<AssignNode> ASTBuilder::visitAssignmentStatement(const ParseTreeNode *node)
    {
        auto assign = std::make_unique<AssignNode>();

        const ParseTreeNode *varNode = findChild(node, "<variable>");
        if (varNode)
        {
            assign->target = visitVariable(varNode);
        }

        const ParseTreeNode *exprNode = findChild(node, "<expression>");
        if (exprNode)
        {
            assign->value = visitExpression(exprNode);
        }

        return assign;
    }

    // <if-statement> -> IFSY expression THENSY statement [ELSESY statement]
    std::unique_ptr<IfNode> ASTBuilder::visitIfStatement(const ParseTreeNode *node)
    {
        auto ifNode = std::make_unique<IfNode>();
        const ParseTreeNode *expr = findChild(node, "<expression>");
        if (expr)
        {
            ifNode->condition = visitExpression(expr);
        }

        // Find statement children (then-branch and optional else-branch)
        auto stmts = findChildren(node, "<statement>");
        if (stmts.size() >= 1)
        {
            ifNode->thenBranch = visitStatement(stmts[0]);
        }
        if (stmts.size() >= 2)
        {
            ifNode->elseBranch = visitStatement(stmts[1]);
        }

        return ifNode;
    }

    // <case-statement> -> CASESY expression OFSY case-block ENDSY
    std::unique_ptr<CaseNode> ASTBuilder::visitCaseStatement(const ParseTreeNode *node)
    {
        auto caseNode = std::make_unique<CaseNode>();

        const ParseTreeNode *expr = findChild(node, "<expression>");
        if (expr)
        {
            caseNode->selector = visitExpression(expr);
        }

        const ParseTreeNode *block = findChild(node, "<case-block>");
        if (block)
        {
            caseNode->branches = visitCaseBlock(block);
        }

        return caseNode;
    }

    // <case-block> -> constant (COMMA constant)* COLON statement [SEMICOLON case-block]
    std::vector<std::unique_ptr<CaseBranchNode>> ASTBuilder::visitCaseBlock(const ParseTreeNode *node)
    {
        std::vector<std::unique_ptr<CaseBranchNode>> branches;
        auto branch = std::make_unique<CaseBranchNode>();

        for (auto &child : node->children())
        {
            if (child->name() == "<constant>")
            {
                branch->labels.push_back(visitConstant(child.get()));
            }
            else if (child->name() == "<statement>")
            {
                branch->body = visitStatement(child.get());
            }
            else if (child->name() == "<case-block>")
            {
                // Recursive case blocks
                auto moreBranches = visitCaseBlock(child.get());
                branches.push_back(std::move(branch));
                for (auto &b : moreBranches)
                {
                    branches.push_back(std::move(b));
                }
                return branches;
            }
        }

        branches.push_back(std::move(branch));
        return branches;
    }

    // <while-statement> -> WHILESY expression DOSY compound-statement
    std::unique_ptr<WhileNode> ASTBuilder::visitWhileStatement(const ParseTreeNode *node)
    {
        auto whileNode = std::make_unique<WhileNode>();

        const ParseTreeNode *expr = findChild(node, "<expression>");
        if (expr)
        {
            whileNode->condition = visitExpression(expr);
        }

        const ParseTreeNode *compound = findChild(node, "<compound-statement>");
        if (compound)
        {
            whileNode->body = visitCompoundStatement(compound);
        }

        return whileNode;
    }

    // <repeat-statement> -> REPEATSY statement-list UNTILSY expression
    std::unique_ptr<RepeatNode> ASTBuilder::visitRepeatStatement(const ParseTreeNode *node)
    {
        auto repeatNode = std::make_unique<RepeatNode>();

        const ParseTreeNode *stmtList = findChild(node, "<statement-list>");
        if (stmtList)
        {
            repeatNode->statements = visitStatementList(stmtList);
        }

        const ParseTreeNode *expr = findChild(node, "<expression>");
        if (expr)
        {
            repeatNode->condition = visitExpression(expr);
        }

        return repeatNode;
    }

    // <for-statement> -> FORSY IDENT BECOMES expr (TOSY|DOWNTOSY) expr DOSY compound-statement
    std::unique_ptr<ForNode> ASTBuilder::visitForStatement(const ParseTreeNode *node)
    {
        auto forNode = std::make_unique<ForNode>();

        auto terminals = getTerminals(node);
        for (auto *t : terminals)
        {
            if (t->token().type == TokenType::IDENT)
            {
                forNode->varName = t->token().value;
                break;
            }
        }

        // Check direction
        for (auto *t : terminals)
        {
            if (t->token().type == TokenType::DOWNTOSY)
            {
                forNode->isDownTo = true;
                break;
            }
        }

        // Get the two expressions (start and end)
        auto exprs = findChildren(node, "<expression>");
        if (exprs.size() >= 1)
        {
            forNode->startExpr = visitExpression(exprs[0]);
        }
        if (exprs.size() >= 2)
        {
            forNode->endExpr = visitExpression(exprs[1]);
        }

        const ParseTreeNode *compound = findChild(node, "<compound-statement>");
        if (compound)
        {
            forNode->body = visitCompoundStatement(compound);
        }

        return forNode;
    }

    // <procedure/function-call> -> IDENT [LPARENT parameter-list RPARENT]
    std::unique_ptr<ASTNode> ASTBuilder::visitProcedureFunctionCall(const ParseTreeNode *node)
    {
        auto call = std::make_unique<ProcCallNode>();

        auto terminals = getTerminals(node);
        for (auto *t : terminals)
        {
            if (t->token().type == TokenType::IDENT)
            {
                call->name = t->token().value;
                break;
            }
        }

        const ParseTreeNode *paramList = findChild(node, "<parameter-list>");
        if (paramList)
        {
            call->args = visitParameterList(paramList);
        }

        return call;
    }

    // <parameter-list> -> expression (COMMA expression)*
    std::vector<std::unique_ptr<ASTNode>> ASTBuilder::visitParameterList(const ParseTreeNode *node)
    {
        std::vector<std::unique_ptr<ASTNode>> args;

        for (auto &child : node->children())
        {
            if (child->name() == "<expression>")
            {
                args.push_back(visitExpression(child.get()));
            }
        }

        return args;
    }

    // <expression> -> simple-expression [relational-operator simple-expression]
    std::unique_ptr<ASTNode> ASTBuilder::visitExpression(const ParseTreeNode *node)
    {
        auto simpleExprs = findChildren(node, "<simple-expression>");
        const ParseTreeNode *relOp = findChild(node, "<relational-operator>");

        if (relOp && simpleExprs.size() >= 2)
        {
            auto binOp = std::make_unique<BinOpNode>();
            binOp->op = extractOperator(relOp);
            binOp->left = visitSimpleExpression(simpleExprs[0]);
            binOp->right = visitSimpleExpression(simpleExprs[1]);
            return binOp;
        }

        if (simpleExprs.size() >= 1)
        {
            return visitSimpleExpression(simpleExprs[0]);
        }

        return std::make_unique<EmptyNode>();
    }

    // <simple-expression> -> [+|-] term (additive-operator term)*
    std::unique_ptr<ASTNode> ASTBuilder::visitSimpleExpression(const ParseTreeNode *node)
    {
        // Check for leading sign
        bool hasNegSign = false;

        auto terms = findChildren(node, "<term>");
        auto addOps = findChildren(node, "<additive-operator>");

        // Check first child for sign
        if (!node->children().empty())
        {
            const auto *firstTerminal = dynamic_cast<const ParseTreeTerminalNode *>(node->children()[0].get());
            if (firstTerminal)
            {
                if (firstTerminal->token().type == TokenType::MINUS)
                {
                    hasNegSign = true;
                }
                // PLUS sign is a no-op
            }
        }

        if (terms.empty())
        {
            return std::make_unique<EmptyNode>();
        }

        // Build the first term
        std::unique_ptr<ASTNode> result = visitTerm(terms[0]);

        // Apply leading negative sign as unary operator
        if (hasNegSign)
        {
            auto unary = std::make_unique<UnaryOpNode>();
            unary->op = "-";
            unary->operand = std::move(result);
            result = std::move(unary);
        }

        // Chain additive operators left-to-right
        for (size_t i = 0; i < addOps.size() && i + 1 < terms.size(); ++i)
        {
            auto binOp = std::make_unique<BinOpNode>();
            binOp->op = extractOperator(addOps[i]);
            binOp->left = std::move(result);
            binOp->right = visitTerm(terms[i + 1]);
            result = std::move(binOp);
        }

        return result;
    }

    // <term> -> factor (multiplicative-operator factor)*
    std::unique_ptr<ASTNode> ASTBuilder::visitTerm(const ParseTreeNode *node)
    {
        auto factors = findChildren(node, "<factor>");
        auto mulOps = findChildren(node, "<multiplicative-operator>");

        if (factors.empty())
        {
            return std::make_unique<EmptyNode>();
        }

        std::unique_ptr<ASTNode> result = visitFactor(factors[0]);

        // Chain multiplicative operators left-to-right
        for (size_t i = 0; i < mulOps.size() && i + 1 < factors.size(); ++i)
        {
            auto binOp = std::make_unique<BinOpNode>();
            binOp->op = extractOperator(mulOps[i]);
            binOp->left = std::move(result);
            binOp->right = visitFactor(factors[i + 1]);
            result = std::move(binOp);
        }

        return result;
    }

    // <factor> -> INTCON | REALCON | CHARCON | STRING | IDENT | variable |
    //             proc-call | LPARENT expression RPARENT | NOTSY factor
    std::unique_ptr<ASTNode> ASTBuilder::visitFactor(const ParseTreeNode *node)
    {
        for (auto &child : node->children())
        {
            // Nested procedure/function call in expression context
            if (child->name() == "<procedure/function-call>")
            {
                // In expression context, treat as function call
                auto procCall = visitProcedureFunctionCall(child.get());
                auto *procCallPtr = dynamic_cast<ProcCallNode *>(procCall.get());
                if (procCallPtr)
                {
                    auto funcCall = std::make_unique<FuncCallNode>();
                    funcCall->name = procCallPtr->name;
                    funcCall->args = std::move(procCallPtr->args);
                    return funcCall;
                }
                return procCall;
            }

            // Variable access (array/field)
            if (child->name() == "<variable>")
            {
                return visitVariable(child.get());
            }

            // Parenthesized expression
            if (child->name() == "<expression>")
            {
                return visitExpression(child.get());
            }

            // NOT factor (unary)
            if (child->name() == "<factor>")
            {
                // This is the operand of a NOT
                auto unary = std::make_unique<UnaryOpNode>();
                unary->op = "not";
                unary->operand = visitFactor(child.get());
                return unary;
            }

            const auto *terminal = dynamic_cast<const ParseTreeTerminalNode *>(child.get());
            if (!terminal) continue;

            switch (terminal->token().type)
            {
            case TokenType::INTCON:
            {
                auto num = std::make_unique<NumberNode>();
                num->value = std::stoi(terminal->token().value);
                return num;
            }
            case TokenType::REALCON:
            {
                auto real = std::make_unique<RealNode>();
                real->value = std::stod(terminal->token().value);
                return real;
            }
            case TokenType::CHARCON:
            {
                auto ch = std::make_unique<CharNode>();
                ch->value = terminal->token().value.empty() ? '\0' : terminal->token().value[0];
                return ch;
            }
            case TokenType::STRING:
            {
                auto str = std::make_unique<StringNode>();
                str->value = terminal->token().value;
                return str;
            }
            case TokenType::IDENT:
            {
                // Check for boolean literals
                if (terminal->token().value == "True" || terminal->token().value == "true")
                {
                    auto boolNode = std::make_unique<BooleanNode>();
                    boolNode->value = true;
                    return boolNode;
                }
                if (terminal->token().value == "False" || terminal->token().value == "false")
                {
                    auto boolNode = std::make_unique<BooleanNode>();
                    boolNode->value = false;
                    return boolNode;
                }
                auto var = std::make_unique<VarRefNode>();
                var->name = terminal->token().value;
                return var;
            }
            case TokenType::NOTSY:
            {
                // NOT is handled by looking at the next child (factor)
                // Continue to find the factor child
                continue;
            }
            default:
                // Skip parentheses and other punctuation
                continue;
            }
        }

        return std::make_unique<EmptyNode>();
    }

    // Extract operator string from an operator node
    std::string ASTBuilder::extractOperator(const ParseTreeNode *node)
    {
        for (auto &child : node->children())
        {
            const auto *terminal = dynamic_cast<const ParseTreeTerminalNode *>(child.get());
            if (terminal)
            {
                switch (terminal->token().type)
                {
                case TokenType::PLUS:   return "+";
                case TokenType::MINUS:  return "-";
                case TokenType::TIMES:  return "*";
                case TokenType::RDIV:   return "/";
                case TokenType::IDIV:   return "div";
                case TokenType::IMOD:   return "mod";
                case TokenType::ANDSY:  return "and";
                case TokenType::ORSY:   return "or";
                case TokenType::EQL:    return "=";
                case TokenType::NEQ:    return "<>";
                case TokenType::GTR:    return ">";
                case TokenType::GEQ:    return ">=";
                case TokenType::LSS:    return "<";
                case TokenType::LEQ:    return "<=";
                default:                return terminal->token().value;
                }
            }
        }
        return "?";
    }

    // Get the first terminal token from a node
    const Token *ASTBuilder::getTerminalToken(const ParseTreeNode *node) const
    {
        const auto *terminal = dynamic_cast<const ParseTreeTerminalNode *>(node);
        if (terminal)
        {
            return &terminal->token();
        }

        for (auto &child : node->children())
        {
            const Token *result = getTerminalToken(child.get());
            if (result)
            {
                return result;
            }
        }

        return nullptr;
    }

    // Find first child with a given name
    const ParseTreeNode *ASTBuilder::findChild(const ParseTreeNode *node, const std::string &name) const
    {
        for (auto &child : node->children())
        {
            if (child->name() == name)
            {
                return child.get();
            }
        }
        return nullptr;
    }

    // Find all children with a given name
    std::vector<const ParseTreeNode *> ASTBuilder::findChildren(const ParseTreeNode *node, const std::string &name) const
    {
        std::vector<const ParseTreeNode *> result;
        for (auto &child : node->children())
        {
            if (child->name() == name)
            {
                result.push_back(child.get());
            }
        }
        return result;
    }

    // Get all terminal nodes (leaves) from a node
    std::vector<const ParseTreeTerminalNode *> ASTBuilder::getTerminals(const ParseTreeNode *node) const
    {
        std::vector<const ParseTreeTerminalNode *> terminals;
        for (auto &child : node->children())
        {
            const auto *terminal = dynamic_cast<const ParseTreeTerminalNode *>(child.get());
            if (terminal)
            {
                terminals.push_back(terminal);
            }
        }
        return terminals;
    }

} // namespace AST

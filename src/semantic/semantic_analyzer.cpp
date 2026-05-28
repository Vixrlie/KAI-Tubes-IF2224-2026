#include "semantic_analyzer.h"

#include <optional>
#include <sstream>

namespace Semantic
{
    namespace
    {
        struct ConstValue
        {
            TypeInfo type;
            bool hasInt = false;
            int intValue = 0;
            bool hasChar = false;
            char charValue = '\0';
            bool hasBool = false;
            bool boolValue = false;
        };
    }

    SemanticAnalyzer::Result SemanticAnalyzer::analyze(AST::ProgramNode *root)
    {
        errors.clear();
        symbols = SymbolTable();
        blockStack.clear();
        subranges.clear();
        enums.clear();
        recordFieldLast.clear();
        procSignatures.clear();
        functionStack.clear();
        blockStack.push_back(0);

        addBuiltinProcedures();

        if (!root)
        {
            addError("SemanticAnalyzer: null AST root");
            return {false, errors};
        }

        visitProgram(root);

        return {errors.empty(), errors};
    }

    const SymbolTable &SemanticAnalyzer::symbolTable() const
    {
        return symbols;
    }

    void SemanticAnalyzer::addError(const std::string &message)
    {
        errors.push_back(message);
    }

    void SemanticAnalyzer::addBuiltinProcedures()
    {
        std::string error;
        TypeInfo unknownType{BasicType::UNKNOWN, 0};

        if (symbols.lookupInCurrentScope("writeln") == std::nullopt)
        {
            symbols.registerIdentifier("writeln", ObjectClass::PROCEDURE, unknownType, 1, 0, error);
            procSignatures["writeln"] = ProcSignature{false, false, unknownType, {}, {}};
        }
        if (symbols.lookupInCurrentScope("readln") == std::nullopt)
        {
            symbols.registerIdentifier("readln", ObjectClass::PROCEDURE, unknownType, 1, 0, error);
            procSignatures["readln"] = ProcSignature{false, false, unknownType, {}, {}};
        }
    }

    void SemanticAnalyzer::enterScope()
    {
        int blockIndex = symbols.pushScope();
        blockStack.push_back(blockIndex);
    }

    void SemanticAnalyzer::exitScope()
    {
        symbols.popScope();
        if (!blockStack.empty())
        {
            blockStack.pop_back();
        }
    }

    int SemanticAnalyzer::currentBlock() const
    {
        return blockStack.empty() ? -1 : blockStack.back();
    }

    int SemanticAnalyzer::currentLevel() const
    {
        return symbols.currentLevel();
    }

    void SemanticAnalyzer::annotateNode(AST::ASTNode *node, const TypeResult &typeResult, int tabIndex)
    {
        if (!node)
        {
            return;
        }
        node->annotation.typeCode = static_cast<int>(typeResult.info.kind);
        node->annotation.typeName = typeResult.name;
        node->annotation.tabIndex = tabIndex;
        node->annotation.blockIndex = currentBlock();
        node->annotation.level = currentLevel();
    }

    SemanticAnalyzer::TypeResult SemanticAnalyzer::unknownType() const
    {
        TypeResult result;
        result.info = {BasicType::UNKNOWN, 0};
        result.name = "Unknown";
        result.ok = false;
        return result;
    }

    SemanticAnalyzer::TypeResult SemanticAnalyzer::basicType(BasicType kind) const
    {
        TypeResult result;
        result.info = {kind, 0};
        result.name = typeNameFromKind(kind);
        result.ok = true;
        return result;
    }

    std::string SemanticAnalyzer::typeNameFromKind(BasicType kind) const
    {
        switch (kind)
        {
        case BasicType::INTEGER:
            return "Integer";
        case BasicType::REAL:
            return "Real";
        case BasicType::CHAR:
            return "Char";
        case BasicType::BOOLEAN:
            return "Boolean";
        case BasicType::STRING:
            return "String";
        case BasicType::ARRAY:
            return "Array";
        case BasicType::RECORD:
            return "Record";
        case BasicType::SUBRANGE:
            return "Subrange";
        case BasicType::ENUMERATED:
            return "Enumerated";
        case BasicType::NAMED:
            return "Named";
        case BasicType::UNKNOWN:
        default:
            return "Unknown";
        }
    }

    SemanticAnalyzer::TypeResult SemanticAnalyzer::resolveNamedType(const std::string &name)
    {
        auto entry = symbols.lookup(name);
        if (!entry.has_value())
        {
            std::string lowered = name;
            for (char &c : lowered)
            {
                c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
            }

            if (lowered == "integer")
            {
                entry = symbols.lookup("Integer");
            }
            else if (lowered == "real")
            {
                entry = symbols.lookup("Real");
            }
            else if (lowered == "char")
            {
                entry = symbols.lookup("Char");
            }
            else if (lowered == "boolean")
            {
                entry = symbols.lookup("Boolean");
            }
            else if (lowered == "string")
            {
                entry = symbols.lookup("String");
            }
        }
        if (!entry.has_value())
        {
            addError("Undefined type: " + name);
            return unknownType();
        }
        if (entry->entry.obj != ObjectClass::TYPE)
        {
            addError("Identifier is not a type: " + name);
            return unknownType();
        }

        TypeResult result;
        result.info = entry->entry.type;
        result.name = typeNameFromKind(entry->entry.type.kind);
        result.ok = true;
        return result;
    }

    SemanticAnalyzer::TypeResult SemanticAnalyzer::resolveTypeSpec(AST::ASTNode *node)
    {
        if (!node)
        {
            return unknownType();
        }

        if (auto *named = dynamic_cast<AST::NamedTypeNode *>(node))
        {
            return resolveNamedType(named->name);
        }

        if (auto *arrayType = dynamic_cast<AST::ArrayTypeNode *>(node))
        {
            TypeResult indexType = resolveTypeSpec(arrayType->indexType.get());
            TypeResult elementType = resolveTypeSpec(arrayType->elementType.get());

            if (indexType.info.kind == BasicType::REAL)
            {
                addError("Array index type cannot be Real");
            }

            AtabEntry arrayInfo{};
            arrayInfo.xtyp = static_cast<int>(indexType.info.kind);
            arrayInfo.xref = indexType.info.ref;
            arrayInfo.etyp = static_cast<int>(elementType.info.kind);
            arrayInfo.eref = elementType.info.ref;
            arrayInfo.low = 0;
            arrayInfo.high = 0;
            arrayInfo.elsz = 1;
            arrayInfo.size = 0;

            if (indexType.info.kind == BasicType::SUBRANGE && indexType.info.ref >= 0 &&
                indexType.info.ref < static_cast<int>(subranges.size()))
            {
                const SubrangeInfo &range = subranges[indexType.info.ref];
                if (range.hasIntBounds)
                {
                    arrayInfo.low = range.lowInt;
                    arrayInfo.high = range.highInt;
                    arrayInfo.size = arrayInfo.high - arrayInfo.low + 1;
                }
                else if (range.hasCharBounds)
                {
                    arrayInfo.low = static_cast<int>(range.lowChar);
                    arrayInfo.high = static_cast<int>(range.highChar);
                    arrayInfo.size = arrayInfo.high - arrayInfo.low + 1;
                }
            }

            // If element type is itself an array, incorporate its total size
            if (elementType.info.kind == BasicType::ARRAY && elementType.info.ref >= 0)
            {
                const auto &atab = symbols.atab();
                if (elementType.info.ref < static_cast<int>(atab.size()))
                {
                    int elemTotal = atab[elementType.info.ref].size;
                    if (elemTotal > 0 && arrayInfo.size > 0)
                    {
                        arrayInfo.elsz = elemTotal;
                        arrayInfo.size = arrayInfo.size * elemTotal;
                    }
                }
            }

            int atabIndex = symbols.addArrayType(arrayInfo);

            TypeResult result;
            result.info = {BasicType::ARRAY, atabIndex};
            result.name = "Array";
            result.ok = true;
            return result;
        }

        if (auto *recordType = dynamic_cast<AST::RecordTypeNode *>(node))
        {
            enterScope();
            int recordRef = currentBlock();
            int lastIndex = 0;
            int fieldOffset = 0;

            for (auto &field : recordType->fields)
            {
                TypeResult fieldType = resolveTypeSpec(field->typeSpec.get());
                std::string error;
                if (!symbols.registerIdentifier(field->name, ObjectClass::VAR, fieldType.info, 1, fieldOffset, error))
                {
                    addError(error);
                }
                else
                {
                    lastIndex = static_cast<int>(symbols.tab().size()) - 1;
                    ++fieldOffset;
                }
            }

            recordFieldLast[recordRef] = lastIndex;
            exitScope();

            TypeResult result;
            result.info = {BasicType::RECORD, recordRef};
            result.name = "Record";
            result.ok = true;
            return result;
        }

        if (auto *subrangeType = dynamic_cast<AST::SubrangeTypeNode *>(node))
        {
            TypeResult lowType = visitConstant(subrangeType->low.get());
            TypeResult highType = visitConstant(subrangeType->high.get());

            if (!isSameType(lowType.info, highType.info))
            {
                addError("Subrange bounds must have the same type");
            }
            if (lowType.info.kind == BasicType::REAL || highType.info.kind == BasicType::REAL)
            {
                addError("Subrange cannot be based on Real");
            }

            SubrangeInfo info;
            info.baseType = lowType.info;

            ConstValue lowConst{};
            ConstValue highConst{};
            lowConst.type = lowType.info;
            highConst.type = highType.info;

            if (auto *num = dynamic_cast<AST::NumberNode *>(subrangeType->low.get()))
            {
                lowConst.hasInt = true;
                lowConst.intValue = num->value;
            }
            if (auto *num = dynamic_cast<AST::NumberNode *>(subrangeType->high.get()))
            {
                highConst.hasInt = true;
                highConst.intValue = num->value;
            }
            if (auto *ch = dynamic_cast<AST::CharNode *>(subrangeType->low.get()))
            {
                lowConst.hasChar = true;
                lowConst.charValue = ch->value;
            }
            if (auto *ch = dynamic_cast<AST::CharNode *>(subrangeType->high.get()))
            {
                highConst.hasChar = true;
                highConst.charValue = ch->value;
            }

            if (lowConst.hasInt && highConst.hasInt)
            {
                if (lowConst.intValue > highConst.intValue)
                {
                    addError("Subrange lower bound is greater than upper bound");
                }
                info.hasIntBounds = true;
                info.lowInt = lowConst.intValue;
                info.highInt = highConst.intValue;
            }
            if (lowConst.hasChar && highConst.hasChar)
            {
                if (lowConst.charValue > highConst.charValue)
                {
                    addError("Subrange lower bound is greater than upper bound");
                }
                info.hasCharBounds = true;
                info.lowChar = lowConst.charValue;
                info.highChar = highConst.charValue;
            }

            subranges.push_back(info);

            TypeResult result;
            result.info = {BasicType::SUBRANGE, static_cast<int>(subranges.size()) - 1};
            result.name = "Subrange";
            result.ok = true;
            return result;
        }

        if (auto *enumType = dynamic_cast<AST::EnumeratedTypeNode *>(node))
        {
            EnumInfo info;
            info.identifiers = enumType->identifiers;
            enums.push_back(info);

            TypeResult result;
            result.info = {BasicType::ENUMERATED, static_cast<int>(enums.size()) - 1};
            result.name = "Enumerated";
            result.ok = true;
            return result;
        }

        return unknownType();
    }

    SemanticAnalyzer::TypeResult SemanticAnalyzer::visitConstant(AST::ASTNode *node)
    {
        if (!node)
        {
            return unknownType();
        }

        if (dynamic_cast<AST::NumberNode *>(node))
        {
            TypeResult result = basicType(BasicType::INTEGER);
            annotateNode(node, result);
            return result;
        }
        if (dynamic_cast<AST::RealNode *>(node))
        {
            TypeResult result = basicType(BasicType::REAL);
            annotateNode(node, result);
            return result;
        }
        if (dynamic_cast<AST::CharNode *>(node))
        {
            TypeResult result = basicType(BasicType::CHAR);
            annotateNode(node, result);
            return result;
        }
        if (dynamic_cast<AST::StringNode *>(node))
        {
            TypeResult result = basicType(BasicType::STRING);
            annotateNode(node, result);
            return result;
        }
        if (dynamic_cast<AST::BooleanNode *>(node))
        {
            TypeResult result = basicType(BasicType::BOOLEAN);
            annotateNode(node, result);
            return result;
        }
        if (auto *var = dynamic_cast<AST::VarRefNode *>(node))
        {
            return visitVariable(var);
        }

        return unknownType();
    }

    SemanticAnalyzer::TypeResult SemanticAnalyzer::visitVariable(AST::ASTNode *node)
    {
        if (auto *var = dynamic_cast<AST::VarRefNode *>(node))
        {
            auto entry = symbols.lookup(var->name);
            if (!entry.has_value())
            {
                addError("Undefined identifier: " + var->name);
                TypeResult result = unknownType();
                annotateNode(node, result);
                return result;
            }

            TypeResult result;
            result.info = entry->entry.type;
            result.name = typeNameFromKind(entry->entry.type.kind);
            result.ok = true;
            annotateNode(node, result, entry->index);
            return result;
        }

        if (auto *arrayAccess = dynamic_cast<AST::ArrayAccessNode *>(node))
        {
            TypeResult currentType = visitExpression(arrayAccess->base.get());
            for (auto &idx : arrayAccess->indices)
            {
                if (currentType.info.kind != BasicType::ARRAY)
                {
                    addError("Array access on non-array expression");
                    TypeResult result = unknownType();
                    annotateNode(node, result);
                    return result;
                }

                TypeResult indexType = visitExpression(idx.get());
                const std::vector<AtabEntry> &atab = symbols.atab();
                if (currentType.info.ref < 0 || currentType.info.ref >= static_cast<int>(atab.size()))
                {
                    addError("Array type reference is invalid");
                    TypeResult result = unknownType();
                    annotateNode(node, result);
                    return result;
                }

                const AtabEntry &arrayInfo = atab[currentType.info.ref];
                TypeInfo expectedIndexType{static_cast<BasicType>(arrayInfo.xtyp), arrayInfo.xref};

                if (!isAssignmentCompatible(expectedIndexType, indexType.info, idx.get()))
                {
                    addError("Array index type is incompatible with array index domain");
                }

                currentType.info.kind = static_cast<BasicType>(arrayInfo.etyp);
                currentType.info.ref = arrayInfo.eref;
                currentType.name = typeNameFromKind(currentType.info.kind);
                currentType.ok = true;
            }

            annotateNode(node, currentType);
            return currentType;
        }

        if (auto *fieldAccess = dynamic_cast<AST::FieldAccessNode *>(node))
        {
            TypeResult baseType = visitExpression(fieldAccess->base.get());
            if (baseType.info.kind != BasicType::RECORD)
            {
                addError("Field access on non-record expression");
                TypeResult result = unknownType();
                annotateNode(node, result);
                return result;
            }

            int recordRef = baseType.info.ref;
            int lastIndex = recordFieldLast.count(recordRef) ? recordFieldLast[recordRef] : 0;
            const auto &tabEntries = symbols.tab();
            int current = lastIndex;
            while (current > 0 && current < static_cast<int>(tabEntries.size()))
            {
                const TabEntry &entry = tabEntries[current];
                if (entry.identifier == fieldAccess->fieldName)
                {
                    TypeResult result;
                    result.info = entry.type;
                    result.name = typeNameFromKind(entry.type.kind);
                    result.ok = true;
                    annotateNode(node, result, current);
                    return result;
                }
                current = entry.link;
            }

            addError("Unknown record field: " + fieldAccess->fieldName);
            TypeResult result = unknownType();
            annotateNode(node, result);
            return result;
        }

        return unknownType();
    }

    SemanticAnalyzer::TypeResult SemanticAnalyzer::visitExpression(AST::ASTNode *node)
    {
        if (!node)
        {
            return unknownType();
        }

        if (dynamic_cast<AST::VarRefNode *>(node) ||
            dynamic_cast<AST::ArrayAccessNode *>(node) ||
            dynamic_cast<AST::FieldAccessNode *>(node))
        {
            return visitVariable(node);
        }

        if (dynamic_cast<AST::NumberNode *>(node) ||
            dynamic_cast<AST::RealNode *>(node) ||
            dynamic_cast<AST::CharNode *>(node) ||
            dynamic_cast<AST::StringNode *>(node) ||
            dynamic_cast<AST::BooleanNode *>(node))
        {
            return visitConstant(node);
        }

        if (auto *binOp = dynamic_cast<AST::BinOpNode *>(node))
        {
            TypeResult left = visitExpression(binOp->left.get());
            TypeResult right = visitExpression(binOp->right.get());

            TypeResult result = unknownType();
            const std::string &op = binOp->op;

            if (op == "+" || op == "-" || op == "*" || op == "/" || op == "div" || op == "mod")
            {
                if (!isNumeric(left.info) || !isNumeric(right.info))
                {
                    addError("Arithmetic operator applied to non-numeric operands");
                }

                if (op == "div" || op == "mod")
                {
                    if (!isIntegerLike(left.info) || !isIntegerLike(right.info))
                    {
                        addError("div/mod require Integer operands");
                    }
                    result = basicType(BasicType::INTEGER);
                }
                else if (op == "/")
                {
                    result = basicType(BasicType::REAL);
                }
                else
                {
                    if (left.info.kind == BasicType::REAL || right.info.kind == BasicType::REAL)
                    {
                        result = basicType(BasicType::REAL);
                    }
                    else
                    {
                        result = basicType(BasicType::INTEGER);
                    }
                }
            }
            else if (op == "and" || op == "or")
            {
                if (!isBoolean(left.info) || !isBoolean(right.info))
                {
                    addError("Boolean operator applied to non-boolean operands");
                }
                result = basicType(BasicType::BOOLEAN);
            }
            else
            {
                bool compatible = isCompatible(left.info, right.info);
                int leftStringLength = stringLiteralLength(binOp->left.get());
                int rightStringLength = stringLiteralLength(binOp->right.get());
                if (compatible &&
                    left.info.kind == BasicType::STRING &&
                    right.info.kind == BasicType::STRING &&
                    leftStringLength >= 0 &&
                    rightStringLength >= 0 &&
                    leftStringLength != rightStringLength)
                {
                    compatible = false;
                }

                if (!compatible)
                {
                    addError("Relational operator applied to incompatible operands");
                }
                result = basicType(BasicType::BOOLEAN);
            }

            annotateNode(node, result);
            return result;
        }

        if (auto *unary = dynamic_cast<AST::UnaryOpNode *>(node))
        {
            TypeResult operand = visitExpression(unary->operand.get());
            TypeResult result = unknownType();

            if (unary->op == "-")
            {
                if (!isNumeric(operand.info))
                {
                    addError("Unary '-' expects numeric operand");
                }
                result = operand;
            }
            else if (unary->op == "not")
            {
                if (!isBoolean(operand.info))
                {
                    addError("'not' expects Boolean operand");
                }
                result = basicType(BasicType::BOOLEAN);
            }

            annotateNode(node, result);
            return result;
        }

        if (auto *call = dynamic_cast<AST::FuncCallNode *>(node))
        {
            return visitFuncCall(call);
        }

        return unknownType();
    }

    void SemanticAnalyzer::visitProgram(AST::ProgramNode *node)
    {
        std::string error;
        TypeInfo programType{BasicType::UNKNOWN, 0};
        symbols.registerIdentifier(node->name, ObjectClass::PROGRAM, programType, 1, 0, error);

        annotateNode(node, basicType(BasicType::UNKNOWN));

        for (auto &decl : node->declarations)
        {
            visitDeclaration(decl.get());
        }

        if (node->body)
        {
            visitCompound(node->body.get());
        }
    }

    void SemanticAnalyzer::visitDeclaration(AST::ASTNode *node)
    {
        if (!node)
        {
            return;
        }

        if (auto *constDecl = dynamic_cast<AST::ConstDeclNode *>(node))
        {
            visitConstDecl(constDecl);
            return;
        }
        if (auto *typeDecl = dynamic_cast<AST::TypeDeclNode *>(node))
        {
            visitTypeDecl(typeDecl);
            return;
        }
        if (auto *varDecl = dynamic_cast<AST::VarDeclNode *>(node))
        {
            visitVarDecl(varDecl);
            return;
        }
        if (auto *procDecl = dynamic_cast<AST::ProcedureDeclNode *>(node))
        {
            visitProcedureDecl(procDecl);
            return;
        }
        if (auto *funcDecl = dynamic_cast<AST::FunctionDeclNode *>(node))
        {
            visitFunctionDecl(funcDecl);
            return;
        }
    }

    void SemanticAnalyzer::visitConstDecl(AST::ConstDeclNode *node)
    {
        TypeResult valueType = visitConstant(node->value.get());

        std::string error;
        int adr = 0;
        if (auto *num = dynamic_cast<AST::NumberNode *>(node->value.get()))
        {
            adr = num->value;
        }
        else if (auto *ch = dynamic_cast<AST::CharNode *>(node->value.get()))
        {
            adr = static_cast<int>(ch->value);
        }
        else if (auto *booleanNode = dynamic_cast<AST::BooleanNode *>(node->value.get()))
        {
            adr = booleanNode->value ? 1 : 0;
        }

        if (!symbols.registerIdentifier(node->name, ObjectClass::CONST, valueType.info, 1, adr, error))
        {
            addError(error);
        }
        else
        {
            int tabIndex = static_cast<int>(symbols.tab().size()) - 1;
            annotateNode(node, valueType, tabIndex);
        }
    }

    void SemanticAnalyzer::visitTypeDecl(AST::TypeDeclNode *node)
    {
        TypeResult typeResult = resolveTypeSpec(node->typeSpec.get());

        std::string error;
        if (!symbols.registerIdentifier(node->name, ObjectClass::TYPE, typeResult.info, 1, 0, error))
        {
            addError(error);
        }
        else
        {
            int tabIndex = static_cast<int>(symbols.tab().size()) - 1;
            annotateNode(node, typeResult, tabIndex);
        }

        if (auto *enumType = dynamic_cast<AST::EnumeratedTypeNode *>(node->typeSpec.get()))
        {
            int ordinal = 0;
            for (const auto &identifier : enumType->identifiers)
            {
                std::string constError;
                symbols.registerIdentifier(identifier, ObjectClass::CONST, typeResult.info, 1, ordinal, constError);
                if (!constError.empty())
                {
                    addError(constError);
                }
                ++ordinal;
            }
        }
    }

    void SemanticAnalyzer::visitVarDecl(AST::VarDeclNode *node)
    {
        TypeResult typeResult = resolveTypeSpec(node->typeSpec.get());

        std::string error;
        if (!symbols.registerIdentifier(node->name, ObjectClass::VAR, typeResult.info, 1, -1, error))
        {
            addError(error);
        }
        else
        {
            int tabIndex = static_cast<int>(symbols.tab().size()) - 1;
            annotateNode(node, typeResult, tabIndex);
        }
    }

    void SemanticAnalyzer::visitProcedureDecl(AST::ProcedureDeclNode *node)
    {
        std::string error;
        TypeInfo procType{BasicType::UNKNOWN, 0};
        if (!symbols.registerIdentifier(node->name, ObjectClass::PROCEDURE, procType, 1, 0, error))
        {
            addError(error);
        }

        ProcSignature signature;
        signature.isFunction = false;
        signature.hasParamInfo = true;

        enterScope();

        for (auto &param : node->params)
        {
            TypeResult paramType = param->typeSpec ? resolveTypeSpec(param->typeSpec.get()) : resolveNamedType(param->typeName);
            std::string paramError;
            int nrm = param->isVar ? 0 : 1;
            if (!symbols.registerIdentifier(param->name, ObjectClass::VAR, paramType.info, nrm, -1, paramError, true))
            {
                addError(paramError);
            }
            signature.params.push_back(paramType.info);
            signature.byRefParams.push_back(param->isVar);
            annotateNode(param.get(), paramType);
        }

        procSignatures[node->name] = signature;

        for (auto &decl : node->localDecls)
        {
            visitDeclaration(decl.get());
        }

        if (node->body)
        {
            visitCompound(node->body.get());
        }

        annotateNode(node, basicType(BasicType::UNKNOWN));
        exitScope();
    }

    void SemanticAnalyzer::visitFunctionDecl(AST::FunctionDeclNode *node)
    {
        TypeResult returnType = resolveNamedType(node->returnTypeName);

        std::string error;
        if (!symbols.registerIdentifier(node->name, ObjectClass::FUNCTION, returnType.info, 1, 0, error))
        {
            addError(error);
        }

        ProcSignature signature;
        signature.isFunction = true;
        signature.hasParamInfo = true;
        signature.returnType = returnType.info;

        enterScope();

        for (auto &param : node->params)
        {
            TypeResult paramType = param->typeSpec ? resolveTypeSpec(param->typeSpec.get()) : resolveNamedType(param->typeName);
            std::string paramError;
            int nrm = param->isVar ? 0 : 1;
            if (!symbols.registerIdentifier(param->name, ObjectClass::VAR, paramType.info, nrm, -1, paramError, true))
            {
                addError(paramError);
            }
            signature.params.push_back(paramType.info);
            signature.byRefParams.push_back(param->isVar);
            annotateNode(param.get(), paramType);
        }

        procSignatures[node->name] = signature;

        functionStack.push_back(node->name);
        for (auto &decl : node->localDecls)
        {
            visitDeclaration(decl.get());
        }

        if (node->body)
        {
            visitCompound(node->body.get());
        }

        if (!node->body || !guaranteesFunctionResult(node->body.get(), node->name))
        {
            addError("Function may exit without assigning a return value: " + node->name);
        }

        annotateNode(node, returnType);
        functionStack.pop_back();
        exitScope();
    }

    void SemanticAnalyzer::visitStatement(AST::ASTNode *node)
    {
        if (!node)
        {
            return;
        }

        if (auto *assign = dynamic_cast<AST::AssignNode *>(node))
        {
            visitAssign(assign);
            return;
        }
        if (auto *ifStmt = dynamic_cast<AST::IfNode *>(node))
        {
            visitIf(ifStmt);
            return;
        }
        if (auto *whileStmt = dynamic_cast<AST::WhileNode *>(node))
        {
            visitWhile(whileStmt);
            return;
        }
        if (auto *repeatStmt = dynamic_cast<AST::RepeatNode *>(node))
        {
            visitRepeat(repeatStmt);
            return;
        }
        if (auto *forStmt = dynamic_cast<AST::ForNode *>(node))
        {
            visitFor(forStmt);
            return;
        }
        if (auto *caseStmt = dynamic_cast<AST::CaseNode *>(node))
        {
            visitCase(caseStmt);
            return;
        }
        if (auto *compound = dynamic_cast<AST::CompoundNode *>(node))
        {
            visitCompound(compound);
            return;
        }
        if (auto *procCall = dynamic_cast<AST::ProcCallNode *>(node))
        {
            visitProcCall(procCall);
            return;
        }
    }

    void SemanticAnalyzer::visitCompound(AST::CompoundNode *node)
    {
        if (!node)
        {
            return;
        }
        annotateNode(node, basicType(BasicType::UNKNOWN));
        for (auto &stmt : node->statements)
        {
            visitStatement(stmt.get());
        }
    }

    void SemanticAnalyzer::visitAssign(AST::AssignNode *node)
    {
        if (!node)
        {
            return;
        }

        TypeResult targetType = visitExpression(node->target.get());
        TypeResult valueType = visitExpression(node->value.get());

        if (!isAssignableTarget(node->target.get()))
        {
            addError("Assignment target is not assignable");
        }

        if (!isAssignmentCompatible(targetType.info, valueType.info, node->value.get()))
        {
            addError("Assignment types are incompatible");
        }

        annotateNode(node, basicType(BasicType::UNKNOWN));
        if (node->target)
        {
            node->target->annotation.initialized = true;
        }
    }

    void SemanticAnalyzer::visitIf(AST::IfNode *node)
    {
        if (!node)
        {
            return;
        }
        TypeResult conditionType = visitExpression(node->condition.get());
        if (!isBoolean(conditionType.info))
        {
            addError("If condition must be Boolean");
        }

        visitStatement(node->thenBranch.get());
        if (node->elseBranch)
        {
            visitStatement(node->elseBranch.get());
        }

        annotateNode(node, basicType(BasicType::UNKNOWN));
    }

    void SemanticAnalyzer::visitWhile(AST::WhileNode *node)
    {
        if (!node)
        {
            return;
        }

        TypeResult conditionType = visitExpression(node->condition.get());
        if (!isBoolean(conditionType.info))
        {
            addError("While condition must be Boolean");
        }

        if (node->body)
        {
            visitCompound(node->body.get());
        }

        annotateNode(node, basicType(BasicType::UNKNOWN));
    }

    void SemanticAnalyzer::visitRepeat(AST::RepeatNode *node)
    {
        if (!node)
        {
            return;
        }

        for (auto &stmt : node->statements)
        {
            visitStatement(stmt.get());
        }

        TypeResult conditionType = visitExpression(node->condition.get());
        if (!isBoolean(conditionType.info))
        {
            addError("Repeat-until condition must be Boolean");
        }

        annotateNode(node, basicType(BasicType::UNKNOWN));
    }

    void SemanticAnalyzer::visitFor(AST::ForNode *node)
    {
        if (!node)
        {
            return;
        }

        auto varEntry = symbols.lookup(node->varName);
        if (!varEntry.has_value())
        {
            addError("Undefined loop variable: " + node->varName);
        }
        else
        {
            TypeInfo varType = varEntry->entry.type;
            if (!isIntegerLike(varType))
            {
                addError("For loop variable must be Integer or Subrange of Integer");
            }
        }

        TypeResult startType = visitExpression(node->startExpr.get());
        TypeResult endType = visitExpression(node->endExpr.get());

        if (!isCompatible(startType.info, endType.info))
        {
            addError("For loop bounds must be compatible types");
        }

        if (node->body)
        {
            visitCompound(node->body.get());
        }

        annotateNode(node, basicType(BasicType::UNKNOWN));
    }

    void SemanticAnalyzer::visitCase(AST::CaseNode *node)
    {
        if (!node)
        {
            return;
        }

        TypeResult selectorType = visitExpression(node->selector.get());
        for (auto &branch : node->branches)
        {
            visitCaseBranch(branch.get(), selectorType);
        }

        annotateNode(node, basicType(BasicType::UNKNOWN));
    }

    void SemanticAnalyzer::visitCaseBranch(AST::CaseBranchNode *node, const TypeResult &selectorType)
    {
        if (!node)
        {
            return;
        }

        for (auto &label : node->labels)
        {
            TypeResult labelType = visitConstant(label.get());
            if (!isCompatible(selectorType.info, labelType.info))
            {
                addError("Case label type incompatible with selector");
            }
        }

        if (node->body)
        {
            visitStatement(node->body.get());
        }

        annotateNode(node, basicType(BasicType::UNKNOWN));
    }

    void SemanticAnalyzer::visitProcCall(AST::ProcCallNode *node)
    {
        if (!node)
        {
            return;
        }

        auto entry = symbols.lookup(node->name);
        if (!entry.has_value())
        {
            addError("Undefined procedure: " + node->name);
            annotateNode(node, basicType(BasicType::UNKNOWN));
            return;
        }

        if (entry->entry.obj != ObjectClass::PROCEDURE)
        {
            addError("Identifier is not a procedure: " + node->name);
        }

        auto signatureIt = procSignatures.find(node->name);
        if (signatureIt != procSignatures.end() && signatureIt->second.hasParamInfo)
        {
            const auto &expectedParams = signatureIt->second.params;
            if (node->args.size() != expectedParams.size())
            {
                addError("Procedure argument count mismatch: " + node->name);
            }
            else
            {
                for (size_t i = 0; i < node->args.size(); ++i)
                {
                    TypeResult argType = visitExpression(node->args[i].get());
                    if (i < signatureIt->second.byRefParams.size() &&
                        signatureIt->second.byRefParams[i] &&
                        !isVariableLike(node->args[i].get()))
                    {
                        addError("By-reference procedure argument must be a variable: " + node->name);
                    }
                    if (!isAssignmentCompatible(expectedParams[i], argType.info, node->args[i].get()))
                    {
                        addError("Procedure argument type mismatch: " + node->name);
                    }
                }
            }
        }
        else
        {
            for (auto &arg : node->args)
            {
                visitExpression(arg.get());
            }
        }

        annotateNode(node, basicType(BasicType::UNKNOWN));
    }

    SemanticAnalyzer::TypeResult SemanticAnalyzer::visitFuncCall(AST::FuncCallNode *node)
    {
        if (!node)
        {
            return unknownType();
        }

        auto entry = symbols.lookup(node->name);
        if (!entry.has_value())
        {
            addError("Undefined function: " + node->name);
            TypeResult result = unknownType();
            annotateNode(node, result);
            return result;
        }

        if (entry->entry.obj != ObjectClass::FUNCTION)
        {
            addError("Identifier is not a function: " + node->name);
        }

        auto signatureIt = procSignatures.find(node->name);
        if (signatureIt != procSignatures.end() && signatureIt->second.hasParamInfo)
        {
            const auto &expectedParams = signatureIt->second.params;
            if (node->args.size() != expectedParams.size())
            {
                addError("Function argument count mismatch: " + node->name);
            }
            else
            {
                for (size_t i = 0; i < node->args.size(); ++i)
                {
                    TypeResult argType = visitExpression(node->args[i].get());
                    if (i < signatureIt->second.byRefParams.size() &&
                        signatureIt->second.byRefParams[i] &&
                        !isVariableLike(node->args[i].get()))
                    {
                        addError("By-reference function argument must be a variable: " + node->name);
                    }
                    if (!isAssignmentCompatible(expectedParams[i], argType.info, node->args[i].get()))
                    {
                        addError("Function argument type mismatch: " + node->name);
                    }
                }
            }
        }
        else
        {
            for (auto &arg : node->args)
            {
                visitExpression(arg.get());
            }
        }

        TypeResult result;
        result.info = entry->entry.type;
        result.name = typeNameFromKind(entry->entry.type.kind);
        result.ok = true;
        annotateNode(node, result, entry->index);
        return result;
    }

    bool SemanticAnalyzer::isAssignableTarget(AST::ASTNode *node)
    {
        if (!node)
        {
            return false;
        }

        if (auto *var = dynamic_cast<AST::VarRefNode *>(node))
        {
            auto entry = symbols.lookup(var->name);
            if (!entry.has_value())
            {
                return false;
            }

            if (entry->entry.obj == ObjectClass::VAR)
            {
                return true;
            }

            if (entry->entry.obj == ObjectClass::FUNCTION &&
                !functionStack.empty() &&
                functionStack.back() == var->name)
            {
                return true;
            }

            return false;
        }

        if (dynamic_cast<AST::ArrayAccessNode *>(node) ||
            dynamic_cast<AST::FieldAccessNode *>(node))
        {
            return true;
        }

        return false;
    }

    bool SemanticAnalyzer::isVariableLike(AST::ASTNode *node) const
    {
        return dynamic_cast<AST::VarRefNode *>(node) ||
               dynamic_cast<AST::ArrayAccessNode *>(node) ||
               dynamic_cast<AST::FieldAccessNode *>(node);
    }

    bool SemanticAnalyzer::assignsFunctionResult(AST::AssignNode *node, const std::string &functionName) const
    {
        if (!node)
        {
            return false;
        }

        auto *target = dynamic_cast<AST::VarRefNode *>(node->target.get());
        return target && target->name == functionName;
    }

    bool SemanticAnalyzer::guaranteesFunctionResult(AST::ASTNode *node, const std::string &functionName) const
    {
        if (!node)
        {
            return false;
        }

        if (auto *assign = dynamic_cast<AST::AssignNode *>(node))
        {
            return assignsFunctionResult(assign, functionName);
        }

        if (auto *compound = dynamic_cast<AST::CompoundNode *>(node))
        {
            for (const auto &stmt : compound->statements)
            {
                if (guaranteesFunctionResult(stmt.get(), functionName))
                {
                    return true;
                }
            }
            return false;
        }

        if (auto *ifStmt = dynamic_cast<AST::IfNode *>(node))
        {
            return ifStmt->thenBranch &&
                   ifStmt->elseBranch &&
                   guaranteesFunctionResult(ifStmt->thenBranch.get(), functionName) &&
                   guaranteesFunctionResult(ifStmt->elseBranch.get(), functionName);
        }

        if (auto *repeatStmt = dynamic_cast<AST::RepeatNode *>(node))
        {
            for (const auto &stmt : repeatStmt->statements)
            {
                if (guaranteesFunctionResult(stmt.get(), functionName))
                {
                    return true;
                }
            }
        }

        return false;
    }

    int SemanticAnalyzer::stringLiteralLength(const AST::ASTNode *node) const
    {
        auto *stringNode = dynamic_cast<const AST::StringNode *>(node);
        if (!stringNode)
        {
            return -1;
        }

        std::string value = stringNode->value;
        if (value.size() >= 2 && value.front() == '\'' && value.back() == '\'')
        {
            value = value.substr(1, value.size() - 2);
        }
        return static_cast<int>(value.size());
    }

    bool SemanticAnalyzer::isNumeric(const TypeInfo &type) const
    {
        TypeInfo baseType = unwrapSubrange(type);
        return baseType.kind == BasicType::INTEGER || baseType.kind == BasicType::REAL;
    }

    bool SemanticAnalyzer::isIntegerLike(const TypeInfo &type) const
    {
        TypeInfo baseType = unwrapSubrange(type);
        return baseType.kind == BasicType::INTEGER;
    }

    bool SemanticAnalyzer::isBoolean(const TypeInfo &type) const
    {
        return type.kind == BasicType::BOOLEAN;
    }

    bool SemanticAnalyzer::isString(const TypeInfo &type) const
    {
        return type.kind == BasicType::STRING;
    }

    TypeInfo SemanticAnalyzer::unwrapSubrange(const TypeInfo &type) const
    {
        if (type.kind != BasicType::SUBRANGE)
        {
            return type;
        }
        if (type.ref >= 0 && type.ref < static_cast<int>(subranges.size()))
        {
            return subranges[type.ref].baseType;
        }
        return type;
    }

    bool SemanticAnalyzer::isSameType(const TypeInfo &lhs, const TypeInfo &rhs) const
    {
        return lhs.kind == rhs.kind && lhs.ref == rhs.ref;
    }

    bool SemanticAnalyzer::isCompatible(const TypeInfo &lhs, const TypeInfo &rhs) const
    {
        if (isSameType(lhs, rhs))
        {
            return true;
        }

        if (lhs.kind == BasicType::SUBRANGE || rhs.kind == BasicType::SUBRANGE)
        {
            TypeInfo leftBase = unwrapSubrange(lhs);
            TypeInfo rightBase = unwrapSubrange(rhs);
            return leftBase.kind == rightBase.kind;
        }

        if (lhs.kind == BasicType::STRING && rhs.kind == BasicType::STRING)
        {
            return true;
        }

        return false;
    }

    bool SemanticAnalyzer::isAssignmentCompatible(const TypeInfo &target, const TypeInfo &value, const AST::ASTNode *valueNode)
    {
        if (target.kind == BasicType::REAL && unwrapSubrange(value).kind == BasicType::INTEGER)
        {
            return true;
        }

        if (!isCompatible(target, value))
        {
            return false;
        }

        if (target.kind == BasicType::SUBRANGE)
        {
            if (auto *num = dynamic_cast<const AST::NumberNode *>(valueNode))
            {
                if (target.ref >= 0 && target.ref < static_cast<int>(subranges.size()))
                {
                    const SubrangeInfo &range = subranges[target.ref];
                    if (range.hasIntBounds)
                    {
                        return num->value >= range.lowInt && num->value <= range.highInt;
                    }
                }
            }
            if (auto *ch = dynamic_cast<const AST::CharNode *>(valueNode))
            {
                if (target.ref >= 0 && target.ref < static_cast<int>(subranges.size()))
                {
                    const SubrangeInfo &range = subranges[target.ref];
                    if (range.hasCharBounds)
                    {
                        return ch->value >= range.lowChar && ch->value <= range.highChar;
                    }
                }
            }
        }

        if (isString(target) && isString(value))
        {
            return true;
        }

        return true;
    }
} // namespace Semantic

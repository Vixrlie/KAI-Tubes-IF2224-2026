#include "symbol_table.h"

namespace Semantic
{
    SymbolTable::SymbolTable()
    {
        ensureBaseScope();
        initializePredefinedIdentifiers();
    }

    int SymbolTable::currentLevel() const
    {
        return static_cast<int>(scopeLastStack.size()) - 1;
    }

    int SymbolTable::pushScope()
    {
        ensureBaseScope();
        BtabEntry block{};
        btabEntries.push_back(block);
        scopeLastStack.push_back(0);
        scopeBlockStack.push_back(static_cast<int>(btabEntries.size()) - 1);
        return static_cast<int>(btabEntries.size()) - 1;
    }

    void SymbolTable::popScope()
    {
        if (scopeLastStack.size() <= 1)
        {
            return;
        }

        scopeLastStack.pop_back();
        scopeBlockStack.pop_back();
    }

    bool SymbolTable::registerIdentifier(const std::string &identifier,
                                         ObjectClass obj,
                                         const TypeInfo &type,
                                         int nrm,
                                         int adr,
                                         std::string &error,
                                         bool isParameter)
    {
        ensureBaseScope();
        if (existsInScope(currentScopeLast(), identifier))
        {
            error = "Identifier already declared in this scope: " + identifier;
            return false;
        }

        TabEntry entry;
        entry.identifier = identifier;
        entry.link = currentScopeLast();
        entry.obj = obj;
        entry.type = type;
        entry.nrm = nrm;
        entry.lev = currentLevel();
        entry.adr = adr;

        int blockIndex = currentScopeBlock();
        if (blockIndex >= 0 && blockIndex < static_cast<int>(btabEntries.size()))
        {
            BtabEntry &block = btabEntries[blockIndex];
            if (obj == ObjectClass::VAR)
            {
                if (isParameter)
                {
                    if (entry.adr < 0)
                    {
                        entry.adr = block.psze;
                    }
                    block.psze += typeSize(type);
                }
                else
                {
                    if (entry.adr < 0)
                    {
                        entry.adr = block.vsze;
                    }
                    block.vsze += typeSize(type);
                }
            }
        }

        int index = appendTabEntry(entry);
        setCurrentScopeLast(index);
        if (blockIndex >= 0 && blockIndex < static_cast<int>(btabEntries.size()))
        {
            BtabEntry &block = btabEntries[blockIndex];
            block.last = index;
            if (isParameter)
            {
                block.lpar = index;
            }
        }
        error.clear();
        return true;
    }

    std::optional<LookupResult> SymbolTable::lookup(const std::string &identifier) const
    {
        if (scopeLastStack.empty())
        {
            return std::nullopt;
        }

        for (int scopeIndex = static_cast<int>(scopeLastStack.size()) - 1; scopeIndex >= 0; --scopeIndex)
        {
            auto result = lookupFromLast(scopeLastStack[scopeIndex], identifier);
            if (result.has_value())
            {
                return result;
            }
        }

        return std::nullopt;
    }

    std::optional<LookupResult> SymbolTable::lookupInCurrentScope(const std::string &identifier) const
    {
        if (scopeLastStack.empty())
        {
            return std::nullopt;
        }

        return lookupFromLast(currentScopeLast(), identifier);
    }

    int SymbolTable::addArrayType(const AtabEntry &arrayInfo)
    {
        atabEntries.push_back(arrayInfo);
        return static_cast<int>(atabEntries.size()) - 1;
    }

    int SymbolTable::addBlock(const BtabEntry &blockInfo)
    {
        btabEntries.push_back(blockInfo);
        return static_cast<int>(btabEntries.size()) - 1;
    }

    const std::vector<TabEntry> &SymbolTable::tab() const
    {
        return tabEntries;
    }

    const std::vector<BtabEntry> &SymbolTable::btab() const
    {
        return btabEntries;
    }

    const std::vector<AtabEntry> &SymbolTable::atab() const
    {
        return atabEntries;
    }

    void SymbolTable::initializePredefinedIdentifiers()
    {
        std::string error;
        TypeInfo integerType{BasicType::INTEGER, 0};
        TypeInfo realType{BasicType::REAL, 0};
        TypeInfo charType{BasicType::CHAR, 0};
        TypeInfo booleanType{BasicType::BOOLEAN, 0};
        TypeInfo stringType{BasicType::STRING, 0};

        registerIdentifier("Integer", ObjectClass::TYPE, integerType, 1, 0, error);
        registerIdentifier("Real", ObjectClass::TYPE, realType, 1, 0, error);
        registerIdentifier("Char", ObjectClass::TYPE, charType, 1, 0, error);
        registerIdentifier("Boolean", ObjectClass::TYPE, booleanType, 1, 0, error);
        registerIdentifier("String", ObjectClass::TYPE, stringType, 1, 0, error);

        registerIdentifier("True", ObjectClass::CONST, booleanType, 1, 1, error);
        registerIdentifier("False", ObjectClass::CONST, booleanType, 1, 0, error);
    }

    int SymbolTable::appendTabEntry(const TabEntry &entry)
    {
        if (static_cast<int>(tabEntries.size()) < kReservedBaseIndex)
        {
            tabEntries.resize(kReservedBaseIndex);
        }

        tabEntries.push_back(entry);
        return static_cast<int>(tabEntries.size()) - 1;
    }

    bool SymbolTable::existsInScope(int scopeLast, const std::string &identifier) const
    {
        auto result = lookupFromLast(scopeLast, identifier);
        return result.has_value();
    }

    std::optional<LookupResult> SymbolTable::lookupFromLast(int scopeLast, const std::string &identifier) const
    {
        int current = scopeLast;
        while (current > 0 && current < static_cast<int>(tabEntries.size()))
        {
            const TabEntry &entry = tabEntries[current];
            if (entry.identifier == identifier)
            {
                return LookupResult{current, entry};
            }
            current = entry.link;
        }
        return std::nullopt;
    }

    void SymbolTable::ensureBaseScope()
    {
        if (scopeLastStack.empty())
        {
            if (btabEntries.empty())
            {
                btabEntries.push_back(BtabEntry{});
            }
            scopeLastStack.push_back(0);
            scopeBlockStack.push_back(0);
        }
    }

    int SymbolTable::currentScopeLast() const
    {
        return scopeLastStack.back();
    }

    int SymbolTable::currentScopeBlock() const
    {
        return scopeBlockStack.empty() ? -1 : scopeBlockStack.back();
    }

    void SymbolTable::setCurrentScopeLast(int index)
    {
        scopeLastStack.back() = index;
    }

    int SymbolTable::typeSize(const TypeInfo &type) const
    {
        if (type.kind == BasicType::ARRAY &&
            type.ref >= 0 &&
            type.ref < static_cast<int>(atabEntries.size()) &&
            atabEntries[type.ref].size > 0)
        {
            return atabEntries[type.ref].size;
        }

        return 1;
    }
}

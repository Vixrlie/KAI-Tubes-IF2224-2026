#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <optional>
#include <string>
#include <vector>

// Symbol table structures for semantic analysis (tab, btab, atab).
namespace Semantic
{
    enum class ObjectClass
    {
        CONST,
        VAR,
        TYPE,
        PROCEDURE,
        FUNCTION
    };

    enum class BasicType
    {
        INTEGER,
        REAL,
        CHAR,
        BOOLEAN,
        STRING,
        ARRAY,
        RECORD,
        SUBRANGE,
        ENUMERATED,
        NAMED,
        UNKNOWN
    };

    struct TypeInfo
    {
        BasicType kind = BasicType::UNKNOWN;
        int ref = 0;
    };

    struct TabEntry
    {
        std::string identifier;
        int link = 0;
        ObjectClass obj = ObjectClass::VAR;
        TypeInfo type;
        int nrm = 1;
        int lev = 0;
        int adr = 0;
    };

    struct BtabEntry
    {
        int last = 0;
        int lpar = 0;
        int psze = 0;
        int vsze = 0;
    };

    struct AtabEntry
    {
        int xtyp = 0;
        int etyp = 0;
        int eref = 0;
        int low = 0;
        int high = 0;
        int elsz = 0;
        int size = 0;
    };

    struct LookupResult
    {
        int index = 0;
        TabEntry entry;
    };

    class SymbolTable
    {
    public:
        SymbolTable();

        int currentLevel() const;

        int pushScope();
        void popScope();

        bool registerIdentifier(const std::string &identifier,
                                ObjectClass obj,
                                const TypeInfo &type,
                                int nrm,
                                int adr,
                                std::string &error);

        std::optional<LookupResult> lookup(const std::string &identifier) const;
        std::optional<LookupResult> lookupInCurrentScope(const std::string &identifier) const;

        int addArrayType(const AtabEntry &arrayInfo);
        int addBlock(const BtabEntry &blockInfo);

        const std::vector<TabEntry> &tab() const;
        const std::vector<BtabEntry> &btab() const;
        const std::vector<AtabEntry> &atab() const;

        void initializePredefinedIdentifiers();

    private:
        static constexpr int kReservedBaseIndex = 33;

        std::vector<TabEntry> tabEntries;
        std::vector<BtabEntry> btabEntries;
        std::vector<AtabEntry> atabEntries;
        std::vector<int> scopeLastStack;

        int appendTabEntry(const TabEntry &entry);
        bool existsInScope(int scopeLast, const std::string &identifier) const;
        std::optional<LookupResult> lookupFromLast(int scopeLast, const std::string &identifier) const;

        void ensureBaseScope();
        int currentScopeLast() const;
        void setCurrentScopeLast(int index);
    };
}

#endif

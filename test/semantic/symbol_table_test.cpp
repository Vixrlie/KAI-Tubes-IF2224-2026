#include <cassert>
#include <iostream>

#include "symbol_table.h"

/*
To run this unit test, use the following command
New-Item -ItemType Directory -Force build\semantic | Out-Null
g++ -std=c++17 -Isrc -Isrc\semantic -o build\semantic\symbol_table_test.exe src\semantic\symbol_table_test.cpp src\semantic\symbol_table.cpp
build\semantic\symbol_table_test.exe

*/

using Semantic::BasicType;
using Semantic::ObjectClass;
using Semantic::SymbolTable;
using Semantic::TypeInfo;

static void test_predefined()
{
    SymbolTable table;
    auto integer = table.lookup("Integer");
    auto real = table.lookup("Real");
    auto boolean = table.lookup("Boolean");
    auto truth = table.lookup("True");
    auto falsity = table.lookup("False");

    assert(integer.has_value());
    assert(real.has_value());
    assert(boolean.has_value());
    assert(truth.has_value());
    assert(falsity.has_value());

    assert(integer->entry.obj == ObjectClass::TYPE);
    assert(integer->entry.type.kind == BasicType::INTEGER);
    assert(real->entry.type.kind == BasicType::REAL);
    assert(boolean->entry.type.kind == BasicType::BOOLEAN);
    assert(truth->entry.obj == ObjectClass::CONST);
    assert(falsity->entry.obj == ObjectClass::CONST);
}

static void test_register_and_lookup()
{
    SymbolTable table;
    std::string error;

    TypeInfo intType{BasicType::INTEGER, 0};
    bool ok = table.registerIdentifier("x", ObjectClass::VAR, intType, 1, 0, error);
    assert(ok);
    assert(error.empty());

    auto found = table.lookup("x");
    assert(found.has_value());
    assert(found->entry.obj == ObjectClass::VAR);
    assert(found->entry.type.kind == BasicType::INTEGER);

    bool duplicate = table.registerIdentifier("x", ObjectClass::VAR, intType, 1, 0, error);
    assert(!duplicate);
    assert(!error.empty());
}

static void test_scope_shadowing()
{
    SymbolTable table;
    std::string error;

    TypeInfo intType{BasicType::INTEGER, 0};
    TypeInfo realType{BasicType::REAL, 0};

    bool ok = table.registerIdentifier("value", ObjectClass::VAR, intType, 1, 0, error);
    assert(ok);

    table.pushScope();
    bool shadow = table.registerIdentifier("value", ObjectClass::VAR, realType, 1, 0, error);
    assert(shadow);

    auto inner = table.lookup("value");
    assert(inner.has_value());
    assert(inner->entry.type.kind == BasicType::REAL);

    table.popScope();
    auto outer = table.lookup("value");
    assert(outer.has_value());
    assert(outer->entry.type.kind == BasicType::INTEGER);
}

static void test_lookup_missing()
{
    SymbolTable table;
    auto missing = table.lookup("does_not_exist");
    assert(!missing.has_value());
}

int main()
{
    test_predefined();
    test_register_and_lookup();
    test_scope_shadowing();
    test_lookup_missing();

    std::cout << "Symbol table tests passed." << std::endl;
    return 0;
}

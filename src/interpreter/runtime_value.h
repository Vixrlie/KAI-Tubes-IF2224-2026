#ifndef RUNTIME_VALUE_H
#define RUNTIME_VALUE_H

#include <string>
#include <variant>

namespace Interpreter
{
    using RuntimeValue = std::variant<int, double, char, bool, std::string>;

    bool getIntValue(const RuntimeValue &value, int &out);
    bool getDoubleValue(const RuntimeValue &value, double &out);
    bool getBoolValue(const RuntimeValue &value, bool &out);
    std::string runtimeValueToString(const RuntimeValue &value);
}

#endif

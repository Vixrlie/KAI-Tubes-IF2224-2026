#include "runtime_value.h"

#include <iomanip>
#include <sstream>

namespace Interpreter
{
    bool getIntValue(const RuntimeValue &value, int &out)
    {
        if (const auto *ptr = std::get_if<int>(&value))
        {
            out = *ptr;
            return true;
        }

        return false;
    }

    bool getDoubleValue(const RuntimeValue &value, double &out)
    {
        if (const auto *ptr = std::get_if<double>(&value))
        {
            out = *ptr;
            return true;
        }

        return false;
    }

    bool getBoolValue(const RuntimeValue &value, bool &out)
    {
        if (const auto *ptr = std::get_if<bool>(&value))
        {
            out = *ptr;
            return true;
        }

        return false;
    }

    std::string runtimeValueToString(const RuntimeValue &value)
    {
        return std::visit(
            [](const auto &val) -> std::string {
                using T = std::decay_t<decltype(val)>;
                if constexpr (std::is_same_v<T, int>)
                {
                    return std::to_string(val);
                }
                else if constexpr (std::is_same_v<T, double>)
                {
                    std::ostringstream out;
                    out << std::setprecision(15) << val;
                    return out.str();
                }
                else if constexpr (std::is_same_v<T, char>)
                {
                    return std::string(1, val);
                }
                else if constexpr (std::is_same_v<T, bool>)
                {
                    return val ? "true" : "false";
                }
                else
                {
                    return val;
                }
            },
            value);
    }
}

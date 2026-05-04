#ifndef TOKEN_STREAM_READER_H
#define TOKEN_STREAM_READER_H

#include <string>
#include <vector>

#include "token.h"

namespace TokenStreamReader
{
    bool looksLikeTokenStream(const std::string &text);
    bool tryRead(const std::string &text, std::vector<Token> &tokens, std::string &errorMessage);
}

#endif

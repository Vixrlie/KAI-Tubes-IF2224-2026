#include "token_stream_reader.h"

#include <cctype>
#include <unordered_map>
#include <utility>

namespace
{
    struct SourceLocation
    {
        int line;
        int column;
    };

    bool isLineBreak(char c)
    {
        return c == '\n' || c == '\r';
    }

    const std::unordered_map<std::string, TokenType> &tokenTypeMap()
    {
        static const std::unordered_map<std::string, TokenType> mapping = {
            {"intcon", TokenType::INTCON},
            {"realcon", TokenType::REALCON},
            {"charcon", TokenType::CHARCON},
            {"string", TokenType::STRING},
            {"notsy", TokenType::NOTSY},
            {"andsy", TokenType::ANDSY},
            {"orsy", TokenType::ORSY},
            {"plus", TokenType::PLUS},
            {"minus", TokenType::MINUS},
            {"times", TokenType::TIMES},
            {"idiv", TokenType::IDIV},
            {"rdiv", TokenType::RDIV},
            {"imod", TokenType::IMOD},
            {"eql", TokenType::EQL},
            {"neq", TokenType::NEQ},
            {"gtr", TokenType::GTR},
            {"geq", TokenType::GEQ},
            {"lss", TokenType::LSS},
            {"leq", TokenType::LEQ},
            {"lparent", TokenType::LPARENT},
            {"rparent", TokenType::RPARENT},
            {"lbrack", TokenType::LBRACK},
            {"rbrack", TokenType::RBRACK},
            {"comma", TokenType::COMMA},
            {"semicolon", TokenType::SEMICOLON},
            {"period", TokenType::PERIOD},
            {"colon", TokenType::COLON},
            {"becomes", TokenType::BECOMES},
            {"constsy", TokenType::CONSTSY},
            {"typesy", TokenType::TYPESY},
            {"varsy", TokenType::VARSY},
            {"functionsy", TokenType::FUNCTIONSY},
            {"proceduresy", TokenType::PROCEDURESY},
            {"arraysy", TokenType::ARRAYSY},
            {"recordsy", TokenType::RECORDSY},
            {"programsy", TokenType::PROGRAMSY},
            {"ident", TokenType::IDENT},
            {"beginsy", TokenType::BEGINSY},
            {"ifsy", TokenType::IFSY},
            {"casesy", TokenType::CASESY},
            {"repeatsy", TokenType::REPEATSY},
            {"whilesy", TokenType::WHILESY},
            {"forsy", TokenType::FORSY},
            {"endsy", TokenType::ENDSY},
            {"elsesy", TokenType::ELSESY},
            {"untilsy", TokenType::UNTILSY},
            {"ofsy", TokenType::OFSY},
            {"dosy", TokenType::DOSY},
            {"tosy", TokenType::TOSY},
            {"downtosy", TokenType::DOWNTOSY},
            {"thensy", TokenType::THENSY},
            {"comment", TokenType::COMMENT},
            {"unknown", TokenType::UNKNOWN},
            {"error", TokenType::ERROR},
        };
        return mapping;
    }

    class TokenStreamParser
    {
    public:
        explicit TokenStreamParser(const std::string &inputText)
            : text(inputText)
        {
            lineStarts.push_back(0);
            for (std::size_t index = 0; index < text.size(); ++index)
            {
                if (text[index] == '\n' && index + 1 < text.size())
                {
                    lineStarts.push_back(index + 1);
                }
            }
        }

        bool looksLikeTokenStream() const
        {
            std::size_t position = skipSeparators(0);
            if (position >= text.size())
            {
                return false;
            }

            std::size_t end = position;
            while (end < text.size() && std::islower(static_cast<unsigned char>(text[end])))
            {
                ++end;
            }

            if (end == position)
            {
                return false;
            }

            return tokenTypeMap().find(text.substr(position, end - position)) != tokenTypeMap().end();
        }

        bool parse(std::vector<Token> &tokens, std::string &error) const
        {
            tokens.clear();
            if (!parseSequence(0, tokens))
            {
                SourceLocation location = locationForIndex(skipSeparators(failureIndex));
                error = "Invalid token stream near line " + std::to_string(location.line) +
                        ", column " + std::to_string(location.column);
                return false;
            }
            error.clear();
            return true;
        }

    private:
        const std::string &text;
        std::vector<std::size_t> lineStarts;
        mutable std::size_t failureIndex = 0;

        std::size_t skipSeparators(std::size_t position) const
        {
            while (position < text.size() && isLineBreak(text[position]))
            {
                ++position;
            }
            return position;
        }

        SourceLocation locationForIndex(std::size_t index) const
        {
            std::size_t lineIndex = 0;
            while (lineIndex + 1 < lineStarts.size() && lineStarts[lineIndex + 1] <= index)
            {
                ++lineIndex;
            }

            return {
                static_cast<int>(lineIndex + 1),
                static_cast<int>(index - lineStarts[lineIndex] + 1),
            };
        }

        bool parseSequence(std::size_t position, std::vector<Token> &tokens) const
        {
            position = skipSeparators(position);
            if (position >= text.size())
            {
                tokens.clear();
                return true;
            }

            std::size_t nameEnd = position;
            while (nameEnd < text.size() && std::islower(static_cast<unsigned char>(text[nameEnd])))
            {
                ++nameEnd;
            }

            if (nameEnd == position)
            {
                failureIndex = position;
                return false;
            }

            const auto tokenTypeIt = tokenTypeMap().find(text.substr(position, nameEnd - position));
            if (tokenTypeIt == tokenTypeMap().end())
            {
                failureIndex = position;
                return false;
            }

            const TokenType type = tokenTypeIt->second;
            const SourceLocation location = locationForIndex(position);

            if (!tokenHasValue(type))
            {
                if (nameEnd < text.size() && !isLineBreak(text[nameEnd]))
                {
                    failureIndex = nameEnd;
                    return false;
                }

                Token token{type, "", location.line, location.column};
                std::vector<Token> rest;
                if (!parseSequence(nameEnd, rest))
                {
                    return false;
                }

                tokens.clear();
                tokens.push_back(std::move(token));
                tokens.insert(tokens.end(),
                              std::make_move_iterator(rest.begin()),
                              std::make_move_iterator(rest.end()));
                return true;
            }

            std::size_t openParen = nameEnd;
            if (openParen < text.size() && text[openParen] == ' ' &&
                openParen + 1 < text.size() && text[openParen + 1] == '(')
            {
                ++openParen;
            }

            if (openParen >= text.size() || text[openParen] != '(')
            {
                failureIndex = nameEnd;
                return false;
            }

            const std::size_t valueStart = openParen + 1;
            for (std::size_t closeParen = valueStart; closeParen < text.size(); ++closeParen)
            {
                if (text[closeParen] != ')')
                {
                    continue;
                }

                if (closeParen + 1 < text.size() && !isLineBreak(text[closeParen + 1]))
                {
                    continue;
                }

                Token token{
                    type,
                    text.substr(valueStart, closeParen - valueStart),
                    location.line,
                    location.column,
                };

                std::vector<Token> rest;
                if (!parseSequence(closeParen + 1, rest))
                {
                    continue;
                }

                tokens.clear();
                tokens.push_back(std::move(token));
                tokens.insert(tokens.end(),
                              std::make_move_iterator(rest.begin()),
                              std::make_move_iterator(rest.end()));
                return true;
            }

            failureIndex = openParen;
            return false;
        }
    };
}

bool TokenStreamReader::looksLikeTokenStream(const std::string &text)
{
    return TokenStreamParser(text).looksLikeTokenStream();
}

bool TokenStreamReader::tryRead(const std::string &text, std::vector<Token> &tokens, std::string &errorMessage)
{
    return TokenStreamParser(text).parse(tokens, errorMessage);
}

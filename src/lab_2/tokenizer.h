#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <fstream>
#include <list>

namespace Tokenizer {
    enum class TokenType;

    struct Token {
        Token(int currentType, std::string value) : currentType(currentType), value(std::move(value)) {}

        int         currentType;
        std::string value;
    };

    enum class IntegerNumType {
        INT = 0,
        LONG,
        LLONG,
        UINT,
        ULONG,
        ULLONG,
        ERROR
    };

    std::string toString(IntegerNumType type);

    enum class State {
        ERROR = -1,
        DEFAULT = 0,
        IN_STRING,
        IN_STRING_SCREENING,
        IN_SYMBOL,
        IN_SYMBOL_SCREENING,
        FIRST_COM_SYMBOL,
        IN_COMMENT,
        IN_ONE_LINE_COMMENT,
        FIRST_UNCOM_SYMBOL,
        DEC_PREFIX,
        OCT_PREFIX,
        HEX_PREFIX,
        OCTAL_NUM,
        DECIMAL_NUM,
        HEXADEC_NUM,
        LDECIMAL_NUM,
        LLDECIMAL_NUM,
        UDECIMAL_NUM,
        ULDECIMAL_NUM,
        ULLDECIMAL_NUM
    };

    std::list<Tokenizer::Token> tokenize(std::ifstream& in);
    // std::list<Tokenizer::Token> tokenizeSymbol(const char c);

    void writeTokens(std::ostream &out, const std::list<Token> &tokens);
}

#endif /* TOKENIZER_H */
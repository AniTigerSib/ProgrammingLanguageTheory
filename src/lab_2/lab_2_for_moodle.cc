#include <cctype>
#include <fstream>
#include <iostream>
#include <list>

#define isoctal(x)\
  ((x) >= '0' && (x) <= '7')
#define ishex(x)\
  ((x) >= '0' && (x) <= '9') || ((x) >= 'a' && (x) <= 'f') || ((x) >= 'A' && (x) <= 'F')
#define isscanning(x)\
  (std::isdigit(x) || std::isalpha(x))

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

    void writeTokens(std::ostream &out, const std::list<Token> &tokens);
}

int main(int argc, char ** argv) {
    std::ifstream input;
    std::ofstream output;
    std::string srcFilename;
    std::string dstFilename;

    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <input_file>" << std::endl;
        return 1;
    }

    srcFilename = std::string("test_data/") + argv[1];
    dstFilename = std::string("test_result/") + "report_" + argv[1] + ".txt";
    input.open(srcFilename, std::ios::in);
    if (!input.is_open()) {
        std::cerr << "Error opening input file: " << srcFilename << std::endl;
        return 1;
    }

    output.open(dstFilename, std::ios::trunc);
    if (!output.is_open()) {
        std::cerr << "Error opening output file: " << dstFilename << std::endl;
        input.close();
        return 1;
    }

    std::list<Tokenizer::Token> tokens = Tokenizer::tokenize(input);
    for (const auto &token : tokens) {
        output << token.value << "  " << Tokenizer::toString(static_cast<Tokenizer::IntegerNumType>(token.currentType)) << std::endl;
    }

    input.close();
    output.close();
    return 0;
}

std::string Tokenizer::toString(IntegerNumType type) {
    switch (type) {
    case IntegerNumType::INT:
        return std::move("int");
    case IntegerNumType::LONG:
        return std::move("long");
    case IntegerNumType::LLONG:
        return std::move("long long");
    case IntegerNumType::UINT:
        return std::move("unsigned int");
    case IntegerNumType::ULONG:
        return std::move("unsigned long");
    case IntegerNumType::ULLONG:
        return std::move("unsigned long long");
    case IntegerNumType::ERROR:
        return std::move("ERROR");
    }
    return std::move("NOT IMPLEMENTED");
}

void standartHandler(Tokenizer::State& state, std::string& token, const char c) {
    if (c == '\"') {
        state = Tokenizer::State::IN_STRING;
    } else if (c == '\'') {
        state = Tokenizer::State::IN_SYMBOL;
    } else if (c == '/') {
        state = Tokenizer::State::FIRST_COM_SYMBOL;
    } else if (c == '0') {
        state = Tokenizer::State::OCT_PREFIX;
        token += c;
    } else if (std::isdigit(c)) {
        state = Tokenizer::State::DEC_PREFIX;
        token += c;
    } else {
        state = Tokenizer::State::DEFAULT;
    }
}

std::list<Tokenizer::Token> Tokenizer::tokenize(std::ifstream& in) {
    Tokenizer::State state = State::DEFAULT;
    std::list<Token> tokens;
    std::string tokenString;
    int c = ' ';
    
    while (1) {
        if (c == EOF) break;
        c = in.get();
        
        if (state == State::DEFAULT) {
            standartHandler(state, tokenString, c);
        } else if (state == State::ERROR) {
            if (!isscanning(c)) {
                state = State::DEFAULT;
                tokens.push_back(Tokenizer::Token(static_cast<int>(IntegerNumType::ERROR), std::move(tokenString)));
                tokenString.clear();
                standartHandler(state, tokenString, c);
            } else {
                tokenString += c;
            }
        } else if (state == State::IN_STRING) {
            if (c == '\\') {
                state = State::IN_STRING_SCREENING;
            } else if (c == '\"') {
                state = State::DEFAULT;
            }
        } else if (state == State::IN_STRING_SCREENING) {
            state = State::IN_STRING;
        } else if (state == State::IN_SYMBOL) {
            if (c == '\\') {
                state = State::IN_SYMBOL_SCREENING;
            } else if (c == '\'') {
                state = State::DEFAULT;
            }
        } else if (state == State::IN_SYMBOL_SCREENING) {
            state = State::IN_SYMBOL;
        } else if (state == State::FIRST_COM_SYMBOL) {
            if (c == '*') {
                state = State::IN_COMMENT;
                continue;
            } else if (c == '/') {
                state = State::IN_ONE_LINE_COMMENT;
                continue;
            }
            state = State::DEFAULT;
            standartHandler(state, tokenString, c);
        } else if (state == State::IN_COMMENT) {
            if (c == '*')
                state = State::FIRST_UNCOM_SYMBOL;
        } else if (state == State::IN_ONE_LINE_COMMENT) {
            if (c == '\n' || c == '\r')
                state = State::DEFAULT;
        } else if (state == State::FIRST_UNCOM_SYMBOL) {
            if (c == '/') {
                state = State::DEFAULT;
                continue;
            } else if (c != '*') {
                state = State::IN_COMMENT;
            }
        } else if (state == State::DEC_PREFIX) {
            if (std::isdigit(c)) {
                state = State::DECIMAL_NUM;
                tokenString += c;
            } else {
                if (!isscanning(c)) {
                    // if (state == State::OCTAL_NUM) {
                    //     state = State::ERROR;
                    //     continue;
                    // } else {
                    state = State::DEFAULT;
                    tokens.push_back(Tokenizer::Token(static_cast<int>(IntegerNumType::INT), std::move(tokenString)));
                    tokenString.clear();
                    standartHandler(state, tokenString, c);
                    continue;
                    // }
                } else {
                    state = State::ERROR;
                    tokenString += c;
                }
            }
        } else if (state == State::OCT_PREFIX) {
            if (isoctal(c)) {
                state = State::OCTAL_NUM;
            } else if (c == 'x') {
                state = State::HEX_PREFIX;
            } else {
                if (!isscanning(c)) {
                    // if (state == State::OCTAL_NUM) {
                    //     state = State::ERROR;
                    //     continue;
                    // } else {
                    state = State::DEFAULT;
                    tokens.push_back(Tokenizer::Token(static_cast<int>(IntegerNumType::INT), std::move(tokenString)));
                    tokenString.clear();
                    standartHandler(state, tokenString, c);
                    // }
                } else {
                    state = State::ERROR;
                }
                continue;
            }
            tokenString += c;
        } else if (state == State::HEX_PREFIX) {
            if (ishex(c)) {
                state = State::HEXADEC_NUM;
                tokenString += c;
            } else {
                if (!isscanning(c)) {
                    state = State::DEFAULT;
                    tokens.push_back(Tokenizer::Token(static_cast<int>(IntegerNumType::ERROR), std::move(tokenString)));
                    tokenString.clear();
                    standartHandler(state, tokenString, c);
                } else {
                    state = State::ERROR;
                    tokenString += c;
                }
            }
        } else if (state == State::OCTAL_NUM) {
            if (c == 'u') {
                state = State::UDECIMAL_NUM;
            } else if (c == 'l') {
                state = State::LDECIMAL_NUM;
            } else if (!isoctal(c)) {
                if (!isscanning(c)) {
                    state = State::DEFAULT;
                    tokens.push_back(Tokenizer::Token(static_cast<int>(IntegerNumType::INT), std::move(tokenString)));
                    tokenString.clear();
                    standartHandler(state, tokenString, c);
                    continue;
                } else {
                    state = State::ERROR;
                }
            }
            tokenString += c;
        } else if (state == State::HEXADEC_NUM) {
            if (c == 'u') {
                state = State::UDECIMAL_NUM;
            } else if (c == 'l') {
                state = State::LDECIMAL_NUM;
            } else if (!ishex(c)) {
                if (!isscanning(c)) {
                    state = State::DEFAULT;
                    tokens.push_back(Tokenizer::Token(static_cast<int>(IntegerNumType::INT), std::move(tokenString)));
                    tokenString.clear();
                    standartHandler(state, tokenString, c);
                    continue;
                } else {
                    state = State::ERROR;
                }
            }
            tokenString += c;
        } else if (state == State::DECIMAL_NUM) {
            // std::cout << tokenString << std::endl;
            if (c == 'u') {
                state = State::UDECIMAL_NUM;
            } else if (c == 'l') {
                state = State::LDECIMAL_NUM;
            } else if (!std::isdigit(c)) {
                if (!isscanning(c)) {
                    state = State::DEFAULT;
                    tokens.push_back(Tokenizer::Token(static_cast<int>(IntegerNumType::INT), std::move(tokenString)));
                    tokenString.clear();
                    standartHandler(state, tokenString, c);
                    continue;
                } else {
                    state = State::ERROR;
                }
            }
            tokenString += c;
        } else if (state == State::UDECIMAL_NUM) {
            if (c == 'l') {
                state = State::ULDECIMAL_NUM;
            } else {
                if (!isscanning(c)) {
                    state = State::DEFAULT;
                    tokens.push_back(Tokenizer::Token(static_cast<int>(IntegerNumType::UINT), std::move(tokenString)));
                    tokenString.clear();
                    standartHandler(state, tokenString, c);
                    continue;
                } else {
                    state = State::ERROR;
                }
            }
            tokenString += c;
        } else if (state == State::LDECIMAL_NUM) {
            if (c == 'l') {
                state = State::LLDECIMAL_NUM;
            } else {
                if (!isscanning(c)) {
                    state = State::DEFAULT;
                    tokens.push_back(Tokenizer::Token(static_cast<int>(IntegerNumType::LONG), std::move(tokenString)));
                    tokenString.clear();
                    standartHandler(state, tokenString, c);
                    continue;
                } else {
                    state = State::ERROR;
                }
            }
            tokenString += c;
        } else if (state == State::LLDECIMAL_NUM) {
            if (!isscanning(c)) {
                state = State::DEFAULT;
                tokens.push_back(Tokenizer::Token(static_cast<int>(IntegerNumType::LLONG), std::move(tokenString)));
                tokenString.clear();
                standartHandler(state, tokenString, c);
            } else {
                state = State::ERROR;
                tokenString += c;
            }
        } else if (state == State::ULDECIMAL_NUM) {
            if (c == 'l') {
                state = State::ULLDECIMAL_NUM;
            } else {
                if (!isscanning(c)) {
                    state = State::DEFAULT;
                    tokens.push_back(Tokenizer::Token(static_cast<int>(IntegerNumType::ULONG), std::move(tokenString)));
                    tokenString.clear();
                    standartHandler(state, tokenString, c);
                    continue;
                } else {
                    state = State::ERROR;
                }
            }
            tokenString += c;
        } else if (state == State::ULLDECIMAL_NUM) {
            if (!isscanning(c)) {
                state = State::DEFAULT;
                tokens.push_back(Tokenizer::Token(static_cast<int>(IntegerNumType::ULLONG), std::move(tokenString)));
                tokenString.clear();
                standartHandler(state, tokenString, c);
            } else {
                state = State::ERROR;
                tokenString += c;
            }
        }
    }

    return tokens;
}
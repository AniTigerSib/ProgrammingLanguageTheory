#include <cctype>
#include <cstring>
#include <fstream>
#include <iostream>
#include <list>
#include <sstream>
#include <string>
#include <vector>

enum Relation {
    kNone = ' ',
    kBefore = '<',
    kTogether = '=',
    kAfter = '>',
    kDual = '%'
};

struct Token {
    char type;
    std::string value;
    Relation relation;
    std::vector<int> triads;
    int thriadNum;
    Token(char c = 0, std::string val = ""): type(c), value(val), relation(kNone), thriadNum(-1) {
        triads.push_back(-1);
    };
    void setTriadNum(int n) {
        thriadNum = n;
        triads[0] = n;
    }
    std::string toString() const;
};

std::string Token::toString() const {
    std::stringstream res;
    res << "(";
    res << (type? type : ' ') << ", " << value;
    res << ", " << (relation? static_cast<char>(relation) : ' ');
    res << ", " << (thriadNum >= 0? thriadNum : ' ');
    res << ")";
    return res.str();
}

void Error(const std::string& message, const char param) {
    throw std::runtime_error(message + param);
}
void Error(const std::string& message) {
    throw std::runtime_error(message);
}

class Scanner {
public:
    explicit Scanner(std::ifstream& in) : is_(in), current_(' ') {}
    void Get() { current_ = is_.get(); }
    Token GetToken();
    // std::list<Token> Tokenize();

private:
    std::ifstream& is_;
    int current_;

    bool IsDigit(const int c) { return c == '0' || c == '1'; }
    bool IsNative(const int c) { return strchr("+-*()[],", c) != nullptr; }
    bool IsAlpha(const int c) { return c >= 'a' && c <= 'z' || c == '_'; }
};

Token Scanner::GetToken() {
    while (isspace(current_) || current_ == '\n') Get();

    if (IsAlpha(current_)) {
        std::string identifier;
        do {
            identifier += static_cast<char>(current_);
            Get();
        } while (IsDigit(current_));
        return {'I', identifier};
    } else if (current_ == '#') {
        std::string constNum;
        Get();
        while (IsDigit(current_)) {
            constNum += static_cast<char>(current_);
            Get();
        }
        if (constNum.empty())
            Error("Invalid constant symbol: ", current_);
        return {'C', constNum};
    } else if (IsNative(current_)) {
        int last = current_;
        Get();
        return {static_cast<char>(last)};
    } else if (current_ == EOF) {
        return {'#'};
    }

    Error("Invalid symbol: ", current_);
}

struct Rule {
    char left;
    std::string right;
};

class Syntaxer {
public:
    explicit Syntaxer(Scanner& scanner) :
        scanner_(scanner),
        operationCounter_(0) {}
    void Parse();

private:
    Scanner& scanner_;
    int operationCounter_;
    std::list<Token> tokens_;
    std::vector<Token> buffer_;

    void PrintStack() {
        for (const auto& token : tokens_) {
            std::cout << static_cast<char>(token.relation) << " " << token.type << " ";
            // std::cout << token.toString() << " ";
        }
        std::cout << std::endl;
    }
    void PrintBuffer() {
        std::cout << "Buffer: ";
        for (const auto& token : buffer_) {
            std::cout << token.type << " ";
            // std::cout << token.toString() << " ";
        }
        std::cout << "\n";
    }

    bool CompareBufferToRule(const Rule& rule) {
        if (buffer_.size() != rule.right.size())
            return false;

        for (size_t i = 0; i < buffer_.size(); ++i) {
            if (buffer_[i].type != rule.right[i])
                return false;
        }
        return true;
    }

    int CharIndex(char c) {
        int res = alphabet_.find(c);
        if (res == std::string::npos)
            Error("Unknown symbol: ", c);
        return res;
    }

    int WriteVar(std::string &name) {
        std::cout << operationCounter_ << ":\t\tV(" << name << ", ∅)\n";
        return operationCounter_++;
    }
    int WriteConst(std::string &value) {
        std::cout << operationCounter_ << ":\t\tC(" << value << ", ∅)\n";
        return operationCounter_++;
    }
    int WriteOperation(char operation, int left, Token &right) {
        std::cout << operationCounter_ << ":\t\t" << operation << "(" << left << ", ^" << right.thriadNum << ")\n";
        return operationCounter_++;
        // return operationCounter_++;
    }
    int WriteOperation(char operation, int left, int right) {
        std::cout << operationCounter_ << ":\t\t" << operation << "(^" << left << ", ^" << right << ")\n";
        return operationCounter_++;
        // return operationCounter_++;
    }
    void WriteVorC(Token &token) {
        if (token.type == 'I') {
            token.setTriadNum(WriteVar(token.value));
        } else if (token.type == 'C') {
            token.setTriadNum(WriteConst(token.value));
        }
    }

    int PerformOperation(char operation, Token &token);

    Relation GetRel(char x, char y) {
        Relation rel = static_cast<Relation>(matrix_[CharIndex(x)][CharIndex(y)]);
        if (rel == kNone)
            Error(std::string("Relation not found between ") + x + std::string(" and ") + y);
        return rel;
    }

    Token g();

    const std::string alphabet_ = "LSET[]()-+*,IC#";
    const std::vector<Rule> rules_ = 
    {
        {'_', "#L#"},
        {'L', "LS"},
            {'L', "S"},
        {'S', "I[E]"},
        {'E', "-(T)"},
            {'E', "+(T)"},
            {'E', "*(T)"},
            {'E', "S"},
            {'E', "I"},
            {'E', "C"},
        {'T', "T,E"},
        {'T', "E"}
        /*... */
    };
    const std::vector<std::string> matrix_ = {
    /*           LSET[]()-+*,IC# */
    /*L*/   " =          < =",
    /*S*/   "     > >   >> >",
    /*E*/   "     = >   >   ",
    /*T*/   "       =   =   ",
    /*[*/   " <=     <<< << ",
    /*]*/   "     > >   >> >",
    /*(*/   " <<%    <<< << ",
    /*)*/   "     > >   >   ",
    /*-*/   "      =        ",
    /*+*/   "      =        ",
    /* * */"      =        ",
    /*,*/  " <=     <<< << ",
    /*I*/  "    => >   >   ",
    /*C*/  "     > >   >   ",
    /*#*/  "%<          <  "
    };
};

int Syntaxer::PerformOperation(char operation, Token &token) {
    int res = 0;
    switch (operation) {
        case '-':
            if (token.triads.size() == 1) {
                return WriteOperation('-', 0, token);
            }
            for (int i = 0; i < token.triads.size() - 1; i++) {
                res = WriteOperation('-', token.triads[i], token.triads[i + 1]);
            }
            break;
        case '+':
            if (token.triads.size() == 1) {
                return WriteOperation('+', 0, token);
            }
            for (int i = 0; i < token.triads.size() - 1; i++) {
                res = WriteOperation('+', token.triads[i], token.triads[i + 1]);
            }
            break;
        case '*':
            if (token.triads.size() == 1) {
                return WriteOperation('*', 1, token);
            }
            for (int i = 0; i < token.triads.size() - 1; i++) {
                res = WriteOperation('*', token.triads[i], token.triads[i + 1]);
            }
            break;
        default:
            Error("Invalid operation");
    }
    return res;
}

Token Syntaxer::g() {
    Token token;
    while (tokens_.back().relation == kTogether || tokens_.back().relation == kDual) {
        buffer_.insert(buffer_.begin(), tokens_.back());
        // PrintBuffer();
        tokens_.pop_back();
        // PrintStack();
    }
    buffer_.insert(buffer_.begin(), tokens_.back());
    // PrintBuffer();
    tokens_.pop_back();
    // PrintStack();

    // bool flag = false;
    while (buffer_.size() > 0) {
        for (int i = 0; i < rules_.size(); i++) {
            bool flag = CompareBufferToRule(rules_[i]);
            if (flag) {
                if (rules_[i].left == 'L' && tokens_.back().type != '#')
                    continue;
                token.type = rules_[i].left;

                if (i == 7 || i == 8 || i == 9 || i == 11) {
                    token.setTriadNum(buffer_[0].thriadNum);
                }
                if (i == 3) {
                    // std::cout << buffer_[0].toString() << std::endl;
                    // std::cout << buffer_[2].toString() << std::endl;
                    token.setTriadNum(WriteOperation('=', buffer_[0].thriadNum, buffer_[2].thriadNum));
                }
                if (i == 10) {
                    token.setTriadNum(buffer_[0].thriadNum);
                    token.triads.assign(buffer_[0].triads.begin(), buffer_[0].triads.end());
                    token.triads.push_back(buffer_[2].thriadNum);
                }
                if (i == 4) {
                    token.setTriadNum(PerformOperation('-', buffer_[2]));
                }
                if (i == 5) {
                    token.setTriadNum(PerformOperation('+', buffer_[2]));
                }
                if (i == 6) {
                    token.setTriadNum(PerformOperation('*', buffer_[2]));
                }

                return token;
            }
        }

        tokens_.push_back(buffer_[0]);
        // PrintStack();
        buffer_.erase(buffer_.begin());
        // PrintBuffer();
    }

    Error("Rule not found!");
}

void Syntaxer::Parse() {
    tokens_.push_back(Token('#'));
    // PrintStack();
    Token token, cooked;
    
    std::cout << "Parsing has started!\n";

    do {
        token = scanner_.GetToken();
        
        WriteVorC(token);

        token.relation = GetRel(tokens_.back().type, token.type);
        // std::cout << static_cast<char>(token.relation) << " " << token.type << std::endl;

        if (token.relation == kBefore || token.relation == kTogether || token.relation == kDual) {
            tokens_.push_back(token);
            // PrintStack();
        } else {
            while (token.relation == kAfter) {
                cooked = g();
                // PrintBuffer();
                // std::cout << cooked.toString() << std::endl;
                buffer_.clear();

                cooked.relation = GetRel(tokens_.back().type, cooked.type);
                tokens_.push_back(cooked);
                // PrintStack();
                token.relation = GetRel(cooked.type, token.type);

                if (token.relation == kBefore || token.relation == kTogether || token.relation == kDual) {
                    tokens_.push_back(token);
                    // PrintStack();
                }
            }
        }
    } while (tokens_.back().type != '#');

    std::cout << "Parsing complete!\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <file>\n";
        return 1;
    }

    std::ifstream in(argv[1], std::ifstream::in);

    if (!in.is_open()) {
        std::cout << "Cannot open file: " << argv[1] << "\n";
        return 1;
    }

    Scanner scanner(in);
    Syntaxer syntaxer(scanner);
    syntaxer.Parse();

    return 0;
}


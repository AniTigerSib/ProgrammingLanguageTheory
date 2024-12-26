#include <fstream>
#include <iostream>
#include <list>
#include <ostream>
#include <string>

class Parser {
public:
    struct Variable {
        std::string name;
        std::string value;
    };

    Parser(std::ifstream& ifs) : ifs_(ifs), index_(0), errFound_(false) {}

    bool StartParsing();
    // void parse(std::string);
    void PrintVariables();

    Variable ProcBase();

    // Name of the variable
    std::string ProcI();
    std::string ProcK();

    // Value of the variable
    std::string ProcE();
    std::string ProcT();
    std::string ProcR() { return std::move(ProcK()); }


private:
    std::ifstream& ifs_;

    std::string string_;
    int index_;
    std::list<Variable> variables_; // TODO: Добавить проверку на наличие переменной в списке

    Variable* Find(std::string& name) {
        for (auto& var : variables_) {
            if (var.name == name)
                return &var;
        }
        return nullptr;
    }
    Variable* Find(Variable& invar) {
        for (auto& var : variables_) {
            if (var.name == invar.name)
                return &var;
        }
        return nullptr;
    }
    void Set(std::string& name, std::string& value) {
        auto* var = Find(name);
        if (var)
            var->value = value;
        else
            variables_.emplace_back(Variable{name, value});
    }
    void Set(Variable& invar) {
        auto* var = Find(invar);
        if (var)
            var->value = invar.value;
        else
            variables_.push_back(invar);
    }

    bool errFound_;

    int Peek() { return (index_ < string_.length()) ? string_[index_++] : -1; }
    void Back() { index_--; }
};

// MAIN PROGRAM ////////////////////////////////////////////////////////////////////

int main(int argc, char ** argv) {
    std::ifstream ifs;
    std::string srcFilename;

    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <input_file>" << std::endl;
        return 1;
    }

    srcFilename = argv[1];
    ifs.open(srcFilename, std::ios::in);
    if (!ifs.is_open()) {
        std::cerr << "Error opening input file: " << srcFilename << std::endl;
        return 1;
    }

    Parser parser(ifs);
    if (!parser.StartParsing())
        parser.PrintVariables();

    ifs.close();

    return 0;
}

// IMPLEMENTATION //////////////////////////////////////////////////////////////////

bool Parser::StartParsing() {
    std::cout << "Parsing has started!" << std::endl;
    int line = 1;
    while (!errFound_ && ifs_.good()) {
        std::getline(ifs_, string_);
        // std::cout << string_ << std::endl;
        try {
            auto var = std::move(ProcBase());
            Set(var);
        } catch (const std::runtime_error& e) {
            std::cout << "In line " + std::to_string(line) + " - " << "Error found: " << e.what() << std::endl;
            errFound_ = true;
        }
        line++;
        index_ = 0;
    }
    if (errFound_) {
        std::cout << "Error found. Parsing aborted!" << std::endl;
    } else {
        std::cout << "Parsing complete!" << std::endl;
    }

    return errFound_;
}

void Parser::PrintVariables() {
    if (!errFound_) {
        if (variables_.empty()) {
            std::cout << "No variables defined." << std::endl;
        } else {
            std::cout << "Variables:" << std::endl;
        }
        for (auto& var : variables_)
            std::cout << var.name << " = " << var.value << std::endl;
    }
}

Parser::Variable Parser::ProcBase() {
    Variable var;
    
    var.name = ProcI();
    if (var.name.empty()) {
        throw std::runtime_error("Missing variable name");
    }
    if (Peek() == '(') {
        var.value = ProcE();
        if (var.value.empty()) {
            throw std::runtime_error("Missing variable value");
        }
        if (Peek() != ')') {
            throw std::runtime_error("Missing closing parenthesis");
        }
    } else {
        throw std::runtime_error("Missing opening parenthesis");
    }

    return std::move(var);
}

std::string Parser::ProcI() {
    std::string value;
    int c = Peek();
    if (c >= 'a' && c <= 'z' || c == '_') {
        value += c;
        value += ProcK();
    } else {
        Back();
    }
    // std::cout << "I " << value << std::endl;
    return std::move(value);
}

std::string Parser::ProcK() {
    std::string value;
    int c = Peek();
    // std::cout << static_cast<char>(c) << std::endl;
    while (c == '0' || c == '1') {
        value += c;
        c = Peek();
    }
    Back();
    return std::move(value);
}

std::string Parser::ProcE() {
    std::string value;
    std::string temp;
    int c = Peek();
    int index = index_;

    if (c == '-') {
        temp = ProcE();
        if (temp.empty())
            throw std::runtime_error("Expected symbol E in " + std::to_string(index_) + " position");
        value = "-" + temp;
    } else if (c == '+') {
        if (Peek() == '(') {
            temp = ProcT();
            if (temp.empty()) {
                throw std::runtime_error("Expected symbol T in" + std::to_string(index_) + " position");
            }
            if (Peek() != ')') {
                throw std::runtime_error("Expected closing parenthesis, got: " + std::string(1, static_cast<char>(c)));
            }
        } else {
            throw std::runtime_error("Expected opening parenthesis, got: " + std::string(1, static_cast<char>(c)));
        }
        value = "+(" + temp + ")";
    } else if (c == '*') {
        if (Peek() == '(') {
            temp = ProcT();
            if (temp.empty()) {
                throw std::runtime_error("Expected symbol T in " + std::to_string(index_) + " position");
            }
            if (Peek() != ')') {
                throw std::runtime_error("Expected closing parenthesis, got: " + std::string(1, static_cast<char>(c)));
            }
        } else {
            throw std::runtime_error("Expected opening parenthesis, got: " + std::string(1, static_cast<char>(c)));
        }
        value = "*(" + temp + ")";
    } else if (c == '#') {
        c = Peek();
        while (c == '0' || c == '1') {
            temp += c;
            c = Peek();
        }
        if (temp.empty())
            throw std::runtime_error("Expected symbol R in " + std::to_string(index_) + " position");
        Back();
        value = "#" + temp;
    } else {
        Back();
        temp = ProcI();
        if (!temp.empty()) {
            if (Peek() == '(') {
                value = ProcE();
                if (value.empty()) {
                    throw std::runtime_error("Expected symbol E in " + std::to_string(index_) + " position");
                }
                c = Peek();
                if (c != ')') {
                    throw std::runtime_error("Expected closing parenthesis, got: " + std::string(1, static_cast<char>(c)));
                }
                Set(temp, value);
            } else {
                Back();
                auto var = Find(temp);
                if (var) {
                    value = var->value;
                } else {
                    throw std::runtime_error("Undefined variable: " + temp); // WHAT????
                }
            }
        }
    }

    if (value.empty())
        index_ = index;

    return std::move(value);
}

std::string Parser::ProcT() {
    std::string value;
    std::string temp;
    int index = index_;

    value = ProcE();
    // std::cout << "Proc " << value << std::endl;
    if (!value.empty()) {
        char c = Peek();
        if (c == ',') {
            temp = ProcT();
            if (temp.empty())
                throw std::runtime_error("Expected symbol T in " + std::to_string(index_) + " position");
            value += "," + temp;
        } else {
            Back();
        }
    }

    if (value.empty())
        index_ = index;

    // std::cout << value << std::endl;

    return std::move(value);
}

// std::string Parser::ProcS() {
//     std::string res;
//     std::string name;
//     std::string value;
//     int c = '\0';
//     int index = index_;

//     name = ProcI();
//     if (!name.empty()) {
//         c = Peek();
//         if (c == '(') {
//             value = ProcE();
//             c = Peek();
//         } else {
//             Back();
//         }
//     }
//     if (!value.empty()) {
//         if (c == ')')
//             value += name + '(' + value + ')';
//         else {
//             value = "";
//             Back();
//         }
//     }

//     if (value.empty())
//         index_ = index;

//     return std::move(res);
// }

// std::string Parser::ProcM() {
//     std::string value;
//     int index = index_;

//     value = ProcI();
//     if (value.empty()) {
//         value = ProcC();
//         // std::cout << "HAHAHA" << std::endl;
//     }
    
//     if (value.empty())
//         index_ = index;

//     return std::move(value);
// }

// std::string Parser::ProcC() {
//     std::string value;
//     char c = Peek();
//     int index = index_;

//     if (c == '#') {
//         value = ProcR();
//         if (!value.empty())
//             value = '#' + value;
//         else
//             throw std::runtime_error("Expected symbol R, got: " + std::to_string(c));
//     } else {
//         Back();
//     }

//     if (value.empty())
//         index_ = index;

//     return std::move(value);
// }
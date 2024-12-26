#include <bitset>
#include <cmath>
#include <fstream>
#include <iostream>
#include <list>
#include <ostream>
#include <string>

class Parser {
public:
    struct Variable {
        std::string name;
        int value;
    };

    struct Value {
        bool hasValue;
        int value;
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
    Value ProcE();
    std::list<Value> ProcT();
    std::string ProcR() { return std::move(ProcK()); }


private:
    std::ifstream& ifs_;

    std::string string_;
    int index_;
    std::list<Variable> variables_;

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
    void Set(std::string& name, int value) {
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

    constexpr const static auto Count = 
        [](std::list<Value>& values, bool isSum) {
            int res = values.back().value;
            values.pop_back();
            for (auto val : values) {
                if (isSum)
                    res += val.value;
                else
                    res *= val.value;
            }
            return res;
        };

    int Peek() { return (index_ < string_.length()) ? string_[index_++] : -1; }
    void Back() { index_--; }
};

// SERVICE FUNCTIONS ///////////////////////////////////////////////////////////////

int stringToBinary(std::string& value) {
    int result = 0;
    for (int i = 0; i < value.size(); i++) {
        result += (value[i] - '0') * pow(2, value.size() - i - 1);
    //   std::cout << value[i] << std::endl;
    }
    return result;
}

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
            std::cout << var.name << " = " << std::bitset<8> (var.value) << std::endl;
    }
}

Parser::Variable Parser::ProcBase() {
    Variable var;
    
    var.name = ProcI();
    if (var.name.empty()) {
        throw std::runtime_error("Missing variable name");
    }
    if (Peek() == '(') {
        auto value = ProcE();
        if (!value.hasValue) {
            throw std::runtime_error("Missing variable value");
        }
        if (Peek() != ')') {
            throw std::runtime_error("Missing closing parenthesis");
        }
        var.value = value.value;
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

Parser::Value Parser::ProcE() {
    Value value{false, 0};
    int c = Peek();
    int index = index_;

    if (c == '-') { // TODO: минус унарный
        Value temp = ProcE();
        if (!temp.hasValue)
            throw std::runtime_error("Expected symbol E in " + std::to_string(index_) + " position");
        value = {true, -temp.value};
    } else if (c == '+') {
        if (Peek() == '(') {
            std::list<Value> values = std::move(ProcT());
            if (values.empty()) {
                throw std::runtime_error("Expected symbol T in" + std::to_string(index_) + " position");
            }
            if (Peek() != ')') {
                throw std::runtime_error("Expected closing parenthesis, got: " + std::string(1, static_cast<char>(c)));
            }
            value = {true, Count(values, true)};
        } else {
            throw std::runtime_error("Expected opening parenthesis, got: " + std::string(1, static_cast<char>(c)));
        }
    } else if (c == '*') {
        if (Peek() == '(') {
            std::list<Value> values = std::move(ProcT());
            if (values.empty()) {
                throw std::runtime_error("Expected symbol T in" + std::to_string(index_) + " position");
            }
            if (Peek() != ')') {
                throw std::runtime_error("Expected closing parenthesis, got: " + std::string(1, static_cast<char>(c)));
            }
            value = {true, Count(values, false)};
        } else {
            throw std::runtime_error("Expected opening parenthesis, got: " + std::string(1, static_cast<char>(c)));
        }
    } else if (c == '#') {
        std::string temp;
        c = Peek();
        while (c == '0' || c == '1') {
            temp += c;
            c = Peek();
        }
        if (temp.empty())
            throw std::runtime_error("Expected symbol R in " + std::to_string(index_) + " position");
        Back();
        value = {true, stringToBinary(temp)};
    } else {
        std::string name;
        Back();
        name = ProcI();
        if (!name.empty()) {
            if (Peek() == '(') {
                Value val = ProcE();
                if (!val.hasValue) {
                    throw std::runtime_error("Expected symbol E in " + std::to_string(index_) + " position");
                }
                c = Peek();
                if (c != ')') {
                    throw std::runtime_error("Expected closing parenthesis, got: " + std::string(1, static_cast<char>(c)));
                }
                Set(name, val.value);
                value = {true, val.value};
            } else {
                Back();
                auto var = Find(name);
                if (var) {
                    value = {true, var->value};
                } else {
                    throw std::runtime_error("Undefined variable: " + name);
                }
            }
        }
    }

    if (!value.hasValue)
        index_ = index;

    return std::move(value);
}

std::list<Parser::Value> Parser::ProcT() {
    std::list<Value> values;
    Value value{false, 0};
    int index = index_;

    value = ProcE();
    if (value.hasValue) {
        if (Peek() == ',') {
            values = std::move(ProcT());
            if (values.empty())
                throw std::runtime_error("Expected symbol T in " + std::to_string(index_) + " position");
            values.push_front(value);
        } else {
            Back();
            values.push_back(value);
        }
    } else {
        index_ = index;
    }

    return std::move(values);
}
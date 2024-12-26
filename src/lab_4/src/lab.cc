#include <cmath>
#include <fstream>
#include <iostream>
#include <list>
#include <string>

// THRIAD DEFINITIONS //////////////////////////////////////////////////////////////

class Thriad;

struct Operand {
    Thriad* thriad_;
    std::string value_;
    std::string ToString() const;
};

class Thriad {
public:
    Thriad(int index, char op, Operand leftOperand, Operand rightOperand):
        index_(index), operator_(op), leftOperand_(leftOperand), rightOperand_(rightOperand) {};

    std::string ToString() const {
        return static_cast<char>(operator_) + std::string("(") + leftOperand_.ToString() + ", " + rightOperand_.ToString() + ")";
    }

    int Index() { return index_; };
    char Operator() { return operator_; };
    Operand& LeftOperand() { return leftOperand_; };
    Operand& RightOperand() { return rightOperand_; };
    bool LeftEmpty() { return leftOperand_.thriad_ == nullptr && leftOperand_.value_.empty(); }
private:
    int index_;
    int operator_;
    Operand leftOperand_;
    Operand rightOperand_;
};

std::string Operand::ToString() const {
    if (thriad_)
        return "^" + std::to_string(thriad_->Index());
    return value_;
}

////////////////////////////////////////////////////////////////////////////////////

class Parser {
public:
    struct Value {
        bool hasValue;
        int value;
    };

    Parser(std::ifstream& ifs) : ifs_(ifs), index_(0), errFound_(false) {}

    bool StartParsing();
    // void parse(std::string);
    void PrintOperations();

    void ProcBase();

    // Name of the variable
    Thriad* ProcI();
    std::string ProcK();

    // Value of the variable
    Thriad* ProcE();
    std::list<Thriad*> ProcT();
    std::string ProcR() { return std::move(ProcK()); }


private:
    std::ifstream& ifs_;

    std::string string_;
    int index_;

    std::list<Thriad> thriads_;
    int thriadCounter_ = 1;

    int GetCounter() { return thriadCounter_++;}

    Thriad* Add(Thriad thriad) {
        thriads_.push_back(thriad);
        return &thriads_.back();
    }
    Thriad* AddByRef(Thriad& thriad) {
        thriads_.push_back(thriad);
        return &thriads_.back();
    }

    Thriad* Find(std::string name) {
        for (auto& thriad : thriads_) {
            if (thriad.LeftOperand().value_ == name)
                return &thriad;
        }
        return nullptr;
    }

    bool errFound_;

    Thriad* Count(std::list<Thriad*>& thriads, bool isSum) {
        auto baseThriad = thriads.front();
        thriads.pop_front();
        for (auto thriad : thriads) {
            Operand lOperand = {baseThriad};
            Operand rOperand = {thriad};
            if (isSum)
                baseThriad = Add(std::move(Thriad(GetCounter(), '+', std::move(lOperand), std::move(rOperand))));
            else
                baseThriad = Add(std::move(Thriad(GetCounter(), '+', std::move(lOperand), std::move(rOperand))));
        }
        return baseThriad;
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
        parser.PrintOperations();

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
            ProcBase();
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

void Parser::PrintOperations() {
    if (!errFound_) {
        if (thriads_.empty()) {
            std::cout << "Operations list is empty." << std::endl;
        } else {
            for (auto& var : thriads_)
                std::cout << var.Index() << ":\t" << var.ToString() << std::endl;
        }
    }
}

void Parser::ProcBase() {
    Operand lOperand = {ProcI()};
    if (lOperand.thriad_->LeftEmpty()) {
        throw std::runtime_error("Missing variable name");
    }
    if (Peek() == '(') {
        Operand rOperand = {ProcE()};
        if (!rOperand.thriad_) {
            throw std::runtime_error("Missing variable value");
        }
        if (Peek() != ')') {
            throw std::runtime_error("Missing closing parenthesis");
        }

        Add(std::move(Thriad(GetCounter(), '=', std::move(lOperand), std::move(rOperand))));
    } else {
        throw std::runtime_error("Missing opening parenthesis");
    }
}

Thriad* Parser::ProcI() {
    std::string value;
    int c = Peek();
    if (c >= 'a' && c <= 'z' || c == '_') {
        value += c;
        value += ProcK();
    } else {
        Back();
    }
    
    return Add(std::move(Thriad(GetCounter(), 'V', {nullptr, value}, {nullptr, "∅"})));
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

Thriad* Parser::ProcE() {
    int c = Peek();
    int index = index_;

    if (c == '-') {
        Operand lOperand = {nullptr, "0"};
        Operand rOperand = {ProcE()};
        if (rOperand.thriad_->Index() == 0)
            throw std::runtime_error("Expected symbol E in " + std::to_string(index_) + " position");
        return Add(std::move(Thriad(GetCounter(), '-', std::move(lOperand), std::move(rOperand))));
    } else if (c == '+') {
        if (Peek() == '(') {
            std::list<Thriad*> thriads = std::move(ProcT());
            if (thriads.empty()) {
                throw std::runtime_error("Expected symbol T in" + std::to_string(index_) + " position");
            }
            if (Peek() != ')') {
                throw std::runtime_error("Expected closing parenthesis, got: " + std::string(1, static_cast<char>(c)));
            }
            return Count(thriads, true);
        } else {
            throw std::runtime_error("Expected opening parenthesis, got: " + std::string(1, static_cast<char>(c)));
        }
    } else if (c == '*') {
        if (Peek() == '(') {
            std::list<Thriad*> thriads = std::move(ProcT());
            if (thriads.empty()) {
                throw std::runtime_error("Expected symbol T in" + std::to_string(index_) + " position");
            }
            if (Peek() != ')') {
                throw std::runtime_error("Expected closing parenthesis, got: " + std::string(1, static_cast<char>(c)));
            }
            return Count(thriads, false);
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
        Operand lOperand = {nullptr, temp};
        Operand rOperand = {nullptr, "∅"};
        return Add(std::move(Thriad(GetCounter(), 'C', std::move(lOperand), std::move(rOperand))));
    } else {
        std::string name;
        Back();
        Operand lOperand = {ProcI()};
        if (!lOperand.thriad_->LeftEmpty()) {
            if (Peek() == '(') {
                Operand rOperand = {ProcE()};
                if (!rOperand.thriad_) {
                    throw std::runtime_error("Expected symbol E in " + std::to_string(index_) + " position");
                }
                c = Peek();
                if (c != ')') {
                    throw std::runtime_error("Expected closing parenthesis, got: " + std::string(1, static_cast<char>(c)));
                }
                return Add(std::move(Thriad(GetCounter(), '=', std::move(lOperand), std::move(rOperand))));
            } else {
                Back();
                auto thriad = Find(name);
                if (thriad) {
                    Operand rOperand = {nullptr, "∅"};
                    // return Add(std::move(Thriad(GetCounter(), 'V', std::move(lOperand), std::move(rOperand))));
                    // return thriad;
                    return lOperand.thriad_;
                } else {
                    throw std::runtime_error("Undefined variable: " + name);
                }
            }
        }
    }
    
    index_ = index;

    return nullptr;
}

std::list<Thriad*> Parser::ProcT() {
    std::list<Thriad*> thriads;
    Value value{false, 0};
    int index = index_;

    Thriad* base = ProcE();
    if (base) {
        if (Peek() == ',') {
            thriads = std::move(ProcT());
            if (thriads.empty())
                throw std::runtime_error("Expected symbol T in " + std::to_string(index_) + " position");
            thriads.push_front(base);
        } else {
            Back();
            thriads.push_back(base);
        }
    } else {
        index_ = index;
    }

    return std::move(thriads);
}
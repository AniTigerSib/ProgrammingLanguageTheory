#include <bitset>
#include <cmath>
#include <fstream>
#include <iostream>
#include <list>
#include <sstream>
#include <string>
#include <vector>

// THRIAD DEFINITIONS //////////////////////////////////////////////////////////////

class Thriad;

struct Int {
    Int(): hasValue_(false) {};
    Int(int value): value_(value), hasValue_(true) {};
    int operator=(int value) {
        hasValue_ = true;
        return value_ = value;
    }
    bool hasValue_;
    int value_;
    std::string ToString() const {
        return std::to_string(value_);
    };
};

class Operand {
public:
    Operand(): thriad_(nullptr), num_(0), value_() {};
    Operand(Thriad* thriad): thriad_(thriad), num_(0), value_() {};
    Operand(Int num): thriad_(nullptr), num_(num), value_() {};
    Operand(std::string value): thriad_(nullptr), num_(0), value_(std::move(value)) {};
    Operand(const char* value): thriad_(nullptr), num_(0), value_(value) {};

    bool operator==(const char* value) { return !value_.empty() && value_ == value; }
    bool operator!=(const char* value) { return !value_.empty() && value_ != value; }

    bool operator==(int value) { return num_.hasValue_ && num_.value_ == value; }
    bool operator!=(int value) { return num_.hasValue_ && num_.value_ != value; }

    void operator=(int num) {
        num_ = num;
        thriad_ = nullptr;
        value_.clear();
    }

    void operator=(std::string val) {
        num_.value_ = 0;
        num_.hasValue_ = false;
        thriad_ = nullptr;
        value_ = val;
    }

    bool hasIndex(int index) const;

    bool isConstRef();
    bool isConst() { return num_.hasValue_; }
    int getConst();
    bool isVar();
    std::string getVar();

    std::string ToString() const;
    bool Empty() const { return !thriad_ && value_.empty() && !num_.hasValue_; }

    Thriad* thriad_;
    Int num_;
    std::string value_;
};

class Thriad {
public:
    Thriad(int index, int op, Operand leftOperand, Operand rightOperand):
        index_(index), operator_(op), leftOperand_(leftOperand), rightOperand_(rightOperand) {};

    std::string ToString() const {
        return static_cast<char>(operator_) + std::string("(") + leftOperand_.ToString() + ", " + rightOperand_.ToString() + ")";
    }

    bool operator==(const Thriad& thriad) const { return thriad.index_ == index_; }
    Thriad operator=(Thriad triad) {
        index_ = triad.index_;
        operator_ = triad.operator_;
        leftOperand_ = triad.leftOperand_;
        rightOperand_ = triad.rightOperand_;
        return *this;
    }

    int Index() { return index_; };
    char Operator() { return operator_; };
    Operand& LeftOperand() { return leftOperand_; };
    Operand& RightOperand() { return rightOperand_; };
    bool LeftEmpty() { return leftOperand_.Empty(); };
private:
    int index_;
    int operator_;
    Operand leftOperand_;
    Operand rightOperand_;
};

std::string Operand::ToString() const {
    if (thriad_)
        return "^" + std::to_string(thriad_->Index());
    if (!value_.empty())
        return value_;
    // std::cout << std::bitset<10>(num_.value_);
    std::basic_stringstream<char> sbuf;
    sbuf << std::bitset<10>(num_.value_);
    return sbuf.str();
}

bool Operand::hasIndex(int index) const {
    return thriad_ && thriad_->Index() == index;
}

bool Operand::isConstRef() {
    return thriad_ && thriad_->Operator() == 'C';
}

int Operand::getConst() {
    if (thriad_)
        return thriad_->LeftOperand().num_.value_;
    else
        return num_.value_;
}

bool Operand::isVar() {
    return thriad_ && thriad_->Operator() == 'V';
}

std::string Operand::getVar() {
    if (thriad_)
        return thriad_->LeftOperand().value_;
    else
        return "";
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
    void Optimize();
    Thriad* RuleOne(Thriad *thriad, bool left);
    void RuleTwo(Thriad *thriad);
    Thriad* RuleThree(Thriad *thriad);
    Thriad* RuleFour(Thriad *thriad);

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
    if (!parser.StartParsing()) {
        parser.Optimize();
        parser.PrintOperations();
    }

    ifs.close();

    return 0;
}

// IMPLEMENTATION //////////////////////////////////////////////////////////////////

void Parser::Optimize() {
    std::vector<Thriad*> toDelete;
    Thriad* res = nullptr;
    auto iter = thriads_.begin();
    for (int i = 0; i < thriads_.size(); i++) {
        if (iter->Operator() == '-') {
            res = RuleOne(&(*iter), true);
            if (res) {
                toDelete.push_back(res);
            }
            RuleTwo(&(*iter));
        }
        if (iter->Operator() == '+' || iter->Operator() == '*') {
            res = RuleOne(&(*iter), false);
            if (res) {
                toDelete.push_back(res);
            }
            res = RuleThree(&(*iter));
            if (res)
                toDelete.push_back(res);
        }
        if (iter->Operator() == '=') {
            res = RuleFour(&(*iter));
            if (res)
                toDelete.push_back(res);
        }

        iter++;
    }
    for (auto thriad : toDelete) {
        thriads_.remove(*thriad);
    }
}

Thriad* Parser::RuleOne(Thriad *thriad, bool left) {
    Thriad *res = nullptr;
    if (left) {
        if (thriad->LeftOperand().isConstRef()) {
            res = thriad->LeftOperand().thriad_;
            thriad->LeftOperand() = thriad->LeftOperand().getConst();
        }
    } else {
        if (thriad->RightOperand().isConstRef()) {
            res = thriad->RightOperand().thriad_;
            thriad->RightOperand() = thriad->RightOperand().getConst();
        }
    }
    return res;
}

void Parser::RuleTwo(Thriad *thriad) {
    if (thriad->LeftOperand().isConst())
        *thriad = Thriad(thriad->Index(), 'C', Operand(-thriad->LeftOperand().getConst()), Operand("∅"));
}

Thriad* Parser::RuleThree(Thriad *thriad) {
    Thriad *res = nullptr;
    if (thriad->LeftOperand().isConstRef() && thriad->RightOperand().isConst()) {
        res = thriad->LeftOperand().thriad_;
        int val = 0;
        if (thriad->Operator() == '+') {
            val = thriad->LeftOperand().getConst() + thriad->RightOperand().getConst();
        } else {
            val = thriad->LeftOperand().getConst() * thriad->RightOperand().getConst();
        }
        *thriad = Thriad(thriad->Index(), 'C', Operand(val), Operand("∅"));
    }
    return res;
}

Thriad* Parser::RuleFour(Thriad *thriad) {
    Thriad *res = nullptr;
    if (thriad->LeftOperand().isVar()) {
        res = thriad->LeftOperand().thriad_;
        thriad->LeftOperand() = thriad->LeftOperand().getVar();
    }
    return res;
}

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
    
    return Add(std::move(Thriad(GetCounter(), 'V', Operand(value), Operand("∅"))));
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
        Operand rOperand = "∅";
        Operand lOperand = ProcE();
        if (lOperand.thriad_->Index() == 0)
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
        Operand lOperand = Operand(stringToBinary(temp));
        Operand rOperand = "∅";
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
                    Operand rOperand = "∅";
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
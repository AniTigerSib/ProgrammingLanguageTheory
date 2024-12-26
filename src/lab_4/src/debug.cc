#include <cmath>
#include <iostream>

int stringToBinary(std::string& value) {
    int result = 0;
    for (int i = 0; i < value.size(); i++) {
        result += (value[i] - '0') * pow(2, value.size() - i - 1);
    //   std::cout << value[i] << std::endl;
    }
    return result;
}

int main() {
    auto val = std::string("0111");
    std::cout << stringToBinary(val) << std::endl;
    return 0;
}
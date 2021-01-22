#include "utils.hpp"

// way faster than regex
bool isDouble(std::string myString) {
    std::istringstream iss(myString);
    double f;
    iss >> std::noskipws >> f; // noskipws considers leading whitespace invalid
    // Check the entire string was consumed and if either failbit or badbit is set
    return iss.eof() && !iss.fail(); 
}

bool splitNumbers(std::vector<std::string> *values, std::string str, char delimiter) {
    (*values).clear();

    std::stringstream ss(str);
    std::string value;

    while (std::getline(ss, value, delimiter)) {
        if (!value.empty() && value[value.length()-1] == '\r') {
            value.erase(value.length()-1);
        }
        //TODO: number check will remove rows with NaN probably, need to fetch last thing instead of removing entire line.
        if (!isDouble(value)) return false;
                
        (*values).push_back(value);
    }
    
    return true;
}
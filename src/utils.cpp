#include "utils.hpp"
#include <fstream>

bool isDouble(std::string str) {
    std::istringstream iss(str);
    double f;
    iss >> std::noskipws >> f; // noskipws considers leading whitespace invalid
    // Check the entire string was consumed and if either failbit or badbit is set
    return iss.eof() && !iss.fail();
}

bool split(std::vector<std::string> *values, std::string str, char delimiter, bool isNumber) {
    (*values).clear();

    std::stringstream ss(str);
    std::string value;

    while (std::getline(ss, value, delimiter)) {
        if (!value.empty() && value[value.length()-1] == '\r') {
            value.erase(value.length()-1);
        }
        //TODO: number check will remove rows with NaN probably, need to fetch last thing instead of removing entire line.
        if (isNumber && !isDouble(value)) return false;
                
        (*values).push_back(value);
    }
    
    return true;
}

void appendLineToFile(std::string filePath, std::string line)
{
    std::ofstream file;
    //can't enable exception now because of gcc bug that raises ios_base::failure with useless message
    //file.exceptions(file.exceptions() | std::ios::failbit);
    file.open(filePath, std::ios::out | std::ios::app);
    if (file.fail())
        throw std::ios_base::failure(std::strerror(errno));

    // Make sure write fails with exception if something is wrong
    file.exceptions(file.exceptions() | std::ios::failbit | std::ifstream::badbit);

    file << line << std::endl;

    file.close();
}
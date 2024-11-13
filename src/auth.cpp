#include "auth.h"
#include <ctime>
#include <sstream>
#include <random>
#include <iostream>

std::string generateToken(const std::string& username) {
    
    std::time_t now = std::time(0);
    std::ostringstream tokenStream;
    tokenStream << username << "-" << now;

    std::random_device rd;
    std::uniform_int_distribution<int> dist(1000, 9999);
    tokenStream << "-" << dist(rd);

    return tokenStream.str();
}

bool validateToken(const std::string& token) {

    if (token.empty()) {
        return false;
    }

    return token.find("user-") == 0;
}


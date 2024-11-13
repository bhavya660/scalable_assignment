#ifndef AUTH_H
#define AUTH_H

#include <string>

std::string generateToken(const std::string& username);

bool validateToken(const std::string& token);

#endif 

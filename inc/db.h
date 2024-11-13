#ifndef DB_H
#define DB_H

#include <string>

bool initDatabase();
bool addUser(const std::string& username, const std::string& password, int roleId);
bool getUser(const std::string& username, std::string& password, int& roleId);
bool roleExists(int roleId);
void closeDatabase();

#endif


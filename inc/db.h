#ifndef DB_H
#define DB_H

#include <string>
#include <vector>

bool initDatabase();
bool addUser(const std::string& username, const std::string& password, int roleId);
bool getUser(const std::string& username, std::string& password, int& roleId);
bool roleExists(int roleId);
bool getAllUsers(std::vector<std::tuple<std::string, std::string, int>>& users);
bool getUserByUsername(const std::string& username, std::string& storedPassword, int& roleId);
void closeDatabase();

#endif


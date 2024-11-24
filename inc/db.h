#ifndef DB_H
#define DB_H

#include <string>
#include <vector>

bool initDatabase();
bool addUser(const std::string& username, const std::string& password, int roleId);
bool addUser(const std::string& username, const std::string& password, int roleId, const std::string& restaurantName, const std::string& contactNumber, const std::string& address);
bool getUser(const std::string& username, std::string& password, int& roleId);
bool roleExists(int roleId);
bool getAllUsers(std::vector<std::tuple<std::string, std::string, int>>& users);
bool getUserByUsername(const std::string& username, std::string& storedPassword, int& roleId);
bool getUserById(int userId, std::string& username, std::string& storedPassword, int& roleId, std::string& restaurantName, std::string& contactNumber, std::string& address);
void closeDatabase();

#endif


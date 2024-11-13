// db.cpp
#include "db.h"
#include <sqlite3.h>
#include <iostream>

sqlite3* db = nullptr;

bool initDatabase() {
    int rc = sqlite3_open("/home/girish/temp/microservice/db/user_service.db", &db);
    if (rc != SQLITE_OK) {
        std::cerr << "Error opening SQLite database: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    // SQL statement to create tables
    const char* createRolesTableSQL = R"(
        CREATE TABLE IF NOT EXISTS roles (
            role_id INTEGER PRIMARY KEY,
            role_name TEXT UNIQUE NOT NULL
        );
    )";

    const char* createUsersTableSQL = R"(
        CREATE TABLE IF NOT EXISTS users (
            user_id INTEGER PRIMARY KEY,
            username TEXT UNIQUE NOT NULL,
            password TEXT NOT NULL,
            role_id INTEGER NOT NULL,
            FOREIGN KEY (role_id) REFERENCES roles(role_id)
        );
    )";

    char* errMsg = nullptr;
    if (sqlite3_exec(db, createRolesTableSQL, 0, 0, &errMsg) != SQLITE_OK) {
        std::cerr << "Error creating roles table: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }

    if (sqlite3_exec(db, createUsersTableSQL, 0, 0, &errMsg) != SQLITE_OK) {
        std::cerr << "Error creating users table: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }

    // Insert roles in roles table
    const char* insertRolesSQL = R"(
        INSERT INTO roles (role_name) VALUES ('admin')
        ON CONFLICT(role_name) DO NOTHING;
        INSERT INTO roles (role_name) VALUES ('user')
        ON CONFLICT(role_name) DO NOTHING;
        INSERT INTO roles (role_name) VALUES ('guest')
        ON CONFLICT(role_name) DO NOTHING;
        INSERT INTO roles (role_name) VALUES ('delivery_partner')
        ON CONFLICT(role_name) DO NOTHING;
    )";

    if (sqlite3_exec(db, insertRolesSQL, 0, 0, &errMsg) != SQLITE_OK) {
        std::cerr << "Error inserting predefined roles: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }

    return true;
}

bool roleExists(int roleId) {
    std::string query = "SELECT COUNT(*) FROM roles WHERE role_id = ?";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Error preparing statement: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    sqlite3_bind_int(stmt, 1, roleId);

    bool exists = false;
    if (sqlite3_step(stmt) == SQLITE_ROW && sqlite3_column_int(stmt, 0) > 0) {
        exists = true;
    }
    sqlite3_finalize(stmt);
    return exists;
}

bool addUser(const std::string& username, const std::string& password, int roleId) {
    const char* sql = "INSERT INTO users (username, password, role_id) VALUES (?, ?, ?);";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, roleId);

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    if (!success) {
        std::cerr << "Failed to execute statement: " << sqlite3_errmsg(db) << std::endl;
    }

    sqlite3_finalize(stmt);
    return success;
}

bool getUser(const std::string& username, std::string& password, int& roleId) {
    const char* sql = "SELECT password, role_id FROM users WHERE username = ?;";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);

    bool userFound = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        password = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        roleId = sqlite3_column_int(stmt, 1);
        userFound = true;
    }

    sqlite3_finalize(stmt);
    return userFound;
}

void closeDatabase() {
    if (db) {
        sqlite3_close(db);
        db = nullptr;
    }
}


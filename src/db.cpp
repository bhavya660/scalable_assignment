// db.cpp
#include "db.h"
#include <sqlite3.h>
#include <iostream>
#include <tuple>
#include "Logger.h"

sqlite3 *db = nullptr;

bool initDatabase()
{
    int rc = sqlite3_open("/home/girish/temp/microservice/db/user_service.db", &db);
    if (rc != SQLITE_OK)
    {
        std::cerr << "Error opening SQLite database: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    // SQL statements to create tables
    const char *createRolesTableSQL = R"(
        CREATE TABLE IF NOT EXISTS roles (
            role_id INTEGER PRIMARY KEY,
            role_name TEXT UNIQUE NOT NULL
        );
    )";

    const char *createUsersTableSQL = R"(
        CREATE TABLE IF NOT EXISTS users (
            user_id INTEGER PRIMARY KEY,
            username TEXT UNIQUE NOT NULL,
            password TEXT NOT NULL,
            role_id INTEGER NOT NULL,
            FOREIGN KEY (role_id) REFERENCES roles(role_id)
        );
    )";

    const char *createRestOwnerTableSQL = R"(
        CREATE TABLE IF NOT EXISTS restaurant_owners (
            owner_id INTEGER PRIMARY KEY,
            user_id INTEGER NOT NULL UNIQUE,
            restaurant_name TEXT NOT NULL,
            contact_number TEXT NOT NULL,
            address TEXT NOT NULL,
            FOREIGN KEY (user_id) REFERENCES users(user_id)
        );
    )";

    char *errMsg = nullptr;
    if (sqlite3_exec(db, createRolesTableSQL, 0, 0, &errMsg) != SQLITE_OK)
    {
        std::cerr << "Error creating roles table: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }

    if (sqlite3_exec(db, createUsersTableSQL, 0, 0, &errMsg) != SQLITE_OK)
    {
        std::cerr << "Error creating users table: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }

    if (sqlite3_exec(db, createRestOwnerTableSQL, 0, 0, &errMsg) != SQLITE_OK)
    {
        std::cerr << "Error creating restaurant owner table: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }

    // Insert roles into roles table
    const char *insertRolesSQL = R"(
        INSERT INTO roles (role_name) VALUES ('admin')
        ON CONFLICT(role_name) DO NOTHING;
        INSERT INTO roles (role_name) VALUES ('user')
        ON CONFLICT(role_name) DO NOTHING;
        INSERT INTO roles (role_name) VALUES ('guest')
        ON CONFLICT(role_name) DO NOTHING;
        INSERT INTO roles (role_name) VALUES ('delivery_partner')
        ON CONFLICT(role_name) DO NOTHING;
        INSERT INTO roles (role_name) VALUES ('restaurant_owner')
        ON CONFLICT(role_name) DO NOTHING;
    )";

    if (sqlite3_exec(db, insertRolesSQL, 0, 0, &errMsg) != SQLITE_OK)
    {
        std::cerr << "Error inserting predefined roles: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }

    // Insert default users and restaurant owners
    const char *insertDefaultUsersSQL = R"(
        INSERT INTO users (username, password, role_id) VALUES
            ('admin', 'Admin@123', (SELECT role_id FROM roles WHERE role_name = 'admin')),
            ('normal_user', 'user_pass', (SELECT role_id FROM roles WHERE role_name = 'user')),
            ('restaurant_owner1', 'owner1_pass', (SELECT role_id FROM roles WHERE role_name = 'restaurant_owner'));
    )";

    const char *insertDefaultOwnersSQL = R"(
        INSERT INTO restaurant_owners (user_id, restaurant_name, contact_number, address) VALUES
            ((SELECT user_id FROM users WHERE username = 'restaurant_owner1'),
             'Gourmet Bistro', '1234567890', '123 Main Street');
    )";

    if (sqlite3_exec(db, insertDefaultUsersSQL, 0, 0, &errMsg) != SQLITE_OK)
    {
        std::cerr << "Error inserting default users: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }

    if (sqlite3_exec(db, insertDefaultOwnersSQL, 0, 0, &errMsg) != SQLITE_OK)
    {
        std::cerr << "Error inserting default restaurant owners: " << errMsg << std::endl;
        sqlite3_free(errMsg);
        return false;
    }

    return true;
}

bool roleExists(int roleId)
{
    std::string query = "SELECT COUNT(*) FROM roles WHERE role_id = ?";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    {
        std::cerr << "Error preparing statement: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }
    sqlite3_bind_int(stmt, 1, roleId);

    bool exists = false;
    if (sqlite3_step(stmt) == SQLITE_ROW && sqlite3_column_int(stmt, 0) > 0)
    {
        exists = true;
    }
    sqlite3_finalize(stmt);
    return exists;
}

bool addUser(const std::string &username, const std::string &password, int roleId)
{
    const char *sql = "INSERT INTO users (username, password, role_id) VALUES (?, ?, ?);";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK)
    {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, roleId);

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    if (!success)
    {
        std::cerr << "Failed to execute statement: " << sqlite3_errmsg(db) << std::endl;
    }

    sqlite3_finalize(stmt);
    return success;
}

bool addUser(const std::string &username, const std::string &password, int roleId, const std::string &restaurantName, const std::string &contactNumber, const std::string &address)
{
    Logger logger("/home/girish/logs/app.log", 1024 * 1024); // Log file with 1MB max size
    logger.log(Logger::LogLevel::DEBUG, "Overloaded function of addUser()");

    if (sqlite3_exec(db, "BEGIN TRANSACTION;", 0, 0, 0) != SQLITE_OK)
    {
        logger.log(Logger::LogLevel::ERROR, "Failed to begin transaction: " + std::string(sqlite3_errmsg(db)));
        return false;
    }

    const char *addUserSQL = "INSERT INTO users (username, password, role_id) VALUES (?, ?, ?);";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, addUserSQL, -1, &stmt, 0) != SQLITE_OK)
    {
        logger.log(Logger::LogLevel::ERROR, "Failed to prepare addUser statement: " + std::string(sqlite3_errmsg(db)));
        sqlite3_exec(db, "ROLLBACK;", 0, 0, 0); // Rollback transaction on error
        return false;
    }

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, roleId);

    if (sqlite3_step(stmt) != SQLITE_DONE)
    {
        logger.log(Logger::LogLevel::ERROR, "Failed to execute addUser statement: " + std::string(sqlite3_errmsg(db)));
        sqlite3_finalize(stmt);
        sqlite3_exec(db, "ROLLBACK;", 0, 0, 0); 
        return false;
    }

    int userId = sqlite3_last_insert_rowid(db);
    sqlite3_finalize(stmt);

    if (roleId == 5)
    { 
        const char *addRestaurantOwnerSQL = R"(
            INSERT INTO restaurant_owners (user_id, restaurant_name, contact_number, address)
            VALUES (?, ?, ?, ?);
        )";

        if (sqlite3_prepare_v2(db, addRestaurantOwnerSQL, -1, &stmt, 0) != SQLITE_OK)
        {
            logger.log(Logger::LogLevel::ERROR, "Failed to prepare addRestaurantOwner statement: " + std::string(sqlite3_errmsg(db)));
            sqlite3_exec(db, "ROLLBACK;", 0, 0, 0);
            return false;
        }

        sqlite3_bind_int(stmt, 1, userId);
        sqlite3_bind_text(stmt, 2, restaurantName.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, contactNumber.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 4, address.c_str(), -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) != SQLITE_DONE)
        {
            logger.log(Logger::LogLevel::ERROR, "Failed to execute addRestaurantOwner statement: " + std::string(sqlite3_errmsg(db)));
            sqlite3_finalize(stmt);
            sqlite3_exec(db, "ROLLBACK;", 0, 0, 0); // Rollback transaction on error
            return false;
        }

        sqlite3_finalize(stmt);
    }

    if (sqlite3_exec(db, "COMMIT;", 0, 0, 0) != SQLITE_OK)
    {
        logger.log(Logger::LogLevel::ERROR, "Failed to commit transaction: " + std::string(sqlite3_errmsg(db)));
        sqlite3_exec(db, "ROLLBACK;", 0, 0, 0); // Rollback transaction on error
        return false;
    }

    logger.log(Logger::LogLevel::INFO, "User added successfully.");
    return true;
}

bool getUser(const std::string &username, std::string &password, int &roleId)
{
    const char *sql = "SELECT password, role_id FROM users WHERE username = ?;";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK)
    {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);

    bool userFound = false;
    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        password = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
        roleId = sqlite3_column_int(stmt, 1);
        userFound = true;
    }

    sqlite3_finalize(stmt);
    return userFound;
}

// Function to retrieve all users from the database
bool getAllUsers(std::vector<std::tuple<std::string, std::string, int>> &users)
{
    const char *sql = "SELECT username, password, role_id FROM users;";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK)
    {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        std::string username = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
        std::string password = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
        int roleId = sqlite3_column_int(stmt, 2);
        users.push_back(std::make_tuple(username, password, roleId));
    }

    sqlite3_finalize(stmt);
    return true;
}

// Function to retrieve a user by username from the database
bool getUserByUsername(const std::string &username, std::string &storedPassword, int &roleId)
{
    const char *sql = "SELECT password, role_id FROM users WHERE username = ?;";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK)
    {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);

    bool userFound = false;
    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        storedPassword = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
        roleId = sqlite3_column_int(stmt, 1);
        userFound = true;
    }

    sqlite3_finalize(stmt);
    return userFound;
}

// Function to retrieve user details or restaurant owner details based on user id
bool getUserById(int userId, std::string &username, std::string &storedPassword, int &roleId, std::string &restaurantName, std::string &contactNumber, std::string &address)
{
    const char *sql = R"(
        SELECT u.username, u.password, u.role_id, ro.restaurant_name, ro.contact_number, ro.address
        FROM users u
        LEFT JOIN restaurant_owners ro ON u.user_id = ro.user_id
        WHERE u.user_id = ?;
    )";

    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK)
    {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    sqlite3_bind_int(stmt, 1, userId); // Bind the userId parameter

    bool userFound = false;
    if (sqlite3_step(stmt) == SQLITE_ROW)
    {
        username = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 0));
        storedPassword = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 1));
        roleId = sqlite3_column_int(stmt, 2);

        // Check if restaurant owner data is present (it will be NULL if the user is not a restaurant owner)
        if (sqlite3_column_text(stmt, 3) != nullptr)
        {
            restaurantName = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 3));
            contactNumber = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 4));
            address = reinterpret_cast<const char *>(sqlite3_column_text(stmt, 5));
        }
        else
        {
            restaurantName.clear();
            contactNumber.clear();
            address.clear();
        }

        userFound = true;
    }

    sqlite3_finalize(stmt);
    return userFound;
}

void closeDatabase()
{
    if (db)
    {
        sqlite3_close(db);
        db = nullptr;
    }
}

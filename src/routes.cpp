// routes.cpp
#include "routes.h"
#include "db.h"
#include "auth.h"
#include <json/json.h>
#include <iostream>
#include "Logger.h"
#include <openssl/sha.h>
#include <iomanip>
#include <regex>

using namespace std;

// Function to handle fetching all users or a specific user by username
void handleGetUsers(FCGX_Request& request, const std::string& username) {
    Logger logger("/home/girish/logs/app.log", 1024 * 1024); // Log file with 1MB max size
    logger.log(Logger::LogLevel::DEBUG, "handleGetUsers()");

    const char *requestUri = FCGX_GetParam("REQUEST_URI", request.envp);
    logger.log(Logger::LogLevel::DEBUG, "%s, username  : %s", requestUri, username);

    if(strcmp(requestUri, "/users") == 0)
    {
        std::vector<std::tuple<std::string, std::string, int>> users;
        if (!getAllUsers(users)) {
            sendResponse(request, 500, "Error retrieving users");
            return;
        }

        Json::Value jsonResponse;
        Json::Value usersArray(Json::arrayValue);

        for (const auto& user : users) {
            Json::Value userJson;
            userJson["username"] = std::get<0>(user);
            userJson["role_id"] = std::get<2>(user);
            usersArray.append(userJson);
        }

        jsonResponse["status"] = 200;
        jsonResponse["users"] = usersArray;

        Json::StreamWriterBuilder writer;
        std::string responseBody = Json::writeString(writer, jsonResponse);
        sendResponse(request, 200, responseBody);
    }
    else
    {
        std::string Uri(requestUri);
        std::string path;
        int id;
        std::string username;
        std::string storedPassword;
        int roleId;
        std::string restaurantName;
        std::string contactNumber;
        std::string address;
        
        size_t pos = Uri.find_last_of('/');
        if (pos != std::string::npos) {
            path = Uri.substr(0, pos);
            std::string idStr = Uri.substr(pos + 1);
            id = std::stoi(idStr);
        }

        if (!getUserById(id, username, storedPassword, roleId, restaurantName, contactNumber, address)) {
            sendResponse(request, 404, "User not found");
            return;
        }

        logger.log(Logger::LogLevel::DEBUG, "\n id: %d, username : %s, storedPassword : %s, roleId : %d, restaurantName : %s, contactNumber : %s, address : %s \n", id, username.c_str(), storedPassword.c_str(), roleId, restaurantName.c_str(), contactNumber.c_str(), address.c_str());

        Json::Value jsonResponse;

        if(restaurantName.empty())
        {
            jsonResponse["username"] = username;
            jsonResponse["roleId"] = roleId;     
        }
        else
        {
            jsonResponse["username"] = username;
            jsonResponse["roleId"] = roleId;
            jsonResponse["restaurantName"] = restaurantName;
            jsonResponse["contactNumber"] = contactNumber;
            jsonResponse["address"] = address;
        }

        Json::StreamWriterBuilder writer;
        std::string responseBody = Json::writeString(writer, jsonResponse);
        sendResponse(request, 200, responseBody);
    }
}

// Function to handle user registration
void handleUserRegistration(FCGX_Request& request) {
    Logger logger("/home/girish/logs/app.log", 1024 * 1024); // Log file with 1MB max size
    logger.log(Logger::LogLevel::DEBUG, "handleUserRegistration()");

    // Read and parse JSON data from the request
    char* contentLengthStr = FCGX_GetParam("CONTENT_LENGTH", request.envp);
    int contentLength = contentLengthStr ? atoi(contentLengthStr) : 0;
    
    std::string requestBody;
    requestBody.resize(contentLength);
    FCGX_GetStr(&requestBody[0], contentLength, request.in);

    // Parse JSON request body
    Json::Value jsonData;
    Json::Reader jsonReader;
    if (!jsonReader.parse(requestBody, jsonData)) {
        sendResponse(request, 400, "Invalid JSON format");
        return;
    }

    std::string username = jsonData.get("username", "").asString();
    std::string password = jsonData.get("password", "").asString();
    int roleId = jsonData.get("role_id", 0).asInt();

    // Restaurant table details
    std::string restaurantName = jsonData.get("restaurantName", "").asString();
    std::string contactNumber = jsonData.get("contactNumber", "").asString();
    std::string address = jsonData.get("address", "").asString();

    if (username.empty() || password.empty() || roleId == 0) {
        sendResponse(request, 400, "Missing required fields");
        return;
    }

    std::string storedPassword;
    int storedRoleId;
    if (getUser(username, storedPassword, storedRoleId)) {
        sendResponse(request, 409, "User already exists");
        return;
    }

    // Check if the role ID is valid
    if (!roleExists(roleId)) {
        sendResponse(request, 400, "Invalid role ID");
        return;
    }

    // Hashing the password
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, password.c_str(), password.size());
    SHA256_Final(hash, &sha256);

    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    std::string password_hash = ss.str();

    logger.log(Logger::LogLevel::INFO, "username : %s, password : %s, role_id : %d", username.c_str(), password_hash.c_str(), roleId);

    if(roleId == RESTAURANT_OWNER)
    {
        if(restaurantName.empty() || contactNumber.empty() || address.empty())
        {
            sendResponse(request, 400, "Missing required fields");
            return;

        }
        
        logger.log(Logger::LogLevel::INFO, "overloaded");
        // Call the addUser function from db.cpp
        if (addUser(username, password_hash, roleId)) {
            logger.log(Logger::LogLevel::INFO, "User registered successfully");
            sendResponse(request, 201, "User registered successfully");
        } else {
            logger.log(Logger::LogLevel::INFO, "Error registering user");
            sendResponse(request, 500, "Error registering user");
        }
    }
    else
    {
        // Call the addUser function from db.cpp
        if (addUser(username, password_hash, roleId, restaurantName, contactNumber, address)) {
            logger.log(Logger::LogLevel::INFO, "User registered successfully");
            sendResponse(request, 201, "User registered successfully");
        } else {
            logger.log(Logger::LogLevel::INFO, "Error registering user");
            sendResponse(request, 500, "Error registering user");
        }
    }
    
}


// Function to handle user login and authentication
void handleUserLogin(FCGX_Request& request) {
    
    char* contentLengthStr = FCGX_GetParam("CONTENT_LENGTH", request.envp);
    int contentLength = contentLengthStr ? atoi(contentLengthStr) : 0;

    std::string requestBody;
    requestBody.resize(contentLength);
    FCGX_GetStr(&requestBody[0], contentLength, request.in);

    // Parse JSON request body
    Json::Value jsonData;
    Json::Reader jsonReader;
    if (!jsonReader.parse(requestBody, jsonData)) {
        sendResponse(request, 400, "Invalid JSON format");
        return;
    }

    std::string username = jsonData.get("username", "").asString();
    std::string password = jsonData.get("password", "").asString();

    if (username.empty() || password.empty()) {
        sendResponse(request, 400, "Missing required fields");
        return;
    }

    // Hashing the password
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, password.c_str(), password.size());
    SHA256_Final(hash, &sha256);

    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    std::string password_hash = ss.str();

    std::string storedPassword;
    int roleId;
    if (!getUser(username, storedPassword, roleId)) {
        sendResponse(request, 401, "Invalid username or password");
        return;
    }

    // Check password and generate token if successful
    if (storedPassword == password_hash) {
        std::string token = generateToken(username);
        Json::Value responseData;
        responseData["message"] = "Login successful";
        responseData["token"] = token;

        // Convert JSON response to string
        Json::StreamWriterBuilder writer;
        std::string responseBody = Json::writeString(writer, responseData);
        sendResponse(request, 200, responseBody);
    } else {
        sendResponse(request, 401, "Invalid username or password");
    }
}

// Function to send HTTP responses
void sendResponse(FCGX_Request& request, int statusCode, const std::string& message) {
    // Set HTTP status code and headers
    FCGX_FPrintF(request.out, "Status: %d\r\n", statusCode);
    FCGX_FPrintF(request.out, "Content-Type: application/json\r\n\r\n");

    // Send JSON response body
    Json::Value jsonResponse;
    jsonResponse["status"] = statusCode;
    jsonResponse["message"] = message;
    
    Json::StreamWriterBuilder writer;
    std::string responseBody = Json::writeString(writer, jsonResponse);

    FCGX_FPrintF(request.out, "%s", responseBody.c_str());
}


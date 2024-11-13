// routes.cpp
#include "routes.h"
#include "db.h"
#include "auth.h"
#include <json/json.h>
#include <iostream>
#include "Logger.h"
#include <openssl/sha.h>
#include <iomanip>

using namespace std;

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

    // Call the addUser function from db.cpp
    if (addUser(username, password_hash, roleId)) {
        logger.log(Logger::LogLevel::INFO, "User registered successfully");
        sendResponse(request, 201, "User registered successfully");
    } else {
        logger.log(Logger::LogLevel::INFO, "Error registering user");
        sendResponse(request, 500, "Error registering user");
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


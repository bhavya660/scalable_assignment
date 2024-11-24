#include "auth.h"
#include <ctime>
#include <sstream>
#include <random>
#include <iostream>
#include "Logger.h"
#include <openssl/md5.h>


std::string md5(const std::string &input)
{
    unsigned char result[MD5_DIGEST_LENGTH];
    MD5((unsigned char *)input.c_str(), input.size(), result);

    std::ostringstream hex;
    for (int i = 0; i < 16; ++i)
        hex << std::setw(2) << std::setfill('0') << std::hex << (int)result[i];

    return hex.str();
}

std::map<std::string, std::string> parse_authorization_header(const std::string &authHeader)
{
    Logger logger("/home/girish/logs/app.log", 1024 * 1024); // Log file with 1MB max size
    logger.log(Logger::LogLevel::INFO, "User service started");

    std::map<std::string, std::string> params;
    std::string key, value;
    std::istringstream iss(authHeader.substr(authHeader.find(' ') + 1)); 
    std::string token;

    while (std::getline(iss, token, ','))
    {
        std::istringstream tokenStream(token);
        std::getline(tokenStream, key, '=');
        std::getline(tokenStream, value);

        key.erase(std::remove_if(key.begin(), key.end(), ::isspace), key.end());
        value.erase(std::remove_if(value.begin(), value.end(), ::isspace), value.end());
        value.erase(std::remove(value.begin(), value.end(), '\"'), value.end());

        params[key] = value;
    }
    return params;
}

bool validate_digest(const char *method, const char *uri, const char *authHeader)
{
    Logger logger("/home/girish/logs/app.log", 1024 * 1024); // Log file with 1MB max size

    auto params = parse_authorization_header(authHeader);
    for (const auto &param : params)
    {
        logger.log(Logger::LogLevel::INFO, "%s", (param.first + ": " + param.second).c_str());
    }

    std::string username = params["username"];
    std::string realm = params["realm"];
    std::string nonce = params["nonce"];
    std::string uri_param = params["uri"];
    std::string response = params["response"];
    std::string qop = params["qop"];
    std::string nc = params["nc"];
    std::string cnonce = params["cnonce"];

    logger.log(Logger::LogLevel::INFO, "username: %s", username.c_str());
    logger.log(Logger::LogLevel::INFO, "realm: %s", realm.c_str());
    logger.log(Logger::LogLevel::INFO, "nonce: %s", nonce.c_str());
    logger.log(Logger::LogLevel::INFO, "uri: %s", uri_param.c_str());
    logger.log(Logger::LogLevel::INFO, "qop: %s", qop.c_str());
    logger.log(Logger::LogLevel::INFO, "nc: %s", nc.c_str());
    logger.log(Logger::LogLevel::INFO, "cnonce: %s", cnonce.c_str());

    std::string methodStr(method);
    std::string password = "Admin@123"; // TODO: To get it from database
    std::string ha1 = md5(username + ":" + realm + ":" + password);
    std::string ha2 = md5(methodStr + ":" + uri_param);

    std::string valid_response;
    if (qop == "auth" || qop == "auth-int")
    {
        valid_response = md5(ha1 + ":" + nonce + ":" + nc + ":" + cnonce + ":" + qop + ":" + ha2);
    }
    else
    {
        valid_response = md5(ha1 + ":" + nonce + ":" + ha2);
    }

    logger.log(Logger::LogLevel::INFO, "HA1: " + ha1);
    logger.log(Logger::LogLevel::INFO, "HA2: " + ha2);
    logger.log(Logger::LogLevel::INFO, "Valid Response: " + valid_response);
    logger.log(Logger::LogLevel::INFO, "Client Response: " + response);

    return response == valid_response;
}

std::string generate_nonce()
{
    std::ostringstream oss;
    oss << std::time(nullptr);
    std::string timestamp = oss.str();

    unsigned char result[MD5_DIGEST_LENGTH];
    MD5((unsigned char *)timestamp.c_str(), timestamp.size(), result);

    std::ostringstream hex;
    for (int i = 0; i < 16; ++i)
        hex << std::setw(2) << std::setfill('0') << std::hex << (int)result[i];

    return hex.str();
}

void send_auth_challenge(FCGX_Request &request)
{
    Logger logger("/home/girish/logs/app.log", 1024 * 1024); // Log file with 1MB max size

    std::string nonce = generate_nonce();
    FCGX_FPrintF(request.out, "Status: 401 Unauthorized\r\n");
    FCGX_FPrintF(request.out, "WWW-Authenticate: Digest realm=\"microservice\", nonce=\"%s\", qop=\"auth\"\r\n", nonce.c_str());
    FCGX_FPrintF(request.out, "Content-Type: text/html\r\n\r\n");
    FCGX_FPrintF(request.out, "<html><body>Unauthorized</body></html>\n");

    logger.log(Logger::LogLevel::INFO,"Sent authentication challenge with nonce: %s", nonce.c_str());
}


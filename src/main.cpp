#include <fcgi_stdio.h>
#include <iostream>
#include <string.h>
#include <pthread.h> // For pthreads
#include <unistd.h>
#include <routes.h>
#include "Logger.h"
#include "db.h"
#include "auth.h"

using namespace std;

FCGX_Request request;

int main()
{
    Logger logger("/home/girish/logs/app.log", 1024 * 1024); // Log file with 1MB max size
    logger.log(Logger::LogLevel::INFO, "User service started");

    // Initialize database
    initDatabase();

    // FCGX_Request request;
    FCGX_Init();
    FCGX_InitRequest(&request, 0, 0);

    while (FCGX_Accept_r(&request) == 0)
    {
        const char *requestMethod = FCGX_GetParam("REQUEST_METHOD", request.envp);
        const char *requestUri = FCGX_GetParam("REQUEST_URI", request.envp);
        char *authHeader = FCGX_GetParam("HTTP_AUTHORIZATION", request.envp);
        char *queryString = FCGX_GetParam("QUERY_STRING", request.envp);

        logger.log(Logger::LogLevel::INFO,  "request method : %s, request uri : %s", requestMethod, requestUri);

        if (requestMethod && requestUri)
        {
            if (authHeader && validate_digest(requestMethod, requestUri, authHeader))
            {
                if (strcmp(requestMethod, "POST") == 0 && strcmp(requestUri, "/register") == 0) {
                    handleUserRegistration(request);
                } else if (strcmp(requestMethod, "POST") == 0 && strcmp(requestUri, "/login") == 0) {
                    handleUserLogin(request);
                } else if (strcmp(requestMethod, "GET") == 0 && strcmp(requestUri, "/users") == 0) {
                    handleGetUsers(request);
                } else if (strcmp(requestMethod, "GET") == 0 && strncmp(requestUri, "/users/", 7) == 0) {
                    std::string username = requestUri + 7;
                    handleGetUsers(request, username);
                } else {
                    sendResponse(request, 404, "Not Found");
                }
            }
            else
            {
                send_auth_challenge(request);
            }
        }

        FCGX_Finish_r(&request);
    }

    closeDatabase();

    return 0;
}

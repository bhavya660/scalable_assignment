#ifndef ROUTES_H
#define ROUTES_H

#include <fcgi_stdio.h>
#include <string>

#define RESTAURANT_OWNER 5

void handleUserRegistration(FCGX_Request& request);
void handleUserLogin(FCGX_Request& request);
void handleGetUsers(FCGX_Request& request, const std::string& username = "");
void sendResponse(FCGX_Request& request, int statusCode, const std::string& message);

#endif
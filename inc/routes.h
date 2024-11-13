#ifndef ROUTES_H
#define ROUTES_H

#include <fcgi_stdio.h>
#include <string>

void handleUserRegistration(FCGX_Request& request);
void handleUserLogin(FCGX_Request& request);
void sendResponse(FCGX_Request& request, int statusCode, const std::string& message);

#endif
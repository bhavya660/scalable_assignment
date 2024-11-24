#ifndef AUTH_H
#define AUTH_H

#include <string>
#include <fcgi_stdio.h>
#include <bits/stdc++.h>
#include <openssl/md5.h>

std::string md5(const std::string &input);

std::map<std::string, std::string> parse_authorization_header(const std::string &authHeader);

bool validate_digest(const char *method, const char *uri, const char *authHeader);

void send_auth_challenge(FCGX_Request &request);

std::string generate_nonce();

#endif 

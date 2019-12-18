#include "Saleae.h"

#include <cstdio>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <iostream>

Saleae::Saleae(const char addr[], std::uint16_t port)
{
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        std::printf("Cannot create socket\n");
        return;
    }

    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    if(inet_pton(AF_INET, addr, &serverAddress.sin_addr)<=0)  
    { 
        std::printf("Invalid address\n"); 
        return; 
    } 
   
    if (connect(fd, reinterpret_cast<struct sockaddr*>(&serverAddress), sizeof(serverAddress)) < 0)
    {
        std::printf("Connection failed\n");
        return;
    }
}

bool Saleae::ValidateResponse(std::string response) const
{
    std::string ack = response.substr(response.find_last_of("A"));
    return ack.compare("ACK") == 0;
}

std::uint32_t Saleae::GetNumSamples() const
{
    char rsp[1024];
    SendCommand("GET_NUM_SAMPLES", rsp, 1024);

    std::string response(rsp);
    if (ValidateResponse(response))
    {
        std::string valStr = response.substr(0, response.find("\n"));
        return std::stoi(valStr, 0, 10);
    }
    return 0;
}

void Saleae::SendCommand(const char cmd[], char response[], std::uint32_t rspLen) const
{
    // strlen + 1 to include null string termination
    send(fd, cmd, std::strlen(cmd) + 1, 0);
    ssize_t len = recv(fd, response, rspLen, 0);
    if (len >= 0)
        response[len] = '\0';    
}

#ifndef SALEAE_H_
#define SALEAE_H_

#include <cstdint>
#include <string>

class Saleae
{
public:
    Saleae(const char addr[], std::uint16_t port);
    std::uint32_t GetNumSamples() const;

private:
    bool ValidateResponse(std::string response) const;
    void SendCommand(const char cmd[], char response[], std::uint32_t rspLen) const;

    int fd;
};

#endif
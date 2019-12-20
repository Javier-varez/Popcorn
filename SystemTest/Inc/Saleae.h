#ifndef SALEAE_H_
#define SALEAE_H_

#include <cstdint>
#include <string>

class Saleae
{
public:
    Saleae(const char addr[], std::uint16_t port);
    bool GetNumSamples(std::uint32_t &samples) const;
    bool SetNumSamples(std::uint32_t numSamples) const;
    bool SetSampleRate(std::uint32_t sampleRate) const;
    bool SetCaptureSeconds(double seconds) const;
    bool Capture() const;
    bool ExportData(std::string filename) const;

private:
    bool ValidateResponse(std::string response) const;
    void SendCommand(const char cmd[], char response[], std::uint32_t rspLen) const;

    int fd;
};

#endif
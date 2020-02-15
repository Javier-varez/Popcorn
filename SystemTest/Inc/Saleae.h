#ifndef SALEAE_H_
#define SALEAE_H_

#include <cstdint>
#include <string>
#include <vector>

class Saleae
{
public:
    struct SampleData
    {
        uint64_t timestamp;
        uint8_t data;
    } __attribute__((packed));

    enum Channels
    {
        Channel_0 = 0x01,
        Channel_1 = 0x02,
        Channel_2 = 0x04,
        Channel_3 = 0x08,
        Channel_4 = 0x10,
        Channel_5 = 0x20,
        Channel_6 = 0x40,
        Channel_7 = 0x80,
    };

    Saleae(const char addr[], std::uint16_t port);
    bool GetNumSamples(std::uint32_t &samples) const;
    bool SetNumSamples(std::uint32_t numSamples) const;
    bool GetSampleRate(std::uint32_t &sampleRate) const;
    bool SetSampleRate(std::uint32_t sampleRate);
    bool SetCaptureSeconds(double seconds);
    bool Capture() const;
    bool ExportData(const std::string& filename) const;
    std::vector<SampleData> ParseData(const std::string&  filename) const;
    std::pair<double, double> GetFrequency(const std::vector<SampleData>& samples, const Channels channel) const;
    std::pair<double, double> GetActiveTime(const std::vector<SampleData>& samples, const Channels channel) const;
private:
    bool ValidateResponse(std::string response) const;
    void SendCommand(const char cmd[], char response[], std::uint32_t rspLen) const;

    int fd;
    uint32_t m_sample_rate;
    double m_sample_time;
};

#endif

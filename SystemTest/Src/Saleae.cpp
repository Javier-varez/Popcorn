#include "Saleae.h"

#include <cstdio>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cmath>
#include <signal.h>

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

    GetSampleRate(m_sample_rate);
}

bool Saleae::ValidateResponse(std::string response) const
{
    if (response.length() == 0) return false;

    std::string ack = response.substr(response.find_last_of("A"));
    return ack.compare("ACK") == 0;
}

bool Saleae::GetNumSamples(std::uint32_t& samples) const
{
    char rsp[1024];
    SendCommand("GET_NUM_SAMPLES", rsp, 1024);

    std::string response(rsp);
    if (ValidateResponse(response))
    {
        std::string valStr = response.substr(0, response.find("\n"));
        samples = std::stoi(valStr, 0, 10);
        return true;
    }
    return false;
}

bool Saleae::SetNumSamples(std::uint32_t numSamples) const
{
    char rsp[1024];
    char cmd[256];
    std::snprintf(cmd, sizeof(cmd), "SET_NUM_SAMPLES, %u", numSamples);
    SendCommand(cmd, rsp, sizeof(rsp));

    std::string response(rsp);
    if (ValidateResponse(response))
    {
        return true;
    }
    return false;
}

bool Saleae::GetSampleRate(std::uint32_t& sampleRate) const
{
    char rsp[1024];
    SendCommand("GET_SAMPLE_RATE", rsp, 1024);

    std::string response(rsp);
    if (ValidateResponse(response))
    {
        std::string valStr = response.substr(0, response.find("\n"));
        sampleRate = std::stoi(valStr, 0, 10);
        return true;
    }
    return false;
}

bool Saleae::SetSampleRate(std::uint32_t sampleRate)
{
    char rsp[1024];
    char cmd[256];
    std::snprintf(cmd, sizeof(cmd), "SET_SAMPLE_RATE, %u, %u", sampleRate, sampleRate);
    SendCommand(cmd, rsp, sizeof(rsp));

    std::string response(rsp);
    if (ValidateResponse(response))
    {
        m_sample_rate = sampleRate;
        return true;
    }
    return false;
}

bool Saleae::SetCaptureSeconds(double seconds) const
{
    char rsp[1024];
    char cmd[256];
    std::snprintf(cmd, sizeof(cmd), "SET_CAPTURE_SECONDS, %f", seconds);
    SendCommand(cmd, rsp, sizeof(rsp));

    std::string response(rsp);
    if (ValidateResponse(response))
    {
        return true;
    }
    return false;
}

bool Saleae::Capture() const
{
    char rsp[1024];
    SendCommand("CAPTURE", rsp, sizeof(rsp));

    std::string response(rsp);
    if (ValidateResponse(response))
    {
        return true;
    }
    return false;
}

bool Saleae::ExportData(const std::string& filename) const
{
    char rsp[1024];
    char cmd[256];
    snprintf(cmd, sizeof(cmd), 
             "EXPORT_DATA2, "
             "%s, "
             "ALL_CHANNELS, "
             "ALL_TIME, "
             "BINARY, "
             "ON_CHANGE, "
             "NO_SHIFT, "
             "8",
             filename.c_str());
    SendCommand(cmd, rsp, sizeof(rsp));

    std::string response(rsp);
    if (ValidateResponse(response))
    {
        return true;
    }
    return false;
}

std::vector<Saleae::SampleData> Saleae::ParseData(const std::string& filename) const
{
    constexpr std::size_t chunck_size = 2048 * sizeof(Saleae::SampleData);
    uint8_t buf[chunck_size];

    std::vector<Saleae::SampleData> samples;

    int binfile = open(filename.c_str(), O_RDONLY);
    if (binfile < 0) return samples;

    int len = 0;
    do {
        len = read(binfile, buf, chunck_size);
        Saleae::SampleData *sample_ptr = reinterpret_cast<Saleae::SampleData*>(buf);
        for (unsigned int i = 0; i < len / sizeof(Saleae::SampleData); i++)
        {
            samples.push_back(sample_ptr[i]);
        }
    } while (len != 0);

    return samples;
}

template<class T>
std::pair<double, double> CalculateMeanAndStdDev(const std::vector<T>& dataVector)
{
    double mean = 0.0;
    // Compute mean
    for (auto data: dataVector)
    {
        mean += data;
    }
    mean /= dataVector.size();

    double stddev = 0.0;
    // Compute stddev
    for (auto data: dataVector)
    {
        stddev += std::pow(data - mean, 2);
    }
    stddev /= dataVector.size() - 1;
    stddev = std::sqrt(stddev);

    return std::make_pair(mean, stddev);
}

std::pair<double, double> Saleae::GetFrequency(const std::vector<Saleae::SampleData>& samples, const Saleae::Channels channel) const
{
    uint8_t prevState = 0xFF;
    uint64_t lastTimestamp = 0x0000000000000000;
    std::vector<double> frequencyData;

    for (auto sample: samples)
    {
        uint8_t currentState = sample.data & channel;

        if (sample.timestamp == 0x00)
        {
            prevState = sample.data & channel;
            continue;
        }
        else if (currentState != prevState)
        {
            uint64_t currentTimestamp = sample.timestamp;
            if (lastTimestamp != 0)
            {
                uint64_t currentPeriod = currentTimestamp - lastTimestamp;
                frequencyData.push_back(static_cast<double>(m_sample_rate) / currentPeriod);
            }
            prevState = currentState;
            lastTimestamp = currentTimestamp;
        }
    }

    return CalculateMeanAndStdDev(frequencyData);
}

void Saleae::SendCommand(const char cmd[], char response[], std::uint32_t rspLen) const
{
    // strlen + 1 to include null string termination
    send(fd, cmd, std::strlen(cmd) + 1, MSG_NOSIGNAL);
    ssize_t len = recv(fd, response, rspLen, 0);
    if (len >= 0)
        response[len] = '\0';
    else
        response[0] = '\0';
}

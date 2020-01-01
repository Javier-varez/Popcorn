#include "Saleae.h"

#include <cstdio>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <cstring>
#include <vector>

static bool FileExists(std::string filePath)
{
    if (FILE* file = fopen(filePath.c_str(), "r"))
    {
        fclose(file);
        return true;
    }
    return false;
}

static std::string GetEnvironmentVariable(std::string envVar, char* envp[])
{
    envVar.append("=");
    while (*envp != NULL)
    {
        if (std::strncmp(envVar.c_str(), *envp, envVar.length()) == 0)
        {
            break;
        }
        envp++;
    }

    if (*envp == nullptr)
    {
        return "";
    }
    std::string Value(*envp);
    Value = Value.substr(Value.find("=") + 1);
    return Value;
}

static std::string FindExecutableInPath(std::string program, char* envp[])
{
    // Parse path
    std::vector<std::string> paths;
    std::string Path(GetEnvironmentVariable("PATH", envp));

    do
    {
        auto nextColon = Path.find(":");
        std::string newPath = Path.substr(0, nextColon);
        paths.push_back(newPath);
        Path = Path.substr(nextColon + 1);
    } while(Path.find(":") != Path.npos);
    paths.push_back(Path);

    for (std::string path: paths)
    {
        std::string file = path.append("/").append(program);
        if (FileExists(file))
        {
            return file;
        }
    }

    std::printf("Exec not found\n");
    return "";
}

//static int Execute(
//    const char* filename,
//    char* const argv[],
//    char* const envp[])
//{
//    int pid = fork();
//    if (pid == 0)
//    {
//        // Child
//        int fd = open("/dev/null",O_WRONLY | O_CREAT, 0666);
//        dup2(fd, 1);
//        dup2(fd, 2);
//        close(fd);
//        execve(filename, argv, envp);
//        return 0;
//    }
//    else
//    {
//        // Parent
//        return pid;
//    }
//}

//static int childPid = 0;

static void ExitHandler(int errCode = 0)
{
    //kill(childPid, SIGTERM);
    exit(errCode);
}

static bool ValidateFrequency(std::pair<double, double> freq, double target)
{
    double tolerance = 0.001;

    // Validate mean
    if ((freq.first > target + tolerance) ||
        (freq.first < target - tolerance))
    {
        return false;
    }

    // Validate stddev
    if (freq.second > tolerance)
    {
        return false;
    }
    return true;
}

int main(int argc, char* argv[], char* envp[])
{
    const char filename[] = "Logic";
    std::string programPath = FindExecutableInPath(filename, envp);
    //childPid = Execute(programPath.c_str(), argv, envp);

    sleep(1);

    Saleae saleae("127.0.0.1", 10429);

    if (!saleae.SetSampleRate(24000000))
    {
        printf("Error setting sample rate\n");
        ExitHandler(-1);
    }
    if (!saleae.SetCaptureSeconds(10.0))
    {
        printf("Error setting capture seconds\n");
        ExitHandler(-1);
    }
    saleae.Capture();

    std::string pwd(GetEnvironmentVariable("PWD", envp));

    pwd.append("/output.bin");
    saleae.ExportData(pwd);
    auto samples = saleae.ParseData(pwd);

    bool fail = false;

    std::pair<double, double> freq = saleae.GetFrequency(samples, Saleae::Channel_0);
    printf("Channel 0 freq = %f, std_dev = %f\n", freq.first, freq.second);
    if (!ValidateFrequency(freq, 0.66666666))
    {
        printf("Failed validation for channel 0\n");
        fail = true;
    }

    freq = saleae.GetFrequency(samples, Saleae::Channel_1);
    printf("Channel 1 freq = %f, std_dev = %f\n", freq.first, freq.second);
    if (!ValidateFrequency(freq, 1.0))
    {
        printf("Failed validation for channel 1\n");
        fail = true;
    }

    ExitHandler(fail ? -1 : 0);
    return 0;
}

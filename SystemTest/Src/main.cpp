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

static int Execute(
    const char* filename,
    char* const argv[],
    char* const envp[])
{
    int pid = fork();
    if (pid == 0)
    {
        // Child
        int fd = open("/dev/null",O_WRONLY | O_CREAT, 0666);
        dup2(fd, 1);
        dup2(fd, 2);
        close(fd);
        execve(filename, argv, envp);
        return 0;
    }
    else
    {
        // Parent
        return pid;
    }
}

static int childPid = 0;

static void ExitHandler()
{
    kill(childPid, SIGTERM);
    exit(1);
}

int main(int argc, char* argv[], char* envp[])
{
    const char filename[] = "Logic";
    std::string programPath = FindExecutableInPath(filename, envp);
    childPid = Execute(programPath.c_str(), argv, envp);

    sleep(1);

    Saleae saleae("127.0.0.1", 10429);

    if (!saleae.SetSampleRate(24000000))
    {
        printf("Error setting sample rate\n");
        ExitHandler();
    }
    if (!saleae.SetCaptureSeconds(10.0))
    {
        printf("Error setting capture seconds\n");
        ExitHandler();
    }
    saleae.Capture();

    std::string pwd(GetEnvironmentVariable("PWD", envp));
    printf("pwd %s\n", pwd.c_str());

    pwd.append("/output.bin");
    saleae.ExportData(pwd);

    int binfile = open(pwd.c_str(), O_RDONLY);
    uint8_t buf[2048];
    int len = read(binfile, buf, 2048);
    uint8_t* ptr = buf;
    for (int i = 0; i < len / 9; i++)
    {
        uint64_t *timestamp_ptr = reinterpret_cast<uint64_t*>(ptr);
        uint8_t* byte = ptr + sizeof(uint64_t);
        printf("Sample #%d: timestamp %lu, val 0x%02x\n", i, *timestamp_ptr, *byte);
        ptr += sizeof(uint64_t) + sizeof(uint8_t);
    }

    ExitHandler();
    return 0;
}
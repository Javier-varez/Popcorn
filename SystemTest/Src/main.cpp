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

static std::string FindExecutableInPath(std::string program, char* envp[])
{
    while (*envp != NULL)
    {
        if (std::strncmp("PATH=", *envp, std::strlen("PATH=")) == 0)
        {
            break;
        }
        envp++;
    }

    if (*envp == NULL)
    {
        return "";
    }

    // Parse path
    std::vector<std::string> paths;
    std::string Path(*envp);

    Path = Path.substr(Path.find("=") + 1);
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
    return Path;
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
        int fd = open("/dev/null",O_WRONLY | O_CREAT, 0666);   // open the file /dev/null
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
    std::printf("Get samples\n");

    std::uint32_t nSamples = 0; 
    if (!saleae.SetNumSamples(1024))
    {
        printf("Error setting samples\n");
        ExitHandler();
    }
    if (!saleae.GetNumSamples(nSamples))
    {
        printf("Error getting samples\n");
        ExitHandler();
    }
    printf("nsamples: %d\n", nSamples);

    if (!saleae.SetSampleRate(24000000))
    {
        printf("Error setting sample rate\n");
        ExitHandler();
    }

    ExitHandler();
    return 0;
}
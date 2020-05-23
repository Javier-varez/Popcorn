/*
 * This file is part of the Cortex-M Scheduler
 * Copyright (c) 2020 Javier Alvarez
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <cstdio>
#include <cstring>
#include <vector>

#include "Saleae.h"

static bool FileExists(std::string filePath) {
  if (FILE* file = fopen(filePath.c_str(), "r")) {
    fclose(file);
    return true;
  }
  return false;
}

static std::string GetEnvironmentVariable(std::string envVar, char* envp[]) {
  envVar.append("=");
  while (*envp != NULL) {
    if (std::strncmp(envVar.c_str(), *envp, envVar.length()) == 0) {
      break;
    }
    envp++;
  }

  if (*envp == nullptr) {
    return "";
  }
  std::string Value(*envp);
  Value = Value.substr(Value.find("=") + 1);
  return Value;
}

static std::string FindExecutableInPath(std::string program, char* envp[]) {
  // Parse path
  std::vector<std::string> paths;
  std::string Path(GetEnvironmentVariable("PATH", envp));

  do {
    auto nextColon = Path.find(":");
    std::string newPath = Path.substr(0, nextColon);
    paths.push_back(newPath);
    Path = Path.substr(nextColon + 1);
  } while (Path.find(":") != Path.npos);
  paths.push_back(Path);

  for (std::string path : paths) {
    std::string file = path.append("/").append(program);
    if (FileExists(file)) {
      return file;
    }
  }

  std::printf("Exec not found\n");
  return "";
}

static int Execute(const char* filename,
                   const char* const argv[],
                   const char* const envp[]) {
  int pid = fork();
  if (pid == 0) {
    // Child
    int fd = open("/dev/null", O_WRONLY | O_CREAT, 0666);
    dup2(fd, 1);
    dup2(fd, 2);
    close(fd);
    execve(filename,
           const_cast<char* const*>(argv),
           const_cast<char* const*>(envp));
    return 0;
  } else {
    // Parent
    return pid;
  }
}

static int LogicPID = 0;

static void ExitHandler(int errCode = 0) {
  kill(LogicPID, SIGTERM);
  exit(errCode);
}

static bool ValidateFrequency(std::pair<double, double> freq, double target) {
  double tolerance = 0.001;

  // Validate mean
  if ((freq.first > target + tolerance) ||
      (freq.first < target - tolerance)) {
    return false;
  }

  // Validate stddev
  if (freq.second > tolerance) {
    return false;
  }
  return true;
}

static bool ValidateCPUUsage(double usage_mean,
                             double std_dev,
                             double maxMean,
                             double maxDev) {
  if (usage_mean > maxMean) {
    return false;
  }

  if (std_dev > maxDev) {
    return false;
  }
  return true;
}

int main(int argc, char* argv[], char* envp[]) {
  std::string LogicPath = FindExecutableInPath("Logic", envp);

  const char* const LogicArgs[] = {
    "Logic",
    nullptr
  };

  LogicPID = Execute(LogicPath.c_str(), LogicArgs, envp);
  sleep(5);

  Saleae saleae("127.0.0.1", 10429);

  constexpr uint64_t sample_rate = 24'000'000;
  constexpr double capture_seconds = 10.0;

  if (!saleae.SetSampleRate(sample_rate)) {
    printf("Error setting sample rate\n");
    ExitHandler(-1);
  }
  if (!saleae.SetCaptureSeconds(capture_seconds)) {
    printf("Error setting capture seconds\n");
    ExitHandler(-1);
  }
  saleae.Capture();

  std::string outputDir(GetEnvironmentVariable("PWD", envp));
  outputDir.append("/output.bin");

  if (!saleae.ExportData(outputDir)) {
    printf("Error when exporting data\n");
    ExitHandler(-1);
  }
  auto samples = saleae.ParseData(outputDir);

  bool fail = false;

  std::pair<double, double> freq = saleae.GetFrequency(samples, Saleae::Channel_0);
  printf("Channel 0 freq = %f, std_dev = %f\n", freq.first, freq.second);
  if (!ValidateFrequency(freq, 0.66666666)) {
    printf("Failed validation for channel 0\n");
    fail = true;
  }

  freq = saleae.GetFrequency(samples, Saleae::Channel_1);
  printf("Channel 1 freq = %f, std_dev = %f\n", freq.first, freq.second);
  if (!ValidateFrequency(freq, 1.0)) {
    printf("Failed validation for channel 1\n");
    fail = true;
  }

  auto cpu_active_data = saleae.GetActiveTime(samples, Saleae::Channel_2);
  printf("CPU Usage: %f %%, std_dev = %f\n",
         cpu_active_data.first * 100.0,
         cpu_active_data.second);

  if (!ValidateCPUUsage(cpu_active_data.first,
                        cpu_active_data.second,
                        0.02,
                        0.001)) {
    printf("CPU Usage validation failed\n");
    fail = true;
  }

  ExitHandler(fail ? -1 : 0);
  return 0;
}

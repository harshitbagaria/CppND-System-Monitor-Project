#include <dirent.h>
#include <unistd.h>
#include <string>
#include <vector>

#include "linux_parser.h"
using std::stol;
using std::stof;
using std::string;
using std::to_string;
using std::vector;

// DONE: An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, tmp,  kernel;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> tmp >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

// Read and return the system memory utilization
float LinuxParser::MemoryUtilization() { 
  string line;
  string key;
  string value;
  long memTotal, memFree;
  std::ifstream filestream(kProcDirectory+kMeminfoFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "MemTotal") 
          memTotal=stol(value);
        else if(key == "MemFree")
          memFree=stol(value);
      }
    }
  }
  return (float)(memTotal-memFree)/memTotal;
}

// Read and return the system uptime
long LinuxParser::UpTime() { 
  string line;
  string value;
  std::ifstream filestream(kProcDirectory+kUptimeFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line); 
      std::istringstream linestream(line);
      linestream >> value;
  }
 return stol(value);
}

// Read and return the number of jiffies for the system
long LinuxParser::Jiffies() { 
  return ActiveJiffies() + IdleJiffies();
}

// Read and return the number of active jiffies for a PID
long LinuxParser::ActiveJiffies(int pid) { 
  string value, line;
  long actJiff=0;
  std::ifstream filestream(kProcDirectory+to_string(pid)+kStatFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    for (int i =0; i < 13; i++)
      linestream >> value;

    linestream >> value; //utime
    actJiff+=stol(value);
    linestream >> value; //stime
    actJiff+=stol(value);
    linestream >> value; //cutime
    actJiff+=stol(value);
    linestream >> value; //cstime
    actJiff+=stol(value);
  }
  return actJiff;
}

// Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() { 
  vector<string> cpu = CpuUtilization();
  return (stol(cpu[CPUStates::kUser_]) + stol(cpu[CPUStates::kNice_]) + stol(cpu[CPUStates::kSystem_]) + stol(cpu[CPUStates::kIRQ_]) + 
          stol(cpu[CPUStates::kSoftIRQ_]) + stol(cpu[CPUStates::kSteal_]) + stol(cpu[CPUStates::kGuest_]) + stol(cpu[CPUStates::kGuestNice_]) );
}

// Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() {
  vector<string> cpu = CpuUtilization();
  return (stol(cpu[CPUStates::kIdle_]) + stol(cpu[CPUStates::kIOwait_]) );
}

// Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() { 
  vector<string> cpu;  
  string line;
  string value;
  std::ifstream filestream(kProcDirectory+kStatFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line); 
    std::istringstream linestream(line);
    linestream >> value;
    while (linestream >> value)
      cpu.push_back(value);
  }  
  return cpu; 
}

// Read and return the total number of processes
int LinuxParser::TotalProcesses() { 
  string line;
  string key, value;
  int totProc;
  std::ifstream filestream(kProcDirectory+kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
        std::istringstream linestream(line);
        while (linestream >> key >> value) {
          if (key == "processes") 
          {  
            totProc=std::stoi(value);
            return totProc;
          }
        }
      }
  }
  return totProc;
}

// Read and return the number of running processes
int LinuxParser::RunningProcesses() {   
  string line;
  string key, value;
  std::ifstream filestream(kProcDirectory+kStatFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
        std::istringstream linestream(line);
        while (linestream >> key >> value) {
          if (key == "procs_running") 
            return std::stoi(value);
        }
      }
  }
  return std::stoi(value);
}

// Read and return the command associated with a process
string LinuxParser::Command(int pid) { 
  string line;
  std::ifstream filestream(kProcDirectory+to_string(pid)+kCmdlineFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    return line; 
  }
  return line; 
}

// Read and return the memory used by a process
string LinuxParser::Ram(int pid) { 
  string line;
  string key, value;
  std::ifstream filestream(kProcDirectory+to_string(pid)+kStatusFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
        std::istringstream linestream(line);
        while (linestream >> key >> value) {
          if (key == "VmSize:") 
            return to_string(std::stoi(value)/1024);
        }
      }
  }
  return value;
}

// Read and return the user ID associated with a process
string LinuxParser::Uid(int pid) { 
  string line;
  string key, value;
  std::ifstream filestream(kProcDirectory+to_string(pid)+kStatusFilename);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
        std::istringstream linestream(line);
        while (linestream >> key >> value) {
          if (key == "Uid:") 
            return value;
        }
      }
   }
   return value;
}

// Read and return the user associated with a process
string LinuxParser::User(int pid) { 
  string line;
  string key, value, tmp;
  string uid=Uid(pid);

  std::ifstream filestream(kPasswordPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
        std::replace(line.begin(), line.end(), ':', ' ');
        std::istringstream linestream(line);
        while (linestream >> key >> tmp >> value) {
          if (value == uid) 
            return key;
        }
      }
  }
  return key;
}

// Read and return the uptime of a process
long LinuxParser::UpTime(int pid) { 
  string line;
  string value;
  std::ifstream filestream(kProcDirectory+to_string(pid)+kStatFilename);
  if (filestream.is_open()) {
    std::getline(filestream, line);
    std::istringstream linestream(line);
    for (int i = 0; i < 22; i++)
      linestream >> value;
   }
   return stol(value)/sysconf(_SC_CLK_TCK);
}
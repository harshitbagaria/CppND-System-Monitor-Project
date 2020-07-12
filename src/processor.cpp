#include "processor.h"
#include "linux_parser.h"
// Return the aggregate CPU utilization
float Processor::Utilization() { 
    return (float)LinuxParser::ActiveJiffies()/(float)LinuxParser::Jiffies();
 }
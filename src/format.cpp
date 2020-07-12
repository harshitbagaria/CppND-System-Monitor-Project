#include <string>

#include "format.h"

using std::string;
using std::to_string;

// Complete this helper function
// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
string Format::ElapsedTime(long seconds) { 
    long sec=seconds%60;
    long min=seconds/60;
    long hrs=min/60;
    min=min%60;
    //append 0 in case of single digit
    string h = (hrs>9)?to_string(hrs):"0"+to_string(hrs);
    string m = (min>9)?to_string(min):"0"+to_string(min);
    string s = (sec>9)?to_string(sec):"0"+to_string(sec);

    return (h+":"+m+":"+s); 
}
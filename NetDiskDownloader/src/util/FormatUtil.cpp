#include "FormatUtil.h"
#include <sstream>
#include <iomanip>
std::wstring SpeedText(double n){std::wostringstream s;s<<std::fixed<<std::setprecision(1);if(n>=1048576)s<<n/1048576<<L" MB/s";else if(n>=1024)s<<n/1024<<L" KB/s";else s<<n<<L" B/s";return s.str();}

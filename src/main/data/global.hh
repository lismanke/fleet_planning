#ifndef GLOBAL_HH
#define GLOBAL_HH

#ifndef EXTERN
#define EXTERN extern
#endif

#include <boost/date_time/posix_time/posix_time.hpp>
#include "boost/date_time/gregorian/gregorian.hpp"

using namespace std;
using namespace boost::gregorian;

extern boost::posix_time::ptime now;
extern date todaysDateInLocalTZ;
extern std::ostringstream date_namebuf;
extern string Path;
extern int FS;
extern vector<string> name;
extern bool is_benders;
extern int T_starting;
#endif // GLOBAL_HH

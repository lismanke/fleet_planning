#include "../data/global.hh"

boost::posix_time::ptime now = boost::posix_time::second_clock::local_time();
date todaysDateInLocalTZ = day_clock::local_day();
std::ostringstream date_namebuf(static_cast<std::ostringstream&&>(std::ostringstream() <<todaysDateInLocalTZ.day()
                                                                                       <<"_"<<todaysDateInLocalTZ.month()
                                                                                       <<"_"<<todaysDateInLocalTZ.year()
                                                                                       <<"_at_"<<now.time_of_day().hours()
                                                                                       <<"_"<<now.time_of_day().minutes()
                                                                                       <<"_"<<now.time_of_day().seconds()));

string Path;
int FS = 1;
vector<string> name = {"eu", "i"};
bool is_benders;
int T_starting;

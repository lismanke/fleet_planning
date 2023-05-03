#ifndef R_DATASET_HH
#define R_DATASET_HH

#include <utility>
#include <iostream>
#include <random>
#include <sstream>
#include <iomanip>
#include <set>
#include <math.h>
#include "data.hh"

//class to randomly generate all necessary data and to store it in the associated data structures
class RandDataset
{
  private:
    std::mt19937& generator;
    int T_max;
    Sizes& sizes;
    int range;

    vector<Retrofit> retrofits;
    Market_Situation market;
    std::vector<Aircraft_Properties> aircrafts;
    std::vector<Net_Class> net_classes;

  public:
    RandDataset(Sizes& sizes, int T_max, std::mt19937& generator, int range);

    ~RandDataset(){}

    //returns market situation data as the associated struct Market_Situation
    Market_Situation& get_market_situation()
    {
      return(market);
    }

    //returns all aircrafts as a vector from the struct Aircraft_Properties
    std::vector<Aircraft_Properties>& get_aircrafts()
    {
      return(aircrafts);
    }

    //returns all netclasses inside a vector containing of struct Net_Class
    std::vector<Net_Class> get_netclasses()
    {
      return(net_classes);
    }

    //generates aircraft data
    void generate_aircrafts();
    //generates every netclass
    void generate_netclasses();
    //generates data for current market sitation
    void generate_market_situation();
};

//class to store all used data
class Instance
{
  private:
    int T_max;
    Market_Situation market;
    vector<Aircraft_Properties> aircrafts;
    vector<Net_Class> net_classes;

    double maximum_flight_hours(int k, int MUik, int k_ref)
    {
      return(MUik*pow(0.99,k-k_ref));
    }

  public:
    Instance(std::string source, int T_max);

    Instance(RandDataset& r_set, int T_max): T_max(T_max), aircrafts(r_set.get_aircrafts()), market(r_set.get_market_situation()), net_classes(r_set.get_netclasses()){}

    ~Instance(){}

    int get_T_max()
    {
      return(T_max);
    }

    //returns market situation data as the associated struct Market_Situation
    Market_Situation& get_market()
    {
      return(market);
    }

    //returns all aircrafts as a vector from the struct Aircraft_Properties
    vector<Aircraft_Properties>& get_aircrafts()
    {
      return(aircrafts);
    }

    //returns all netclasses inside a vector containing of struct Net_Class
    vector<Net_Class>& get_net_classes()
    {
      return(net_classes);
    }
};

#endif // R_DATASET_HH

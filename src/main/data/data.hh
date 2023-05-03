#ifndef DATA_HH
#define DATA_HH

#include <iostream>
#include <vector>
#include <utility>
#include <assert.h>

using namespace std;

struct FU_Param                                                               //technological fuel parameter
{
  double e_i;
  double f_i;
  int    g_i;
  int    h_i;
};

struct BT_Param                                                               //parameter to compute needed time for a flight
{
  double c_i;
  double d_i;
};

struct retrofit_candidate
{
  int idx;                                                                    //identification number of retrofit
  int type;                                                                   //corresponding index number for aircraft type
  double price;                                                               //price of the retrofit in M$
  int purchasable_since;                                                      //period of market launch of retrofit
};

struct TechnicalDetails
{
  int nmb_eng;                                                                  //number of engines
  int slst;                                                                     //Sea level static thrust in [kN]
  int mtgw;                                                                     //Maximum takeoﬀ gross weight in [t]
  int afw;                                                                      //airframe weight in [t]
};

struct Sizes
{
  std::pair<int, int> age_range={0,0};
  int netclasses = 0;
  int types      = 0;
};

class Retrofit
{
  private:
    int max_per_t;
    vector<retrofit_candidate> candidates;

  public:
    Retrofit(int max_per_t, vector<retrofit_candidate> candidates): candidates(candidates), max_per_t(max_per_t){}

    ~Retrofit(){}

    vector<retrofit_candidate>& get_candidates()
    {
      return(candidates);
    }

    int get_max_per_t()
    {
      return(max_per_t);
    }
};

class Market_Situation
{
  private:
    //fuel prices per gallon in $
    vector<double> fuel_prices;

    //price per tonne CO_2 emission in $, pos 0 = eu, pos 1 = i
    vector<vector<double>> emission_price;

    //proportion of the international emissions to be compensated
    vector<double> prop_em_compensation;

    //number of purchasable aircrafts of type c at time t
    vector<vector<int>> purchasable_c_t;

    //purchasing price for one aircraft of type c at time t in M$
    vector<vector<double>> purchase_price_c_t;

    //retail price for one aircraft of type c at time t
    vector<vector<vector<double>>> retail_price_c_t_age;

    //retrofit info
    vector<Retrofit> retrofits;

  public:
    Market_Situation(){}

    ~Market_Situation(){}

    void insert_data(vector<double> fuel_prices_,
                     std::vector<vector<double>> emission_price_,
                     vector<double> prop_em_compensation_,
                     vector<vector<int>> purchasable_c_t_,
                     vector<vector<double>> purchase_price_c_t_,
                     vector<vector<vector<double>>> retail_price_c_t_age_,
                     vector<Retrofit> retrofits_)
    {
      fuel_prices          = fuel_prices_;
      emission_price       = emission_price_;
      prop_em_compensation = prop_em_compensation_;
      purchasable_c_t      = purchasable_c_t_;
      purchase_price_c_t   = purchase_price_c_t_;
      retail_price_c_t_age = retail_price_c_t_age_;
      retrofits            = retrofits_;
    }

    double get_fuel_price_at(int t)
    {
      return(fuel_prices[t]);
    }

    double get_emission_price_in_as_at(int q, int t)
    {
      return(emission_price[q][t]);
    }

    vector<double> get_prop_em_compensation()
    {
      return(prop_em_compensation);
    }

    int get_nmb_purchasable_aircrafts_of_at(int c, int t)
    {
      return(purchasable_c_t[c][t]);
    }

    double get_purchase_price_for_at(int c,int t)
    {
      return(purchase_price_c_t[c][t]);
    }

    double get_retail_price_for_at_at(int c,int t,int k)
    {
      return(retail_price_c_t_age[c][t][k]);
    }

    vector<Retrofit>& get_retrofits()
    {
      return(retrofits);
    }
};

class Aircraft_Properties
{
  private:
    //maximum service time of aircraft type
    int service_t;

    //number of usable seats
    int nmb_seats;

    //range of aircraft in km
    int range;

    //price M$ at t=1
    double price;

    //nmb of aisles
    int nmb_aisle;

    //fuel use parameter
    FU_Param fuel_use_param;

    //block time parameter
    BT_Param block_time_param;

    //period since aircraft_type is purchasable
    int purchasable_since;

    //period since retrofit is purchasable
    int retro_purchasable_since;

    //number of aircrafts in fleet at t=0
    vector<int> owned_at_start;

    //list of maximum flight hours per year for each year within the service time
    vector<double> hours_p_a;

    //list of aircraft types this aircraft can be retrofitted to
    vector<int> retrofit_aims;

    //price matrix for retrofit r from class c to this at t
    vector<vector<double>> retrofit_price_r_t;

    //retrofitted aircraft (base type if false)
    bool retrofitted;

    //get fuel reduction (for retrofitted)
    pair<int,double> fuel_reduction;

    TechnicalDetails tech_data;

    int origin;

  public:

    Aircraft_Properties(int service_t,
                        int nmb_seats,
                        int range,
                        double price,
                        int nmb_aisle,
                        FU_Param fuel_use_param,
                        BT_Param block_time_param,
                        TechnicalDetails tech_data,
                        int purchasable_since,
                        vector<int> owned_at_start,
                        vector<double> hours_p_a,
                        vector<int> retrofit_aims,
                        vector<vector<double>> retrofit_price_r_t,
                        bool retrofitted):
                        service_t(service_t),
                        nmb_seats(nmb_seats),
                        range(range),
                        price(price),
                        nmb_aisle(nmb_aisle),
                        fuel_use_param(fuel_use_param),
                        block_time_param(block_time_param),
                        tech_data(tech_data),
                        purchasable_since(purchasable_since),
                        owned_at_start(owned_at_start),
                        hours_p_a(hours_p_a),
                        retrofit_aims(retrofit_aims),
                        retrofit_price_r_t(retrofit_price_r_t),
                        retrofitted(retrofitted){}

    Aircraft_Properties(int service_t,
                        int nmb_seats,
                        int range,
                        double price,
                        int nmb_aisle,
                        FU_Param fuel_use_param,
                        BT_Param block_time_param,
                        TechnicalDetails tech_data,
                        int purchasable_since,
                        int retro_purchasable_since,
                        vector<double> hours_p_a,
                        vector<int> retrofit_aims,
                        bool retrofitted,
                        pair<int,double> fuel_reduction,
                        int origin):
                        service_t(service_t),
                        nmb_seats(nmb_seats),
                        range(range),
                        price(price),
                        nmb_aisle(nmb_aisle),
                        fuel_use_param(fuel_use_param),
                        block_time_param(block_time_param),
                        tech_data(tech_data),
                        purchasable_since(purchasable_since),
                        retro_purchasable_since(retro_purchasable_since),
                        hours_p_a(hours_p_a),
                        retrofit_aims(retrofit_aims),
                        retrofitted(retrofitted),
                        fuel_reduction(fuel_reduction),
                        origin(origin){}

    ~Aircraft_Properties(){}

    int get_service_t()
    {
      return(service_t);
    }

    int get_nmb_seats()
    {
      return(nmb_seats);
    }

    int get_range()
    {
      return(range);
    }

    int get_nmb_aisles()
    {
      return(nmb_aisle);
    }

    int get_purchasable_since_period()
    {
      return(purchasable_since);
    }

    void set_purchasable_since_period(int t)
    {
      purchasable_since = t;
    }

    int get_retro_purchasable_since_period()
    {
      return(retro_purchasable_since);
    }

    int get_origin()
    {
      assert(retrofitted);
      return(origin);
    }

    vector<int>& get_nmb_owned_at_start()
    {
      return(owned_at_start);
    }

    void set_nmb_owned_at_start(vector<int> planes)
    {
      owned_at_start = planes;
    }


    double get_hours_p_a_at(int t)
    {
      return(hours_p_a[t]);
    }

    vector<int>& get_retrofit_aims()
    {
      return(retrofit_aims);
    }

    double get_retrofit_price(int r, int t)
    {
      return(retrofit_price_r_t[r][t]);
    }

    bool is_retrofitted()
    {
      return(retrofitted);
    }

    double get_price()
    {
      return(price);
    }

    FU_Param& get_fuel_use_param()
    {
      return(fuel_use_param);
    }

    BT_Param& get_block_time_param()
    {
      return(block_time_param);
    }

    pair<int,double>& get_fuel_reduction()
    {
      return(fuel_reduction);
    }

    TechnicalDetails& get_tech_data()
    {
      return(tech_data);
    }
};

class Net_Class
{
  private:
    //demand of flights
    vector<int> D_flights;

    //demand of flights within each of the aerospaces (as);
    vector<vector<int>> D_flights_in_as;

    //range of seats per flight
    pair<int,int> seats_per_flight;

    //demand of seats
    vector<int> D_seats;

    //demand of seats within EU
    vector<int> D_seats_EU;

    //demand of seats international
    vector<int> D_seats_I;

    // list of distance range
    pair<int,int> dist_range;

    //list of indices of appropriate aircraft types for this net class
    vector<int> usable_aircrafts;

    //list of fuel use per flight for each appropriate aircraft (gallon per flight)
    vector<double> fuel_uses;

    //list of blocked time per flight for each appropriate aircraft
    vector<double> blocked_times;

  public:
    Net_Class(vector<int> D_flights,
              vector<vector<int>> D_flights_in_as,
              pair<int,int> seats_per_flight,
              vector<int> D_seats,
              vector<int> D_seats_EU,
              vector<int> D_seats_I,
              pair<int,int> dist_range,
              vector<int> usable_aircrafts,
              vector<double> fuel_uses,
              vector<double> blocked_times):
              D_flights(D_flights),
              D_flights_in_as(D_flights_in_as),
              seats_per_flight(seats_per_flight),
              D_seats(D_seats),
              D_seats_EU(D_seats_EU),
              D_seats_I(D_seats_I),
              dist_range(dist_range),
              usable_aircrafts(usable_aircrafts),
              fuel_uses(fuel_uses),
              blocked_times(blocked_times){}

    ~Net_Class(){}

    //demand of flights
    vector<int>& get_D_flights()
    {
      return(D_flights);
    }

    //demand of flights within each of the aerospaces (as);
    int get_D_flights_in_as_at_t(int q, int t)
    {
      return(D_flights_in_as[q][t]);
    }

    //set the demand of flights within each of the aerospaces (as);
    void set_D_flights_in_as_at_t(int q, int t, int flights)
    {
      D_flights_in_as[q][t] = flights;
    }

    //range of seats per flight
    pair<int, int> get_seats_per_flight()
    {
      return(seats_per_flight);
    }

    //demand of seats
    vector<int>& get_D_seats()
    {
      return(D_seats);
    }

    //demand of seats within EU
    vector<int>& get_D_seats_EU()
    {
      return(D_seats_EU);
    }

    //demand of seats international
    vector<int>& get_D_seats_I()
    {
      return(D_seats_I);
    }

    // list of distance range
    pair<int,int>& get_dist_range()
    {
      return(dist_range);
    }

    //list of indices of appropriate aircraft types for this net class
    vector<int>& get_usable_aircrafts()
    {
      return(usable_aircrafts);
    }

    //list of fuel use per flight for each appropriate aircraft (gallon per flight)
    vector<double>& get_fuel_uses()
    {
      return(fuel_uses);
    }

    //list of blocked time per flight for each appropriate aircraft
    vector<double>& get_blocked_times()
    {
      return(blocked_times);
    }
  };

#endif // DATA_HH

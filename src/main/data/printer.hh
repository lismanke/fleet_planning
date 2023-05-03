#ifndef PRINTER_HH
#define PRINTER_HH

#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <stdio.h>
#include <math.h>

#include <scip/scip.h>
#include <scip/scipdefplugins.h>

#include "boost/filesystem.hpp"

#include "../data/global.hh"
#include "../data/r_dataset.hh"
#include "../data/data.hh"
#include "../graph/fleet_graph.hh"
#include "../fleet_planning_program.hh"

using namespace std;

//Creates a class to print all informations for every solution in files
class Printer
{
    private: 
    vector<int>            number_both_solved        = {0,0,0,0,0,0,0,0,0,0,0};
    vector<vector<int>>    feasible_instances_solved = {{0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0}};
    vector<vector<int>>    feasible_instances        = {{0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0}};
    vector<vector<double>> average_solving_time      = {{0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.},{0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.}};
    vector<vector<double>> variance_solving_time     = {{0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.},{0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.}};
    vector<vector<int>>    average_number_vars       = {{0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0}};
    vector<vector<int>>    average_number_cons       = {{0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0}};
    vector<vector<int>>    average_solving_nodes     = {{0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0}};
    vector<vector<int>>    average_lp_iterations     = {{0,0,0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0,0,0}};
    vector<vector<double>> average_gaps              = {{0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.},{0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.}};
    vector<vector<double>> solution_gap_ab           = {{0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.},{0.,0.,0.,0.,0.,0.,0.,0.,0.,0.,0.}};

    vector<int> lb_aircraft_types = {2,6,16,2};
    vector<int> lb_flight_range   = {0,4001,9001,0};
    vector<int> lb_netclasses     = {11,21,35,11};
    vector<int> ub_aircraft_types = {5,15,25,25};
    vector<int> ub_flight_range   = {4000,9000,15000,15000};
    vector<int> ub_netclasses     = {20,34,47,47};
    int         T_stats           = T_starting;

    bool   orig_time       = false;
    double orig_sol_buffer = 0.;
    
    public:

    Printer();

    ~Printer(){}

    void print_stats(SCIP* scip, int t, int c, int n, int d, int round, bool benders, bool is_benders);

    void print_full_stats_orig(SCIP* scip, int t, int c, int n, int d, int round, int i, bool endstatistics);

    void print_full_stats_benders(SCIP* scip, int t, int c, int n, int d, int round, bool endstatistics);

    void print_single_results(SCIP* scip, int t, int c, int n, int round, int i, bool feasible);

    void write_stats(SCIP* scip, int i, int j);

    void print_all_headers(int t, int i);

    void reset_full_stats(SCIP* scip, int t, int c, int n, int d, int round, int i)
    {
      if(t>1)
      {
        if(i==1) print_full_stats_benders(scip, t-1, c, n, d, round, true);
        else  print_full_stats_orig(scip, t-1, c, n, d, round, 0, true);
      }

      for(int j=0; j<10; j++)
      {
        feasible_instances[i][j]        = 0;
        feasible_instances_solved[i][j] = 0;
        average_solving_time[i][j]      = 0.;
        variance_solving_time[i][j]     = 0.;
        average_number_vars[i][j]       = 0;
        average_number_cons[i][j]       = 0;
        average_solving_nodes[i][j]     = 0;
        average_lp_iterations[i][j]     = 0;
        average_gaps[i][j]              = 0.;
        solution_gap_ab[i][j]           = 0.;
      }
    }
};

string stringify_number(int n);

void print_scip_solution(SCIP *scip, string problem_name);

void print_sub_scip_solution(SCIP *scip, SCIP* sub_scip, string problem_name);

void print_scip_problem(SCIP* scip, string problem_name);

void print_benders_statistics(SCIP* scip, string name);

void print_net_classes(int t, int c, int n, int rounds, vector<Net_Class>& net_classes, ofstream& data_nc);

void print_retrofits(int t, int c, int n, int rounds, vector<Retrofit>& retrofits, ofstream& data_r);

void print_age_dept_props(int t, int c, int n, int rounds, Market_Situation& market, vector<Aircraft_Properties>& aircrafts, ofstream& data_ad);

void print_aircraft_types_age_indept(int t, int c, int n, int rounds, vector<Aircraft_Properties>& aircrafts, ofstream& data_a);

void print_market_situation(int t, int c, int n, int rounds, Market_Situation& market, vector<Aircraft_Properties>& aircrafts, ofstream& data_ms);

//prints the generated dataset for every instance
void print_dataset(int t, int c, int n, int r, Instance& instance);

#endif // PRINTER_NONOPT_HH

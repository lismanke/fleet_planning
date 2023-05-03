#ifndef FUNCTIONS_HH
#define FUNCTIONS_HH

#include <math.h>
#include <fstream>
#include "data/global.hh"
#include "data/data.hh"
#include "data/r_dataset.hh"
#include "graph/fleet_graph.hh"

#include <scip/scip.h>
#include <scip/scipdefplugins.h>
#include <scip/bendersdefcuts.h>

#include "scip_exception.hh"

//defined in file "function_for_vars.cc"
SCIP* create_basic_scip_program(const std::string& name);

void solve_scip_problem(SCIP* scip, int max_solving_time);

SCIP_VAR* scip_create_var_continous(SCIP* scip, std::ostringstream &namebuf, double lb, double ub, double obj, bool removable);

SCIP_VAR* scip_create_var_int(SCIP* scip, std::ostringstream &namebuf, double lb, double ub, double obj, bool removable);

ostringstream define_namebuf_flow_variables(Fleet_Planning_Graph::edge_iterator ei, FleetGraph& fleet_graph, const Fleet_Planning_Graph& graph,  string problem = "");

pair<double, double> compute_ub_and_obj_value_flow_variables(SCIP* scip, Fleet_Planning_Graph::edge_iterator ei, FleetGraph& fleet_graph, Instance& instance, const Fleet_Planning_Graph& graph);

void add_flow_variables(SCIP* scip, FleetGraph& fleet_graph, Instance& instance, int t = -1);

ostringstream define_namebuf_flight_variables(int t, int l, int c, int a, int q);

double compute_obj_value_flight_variables(int t, int l, int c, int a, vector<Net_Class>& net_classes, vector<Aircraft_Properties>& aircrafts, vector<int> usable_aircrafts, Market_Situation& market);

void add_flight_variables(SCIP* scip, Instance& instance, vector<vector<vector<vector<vector<SCIP_VAR*>>>>>& flight_var_t_l_q_c_a, int t_start, int t_end);

void add_emission_variables(SCIP* scip, Instance& instance, vector<vector<SCIP_VAR*>>& e_t_q, int t_start, int t_end);

//defined in file "functions_for_cons.cc"
void create_flow_conservation_cons(SCIP* scip, FleetGraph& fleet_graph, SCIP_Real bound, Fleet_Planning_Graph::vertex_iterator vit, std::unordered_set <SCIP_CONS*>& eq_cons);

std::unordered_set <SCIP_CONS*> add_flow_constraints(SCIP* scip, FleetGraph& fleet_graph, Instance& instance);

void create_retrofit_constraints(SCIP* scip, FleetGraph& fleet_graph, vector<Retrofit> retrofits, vector<Aircraft_Properties>& aircrafts, int t, int r);

void add_retrofit_constraints(SCIP* scip, FleetGraph& fleet_graph, Instance& instance, int t_start, int t_end);

void create_benders_constraints(SCIP* scip, Instance& instance, FleetGraph& fleet_graph, int t);

void add_benders_constraints(SCIP* scip, Instance& instance, FleetGraph& fleet_graph);

void create_flight_demand_cons(SCIP* scip, int D_flights, std::unordered_set <SCIP_CONS*>& demand_cons, vector<vector<vector<vector<vector<SCIP_VAR*>>>>>& flight_var_t_l_q_c_a,  int t, int l, int q);

std::unordered_set <SCIP_CONS*> add_demand_constraints(SCIP* scip, Instance& instance, vector<vector<vector<vector<vector<SCIP_VAR*>>>>>& flight_var_t_l_q_c_a, int t_start, int t_end);

void create_supply_constraints(vector<Aircraft_Properties>& aircrafts, vector<Net_Class>& net_classes, vector<SCIP_CONS*>& supply_cons, FleetGraph& fleet_graph, vector<vector<vector<vector<vector<SCIP_VAR*>>>>>& flight_var_t_l_q_c_a, int c, int a, int t);

vector<SCIP_CONS*> add_supply_constraints(SCIP* scip, Instance& instance, FleetGraph& fleet_graph, vector<vector<vector<vector<vector<SCIP_VAR*>>>>>& flight_var_t_l_q_c_a, int t_start, int t_end);

void add_emission_constraints(SCIP* scip, Instance& instance, vector<vector<vector<vector<vector<SCIP_VAR*>>>>>& flight_var_t_l_q_c_a, vector<vector<SCIP_VAR*>>& e_t_q, int t_start, int t_end);

void create_emission_constraints(SCIP* scip, Instance& instance, vector<vector<vector<vector<vector<SCIP_VAR*>>>>>& flight_var_t_l_q_c_a, vector<vector<SCIP_VAR*>>& e_t_q, int q, int t);

double get_AFLAB(double afw, double fh);

double get_AFMAT(double afw, double fh);

double get_ENGLAB(int slst, int nmb_eng, double fh);

double get_ENGMAT(int slst, int nmb_eng, double fh);

double get_costs_cabin(int mtgw, double fh);

double get_costs_crew(int nmb_seats, double fh);

//defined in file "functions_for_decomp.cc"
void create_master_problem(SCIP* scip, FleetGraph& fleet_graph, Instance& instance);

void create_subproblems(SCIP** sub_scip, FleetGraph& fleet_graph, Instance& instance, vector<vector<SCIP_VAR*>>& e_t_q, vector<vector<vector<vector<vector<SCIP_VAR*>>>>>& flight_var_t_l_q_c_a);

void set_benders_parameter_setting(SCIP* scip);

void release_flow_variables(SCIP* scip, FleetGraph& fleet_graph, int t=-1);

void release_flight_variables(SCIP* scip, vector<vector<vector<vector<vector<SCIP_VAR*>>>>>& flight_var_t_l_q_c_a, int t=-1);

void release_emission_variables(SCIP* scip, vector<vector<SCIP_VAR*>>& e_t_q, int t_out=-1);
#endif /* FUNCTIONS_HH */

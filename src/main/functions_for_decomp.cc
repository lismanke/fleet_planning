#include "functions.hh"

//creates the scip Masterproblem for the Benders Decomposition
void create_master_problem(SCIP* scip, FleetGraph& fleet_graph, Instance& instance)
{
  add_flow_variables(scip, fleet_graph, instance);
  add_flow_constraints(scip, fleet_graph, instance);
	add_retrofit_constraints(scip, fleet_graph, instance, 0, instance.get_T_max());
}

//creates the Subproblems for the Benders Decomposition
void create_subproblems(SCIP** sub_scip, FleetGraph& fleet_graph, Instance& instance, vector<vector<SCIP_VAR*>>& e_t_q, vector<vector<vector<vector<vector<SCIP_VAR*>>>>>& flight_var_t_l_q_c_a)
{

  flight_var_t_l_q_c_a.resize(instance.get_T_max());
  e_t_q.resize(instance.get_T_max());

  for(int t=0; t<instance.get_T_max(); t++)
  {
    add_flow_variables(sub_scip[t], fleet_graph, instance, t);
    add_flight_variables(sub_scip[t], instance, flight_var_t_l_q_c_a, t, t+1);
    add_emission_variables(sub_scip[t], instance, e_t_q, t, t+1);
    add_supply_constraints(sub_scip[t], instance, fleet_graph, flight_var_t_l_q_c_a, t, t+1);
    add_demand_constraints(sub_scip[t], instance, flight_var_t_l_q_c_a, t, t+1);
    add_emission_constraints(sub_scip[t], instance, flight_var_t_l_q_c_a, e_t_q, t, t+1);

    //release vars to avoid storing to much of them
    release_flow_variables(sub_scip[t], fleet_graph, t);
  }
}

//set the parameters within scip to be able to use the default Benders Decomposition
void set_benders_parameter_setting(SCIP* scip)
{
  SCIP_CALL_EXC(SCIPsetPresolving(scip, SCIP_PARAMSETTING_OFF, true));

  //activating the Benders' decomposition constraint handlers
  SCIP_CALL_EXC( SCIPsetBoolParam(scip, "constraints/benders/active", TRUE) );
  SCIP_CALL_EXC( SCIPsetBoolParam(scip, "constraints/benderslp/active", TRUE) );
  SCIP_CALL_EXC( SCIPsetIntParam(scip, "constraints/benders/maxprerounds", 1) );
  SCIP_CALL_EXC( SCIPsetIntParam(scip, "presolving/maxrounds", 1) );

  //for Benders' decomposition instances restarts must be disabled
  SCIP_CALL_EXC( SCIPsetIntParam(scip,"presolving/maxrestarts", 0) );

  //activating the trysol heuristic so that solutions checked by the Benders' decomposition subproblems are added to the main SCIP
  SCIP_CALL_EXC( SCIPsetIntParam(scip, "heuristics/trysol/freq", 1) );
}

//function to release the flow variables
void release_flow_variables(SCIP* scip, FleetGraph& fleet_graph, int t)
{
  const Fleet_Planning_Graph& graph = fleet_graph.get_graph();
  Fleet_Planning_Graph::edge_iterator ei, ei_end;
  for(std::tie(ei,ei_end) = edges(graph); ei != ei_end; ++ei)
  {
    if (fleet_graph.get_edge_type(*ei) == 'f') continue;
    if(fleet_graph.get_vertex_time(source(*ei,graph)) != t && t != -1) continue;

    SCIP_RETCODE released = fleet_graph.release_var_at(scip, *ei);
  }
}

//function to release the flight variables
void release_flight_variables(SCIP* scip, vector<vector<vector<vector<vector<SCIP_VAR*>>>>>& flight_var_t_l_q_c_a, int t_out)
{
  for(int t=0;t<flight_var_t_l_q_c_a.size();t++)
  {
    for(int l=0;l<flight_var_t_l_q_c_a[t].size();l++)
    {
      for(int q=0;q<flight_var_t_l_q_c_a[t][l].size();q++)
      {
        for(int c=0;c<flight_var_t_l_q_c_a[t][l][q].size();c++)
        {
          for(int a=0;a<flight_var_t_l_q_c_a[t][l][q][c].size();a++)
          {
            if(t_out != t && t_out != -1) continue;
            SCIP_VAR* t_var = flight_var_t_l_q_c_a[t][l][q][c][a];
            if(&t_var != NULL) SCIPreleaseVar(scip,&t_var);
          }
        }
      }
    }
  }
}

//function to release the emission variables
void release_emission_variables(SCIP* scip, vector<vector<SCIP_VAR*>>& e_t_q, int t_out)
{
  for(int t=0;t<e_t_q.size();t++)
  {
    for(int q=0;q<e_t_q[t].size();q++)
    {
      if(t_out != t && t_out != -1) continue;
      SCIP_VAR* t_var = e_t_q[t][q];
      if(&t_var != NULL) SCIPreleaseVar(scip,&t_var);
    }
  }
}

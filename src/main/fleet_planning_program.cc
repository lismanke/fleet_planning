#define SCIP_DEBUG
#include "fleet_planning_program.hh"

using namespace boost;

//creates a the original formulation of the fleet planning problem
OriginalProgram::OriginalProgram(FleetGraph& fleet_graph, Instance& instance) : fleet_graph(fleet_graph)
{
	scip = create_basic_scip_program("flow");
	add_flow_variables(scip, fleet_graph, instance);
	flight_var_t_l_q_c_a.resize(instance.get_T_max());
	add_flight_variables(scip, instance, flight_var_t_l_q_c_a, 0, instance.get_T_max());
	e_t_q.resize(instance.get_T_max());
	add_emission_variables(scip, instance, e_t_q, 0, instance.get_T_max());
	add_flow_constraints(scip, fleet_graph, instance);
	add_retrofit_constraints(scip, fleet_graph, instance, 0, instance.get_T_max());
	add_supply_constraints(scip, instance, fleet_graph, flight_var_t_l_q_c_a, 0, flight_var_t_l_q_c_a.size());
	add_demand_constraints(scip, instance, flight_var_t_l_q_c_a, 0, flight_var_t_l_q_c_a.size());
	add_emission_constraints(scip, instance, flight_var_t_l_q_c_a, e_t_q, 0, flight_var_t_l_q_c_a.size());
}

OriginalProgram::~OriginalProgram()
{
	if(scip != NULL)
	{
		SCIPfreeTransform(scip);

		release_flow_variables(scip, fleet_graph);
		release_flight_variables(scip, flight_var_t_l_q_c_a);
		release_emission_variables(scip, e_t_q);

		SCIPfree(&scip);
	}

}

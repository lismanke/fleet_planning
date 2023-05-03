#include "benders.hh"

//creates benders decomposition formulation of the fleet planning problem
BendersDecomp::BendersDecomp(FleetGraph& fleet_graph, Instance& instance) : fleet_graph(fleet_graph), T_max(instance.get_T_max())
{
	scip = create_basic_scip_program("master");
	
  //allocate memory for subproblems
  SCIP_CALL_EXC(SCIPallocBufferArray(scip, &sub_scip, instance.get_T_max()));
  for(int t=0; t<instance.get_T_max(); t++)
  {
    sub_scip[t] = create_basic_scip_program("sub");
  }

  create_subproblems(sub_scip, fleet_graph, instance, e_t_q, flight_var_t_l_q_c_a);
	create_master_problem(scip, fleet_graph, instance);

	set_benders_parameter_setting(scip);

  SCIP_CALL_EXC( SCIPcreateBendersDefault(scip, sub_scip, instance.get_T_max()) );
}

//releases all remaining variables and the scip program
BendersDecomp::~BendersDecomp()
{
	if(scip != NULL)
	{
		SCIPfreeTransform(scip);
		release_flow_variables(scip, fleet_graph);

		for(int t=0; t<T_max; t++)
		{
			release_emission_variables(sub_scip[t], e_t_q, t);
			release_flight_variables(sub_scip[t], flight_var_t_l_q_c_a, t);
			SCIPfree(&sub_scip[t]);
		}
		SCIPfree(&scip);	
	}
}

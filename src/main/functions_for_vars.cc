#include "functions.hh"
double INFLATION = 0.02;

//creates the basic scip program
SCIP* create_basic_scip_program(const std::string& name)
{
	SCIP* scip;
	SCIP_CALL_EXC(SCIPcreate(&scip));
	SCIP_CALL_EXC(SCIPincludeDefaultPlugins(scip));
	SCIP_CALL_EXC(SCIPcreateProbBasic(scip, name.c_str()));
	return(scip);
}

//starts the solving process of the previously set up scip program
void solve_scip_problem(SCIP* scip, int max_solving_time)
{
	SCIP_CALL_EXC(SCIPsetRealParam(scip, "limits/time", max_solving_time));
	SCIP_CALL_EXC( SCIPsetIntParam(scip, "display/verblevel", 5) );
  //for a solving attempt wizh limited memory capacity
	// SCIP_CALL_EXC(SCIPsetRealParam(scip, "memory/savefac", 0.5));
	// SCIP_CALL_EXC(SCIPsetRealParam(scip, "limits/memory", 7300));
	SCIPsetMessagehdlrLogfile(scip, (Path + std::string("/log/") + "log_" + string(SCIPgetProbName(scip)) + ".txt" ).c_str());
	SCIP_CALL_EXC( SCIPwriteParams(scip, (Path + std::string("/log/") + "parameter_" + string(SCIPgetProbName(scip)) + ".txt" ).c_str(), FALSE, TRUE) );

	SCIP_CALL_EXC(SCIPsolve(scip));
}

//creates a continous variable inside the scip program
SCIP_VAR* scip_create_var_continous(SCIP* scip, std::ostringstream &namebuf, double lb, double ub, double obj, bool removable)
{
  SCIP_VAR* var;
  SCIP_CALL_EXC(SCIPcreateVar(scip,
                              &var,
                              namebuf.str().c_str(), 			                      // name of var
                              lb,                     												  // lower
                              ub,									   													  // upper
                              obj,                													   	// objective value
                              SCIP_VARTYPE_CONTINUOUS,											   	// vartype
                              TRUE,                   											   	// initial
                              removable,             											   		// removable
                              NULL, NULL, NULL, NULL, NULL));
  SCIP_CALL_EXC(SCIPaddVar(scip, var));
  return(var);
}

//creates an integer variable inside the scip program
SCIP_VAR* scip_create_var_int(SCIP* scip, std::ostringstream &namebuf, double lb, double ub, double obj, bool removable)
{
  SCIP_VAR* var;
  SCIP_CALL_EXC(SCIPcreateVar(scip,
                              &var,
                              namebuf.str().c_str(), 			                      // name of var
                              lb,                     												  // lower
                              ub,   																					  // upper
                              obj,               														   	// objective value
                              SCIP_VARTYPE_INTEGER,   											   	// vartype
                              TRUE,                   											   	// initial
                              removable,             											   		// removable
                              NULL, NULL, NULL, NULL, NULL));
  SCIP_CALL_EXC(SCIPaddVar(scip, var));
  return(var);
}

//define the name of the flow variables
ostringstream define_namebuf_flow_variables(Fleet_Planning_Graph::edge_iterator ei, FleetGraph& fleet_graph, const Fleet_Planning_Graph& graph,  string problem)
{
  std::ostringstream namebuf;

  namebuf << problem;
  namebuf << fleet_graph.get_edge_type(*ei);
  // namebuf << "_edge_";//<<source(*ei,graph)<<"_to_"<<target(*ei,graph);
  if(fleet_graph.get_edge_type(*ei) == 's')
  {
    namebuf << "_c(" << fleet_graph.get_vertex_class(source(*ei,graph)) << ")"
            << "_a(" << fleet_graph.get_vertex_age(source(*ei,graph)) << ")"
            << "_t(" << fleet_graph.get_vertex_time(source(*ei,graph)) << ")";
  }
  else if(fleet_graph.get_edge_type(*ei) == 'p')
  {
    namebuf << "_c(" << fleet_graph.get_vertex_class(target(*ei,graph)) << ")"
            << "_t("<< fleet_graph.get_vertex_time(target(*ei,graph)) << ")";
  }
  else
  {
    namebuf << "_c(" << fleet_graph.get_vertex_class(source(*ei,graph)) << ")"
            << "_a(" << fleet_graph.get_vertex_age(source(*ei,graph)) << ")"
            << "_t(" << fleet_graph.get_vertex_time(source(*ei,graph)) << ")"
            << "_to"
            << "_c(" << fleet_graph.get_vertex_class(target(*ei,graph)) << ")"
            << "_a(" << fleet_graph.get_vertex_age(target(*ei,graph)) << ")"
            << "_t(" << fleet_graph.get_vertex_time(target(*ei,graph)) << ")";
  }
  return(namebuf);
}

//compute upper bound and objective value for flow variables
pair<double, double> compute_ub_and_obj_value_flow_variables(SCIP* scip, Fleet_Planning_Graph::edge_iterator ei, FleetGraph& fleet_graph, Instance& instance, const Fleet_Planning_Graph& graph)
{
  double ub  = 0.;//SCIPinfinity(scip);
  double obj = 0.;
	int time = fleet_graph.get_vertex_time(source(*ei,graph))-fleet_graph.get_vertex_age(source(*ei,graph));

  //consider purchase (and retrofit limits)
  if(fleet_graph.get_edge_type(*ei) == 'p')
  {
    ub  = instance.get_market().get_nmb_purchasable_aircrafts_of_at(fleet_graph.get_vertex_class(target(*ei,graph)), fleet_graph.get_vertex_time(target(*ei,graph))-1);
    obj = instance.get_market().get_purchase_price_for_at(fleet_graph.get_vertex_class(target(*ei,graph)), fleet_graph.get_vertex_time(target(*ei,graph))-1);// * pow(1.+INFLATION,-fleet_graph.get_vertex_time(source(*ei,graph))+1);
    obj = round(obj*pow(10,4))*pow(10,-4);                                      //round to 4 decimal places
  }
  else if(fleet_graph.get_edge_type(*ei) == 'r')
  {
    obj      = fleet_graph.get_edge_cost(*ei) * pow(1.+INFLATION, fleet_graph.get_vertex_time(source(*ei,graph)));
    obj      = round(obj*pow(10,4))*pow(10,-4);                                 //round to 4 decimal places
    int c    = fleet_graph.get_vertex_class(source(*ei,graph));
    //upper bound is a constraint over all retroffited types made of a retrofit
    if(time>0)
    {
      ub = instance.get_market().get_nmb_purchasable_aircrafts_of_at(c, time);
    }
    else if(instance.get_aircrafts()[c].get_nmb_owned_at_start().size()>0)
    {
      ub = instance.get_aircrafts()[c].get_nmb_owned_at_start()[-time-1];
    }
  }
  else if(fleet_graph.get_edge_type(*ei)=='s')
  {			                                                                        //price decreases by time
    obj   = -instance.get_market().get_retail_price_for_at_at(fleet_graph.get_vertex_class(source(*ei,graph)), fleet_graph.get_vertex_time(source(*ei,graph)), fleet_graph.get_vertex_age(source(*ei,graph)));
    obj   = round(obj*pow(10,4))*pow(10,-4);                                    //round to 4 decimal places
    int c = fleet_graph.get_vertex_class(source(*ei,graph));
    if(instance.get_aircrafts()[c].is_retrofitted())
    {
      int origin = instance.get_aircrafts()[c].get_origin();
      if(time > 0)
      {
        ub = instance.get_market().get_nmb_purchasable_aircrafts_of_at(origin, time);
      }
      else if(instance.get_aircrafts()[origin].get_nmb_owned_at_start().size()>0)
      {
        ub = instance.get_aircrafts()[origin].get_nmb_owned_at_start()[-time];
      }
    }
    else
    {
      if(time>0)
      {
        ub = instance.get_market().get_nmb_purchasable_aircrafts_of_at(c, time);
      }
      else if(instance.get_aircrafts()[c].get_nmb_owned_at_start().size()>0)
      {
        ub = instance.get_aircrafts()[c].get_nmb_owned_at_start()[-time];
      }
    }
  }
  else if(fleet_graph.get_edge_type(*ei) == 'o')
  {
    int c = fleet_graph.get_vertex_class(source(*ei,graph));
    if(instance.get_aircrafts()[c].is_retrofitted())
    {
      int origin = instance.get_aircrafts()[c].get_origin();
      if(time>0)
      {
        ub = instance.get_market().get_nmb_purchasable_aircrafts_of_at(origin, time);
      }
      else if(instance.get_aircrafts()[origin].get_nmb_owned_at_start().size()>0)
      {
        ub = instance.get_aircrafts()[origin].get_nmb_owned_at_start()[-time];
      }
    }
    else
    {
      if(time>0)
      {
        ub = instance.get_market().get_nmb_purchasable_aircrafts_of_at(c, time);
      }
      else if(instance.get_aircrafts()[c].get_nmb_owned_at_start().size()>0)
      {
        ub = instance.get_aircrafts()[c].get_nmb_owned_at_start()[-time];
      }
    }
  }
	if(string(SCIPgetProbName(scip)) == "sub")	obj = 0;
  pair<double, double> r = {ub, obj};
  return(r);
}

//add flow variables
void add_flow_variables(SCIP* scip, FleetGraph& fleet_graph, Instance& instance, int t)
{
  const Fleet_Planning_Graph& graph = fleet_graph.get_graph();
	Fleet_Planning_Graph::edge_iterator ei, ei_end;

  ofstream pFile((Path + std::string("/variables/flow_vars_" + string(SCIPgetProbName(scip)) + to_string(t+1) + ".txt").c_str()));
  pFile << "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t|\tobjective function value\t|\tupper bound of variable\n";
	for(std::tie(ei,ei_end) = edges(graph); ei != ei_end; ++ei)
	{
    if(fleet_graph.get_edge_type(*ei) == 'f') continue;
		if(fleet_graph.get_vertex_time(source(*ei,graph)) != t && t != -1) continue;

		std::ostringstream namebuf  = define_namebuf_flow_variables(ei, fleet_graph, graph);
    pair<double, double> ub_obj = compute_ub_and_obj_value_flow_variables(scip, ei, fleet_graph, instance, graph);
    SCIP_VAR* var               = scip_create_var_int(scip, namebuf, 0., ub_obj.first, ub_obj.second, TRUE);    //convert to M$

		fleet_graph.var_to_edge(var, *ei);

    pFile << setw(40) << namebuf.str() << "\t|\t\t\t\t\t" << setw(10) << to_string(ub_obj.second) << "\t\t\t\t|\t\t\t\t\t" << setw(5) << to_string(ub_obj.first) << "\n";
  }
  pFile.close();
}

//define the name of the flight variables
ostringstream define_namebuf_flight_variables(int t, int l, int c, int a, int q)
{
  std::ostringstream namebuf;

  namebuf << "z_c(" << c << ")"
          << "_a(" << a << ")"
          << "_t(" << t+1 << ")"
          << "_l(" << l << ")"
					<< "_" << name[q];
  return(namebuf);
}

//compute objective value for flight variables
double compute_obj_value_flight_variables(int t, int l, int c, int a, vector<Net_Class>& net_classes, vector<Aircraft_Properties>& aircrafts, vector<int> usable_aircrafts, Market_Situation& market)
{
  TechnicalDetails& tech_data=aircrafts[usable_aircrafts[c]].get_tech_data();

  double CC = get_costs_cabin(tech_data.mtgw, net_classes[l].get_blocked_times()[c]/60);
  CC       += get_costs_crew(aircrafts[usable_aircrafts[c]].get_nmb_seats(), net_classes[l].get_blocked_times()[c]/60);

  double MC = 3*get_AFLAB(tech_data.afw, net_classes[l].get_blocked_times()[c]/60);
  MC       += get_AFMAT(tech_data.afw, net_classes[l].get_blocked_times()[c]/60);
  MC       += 3*get_ENGLAB(tech_data.slst,tech_data.nmb_eng, net_classes[l].get_blocked_times()[c]/60);
  MC       += get_ENGMAT(tech_data.slst,tech_data.nmb_eng, net_classes[l].get_blocked_times()[c]/60);

  double afm = pow(1.02, 23+t);
  if(a<7)
  {
    afm *= 0.12*a+0.18;
  }
  else if(a<14)
  {
    afm *= 1;
  }
  else
  {
    afm *= 0.037*a+0.49;
  }

  double obj = net_classes[l].get_fuel_uses()[c] * market.get_fuel_price_at(t) + CC * pow(1.02,23+t) + MC * afm;
  obj       *= pow(1.+INFLATION, -t-1);
  obj        = round(obj)*pow(10,-6);                                           //convert to M$

	return(obj);
}

//add flight variables
void add_flight_variables(SCIP* scip, Instance& instance, vector<vector<vector<vector<vector<SCIP_VAR*>>>>>& flight_var_t_l_q_c_a, int t_start, int t_end)
{
	vector<Net_Class>& net_classes         = instance.get_net_classes();
	vector<Aircraft_Properties>& aircrafts = instance.get_aircrafts();

  ofstream pFile((Path + std::string("/variables/flight_vars_" + string(SCIPgetProbName(scip)) + to_string(t_end) + ".txt").c_str()));
  pFile << "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t|\tobjective function value\t|\tupper bound of variable\n";
	for(int t = t_start; t < t_end; t++)
	{
		flight_var_t_l_q_c_a[t].resize(net_classes.size());
		for(int l = 0;l<net_classes.size();l++)
		{
			flight_var_t_l_q_c_a[t][l].resize(2);
			vector<int> usable_aircrafts = net_classes[l].get_usable_aircrafts();
			for(int q=0; q<2; q++)
			{
				flight_var_t_l_q_c_a[t][l][q].resize(usable_aircrafts.size());
				for(int c=0;c<usable_aircrafts.size();c++)
				{
					//skip retrofitted aircrafts at t=0 				& retrofitted types if retrofit is not available jet 																				& types which are not available jet
					if((aircrafts[usable_aircrafts[c]].is_retrofitted() && t<1) || (aircrafts[usable_aircrafts[c]].is_retrofitted() && t <= aircrafts[usable_aircrafts[c]].get_retro_purchasable_since_period()) || (t <= aircrafts[usable_aircrafts[c]].get_purchasable_since_period()))	continue;

					//create one var for each possible age of aircraft type (idea is to treat it like an own aircraft type)
					int service_t = aircrafts[usable_aircrafts[c]].get_service_t()+1;
					if(service_t > t-aircrafts[usable_aircrafts[c]].get_purchasable_since_period()) service_t = t-aircrafts[usable_aircrafts[c]].get_purchasable_since_period();
					for(int a=0;a<service_t;a++)
					{
						//skip types at service_t if available during optimization process
						if(aircrafts[usable_aircrafts[c]].get_purchasable_since_period() > 0 && a == service_t) continue;
						//skip retrofitted aircrafts at age=0
						if(aircrafts[usable_aircrafts[c]].is_retrofitted() && a==0)	continue;

						ostringstream namebuf = define_namebuf_flight_variables(t, l, usable_aircrafts[c], a, q);
						double obj            = compute_obj_value_flight_variables(t, l, c, a, net_classes, aircrafts, usable_aircrafts, instance.get_market());
						SCIP_VAR* var         = scip_create_var_continous(scip, namebuf, 0., net_classes[l].get_D_flights_in_as_at_t(q,t), obj, TRUE);

						if(net_classes[l].get_D_flights_in_as_at_t(q,t) == 0) obj = 0;

						flight_var_t_l_q_c_a[t][l][q][c].push_back(var);

						pFile << setw(35) << namebuf.str() << "\t|\t\t\t\t\t" << setw(10) << to_string(obj) << "\t\t\t\t|\t\t\t\t\t" << setw(5) << to_string(instance.get_net_classes()[l].get_D_flights_in_as_at_t(q,t)) << "\n";
					}
				}
			}
		}
	}
  pFile.close();
}

//add variables regarding emession
void add_emission_variables(SCIP* scip, Instance& instance, vector<vector<SCIP_VAR*>>& e_t_q, int t_start, int t_end)
{
  ofstream pFile((Path + std::string("/variables/emission_vars_" + string(SCIPgetProbName(scip)) + to_string(t_end) + ".txt").c_str()));
  pFile << "\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t|\tobjective function value\t|\tupper bound of variable\n";
	for(int t=t_start; t<t_end; t++)
	{
		for(int q=0; q<2; q++)
		{
			std::ostringstream namebuf;
			namebuf << "e_" << name[q] << "_t(" << t+1 << ")";
			double obj = instance.get_market().get_emission_price_in_as_at(q, t);
	    SCIP_VAR* var = scip_create_var_continous(scip, namebuf, 0., SCIPinfinity(scip), obj, TRUE);

			e_t_q[t].push_back(var);

	    pFile << setw(35) << namebuf.str() << "\t|\t\t\t\t\t" << setw(10) << to_string(instance.get_market().get_emission_price_in_as_at(q, t)) << "\t\t\t\t|\t\t\t\t\t" << setw(5) << to_string(SCIPinfinity(scip)) << "\n";
		}
	}
  pFile.close();
}

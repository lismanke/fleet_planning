#define SCIP_DEBUG
#include "functions.hh"

//create flow conservation constraint
void create_flow_conservation_cons(SCIP* scip, FleetGraph& fleet_graph, SCIP_Real bound, Fleet_Planning_Graph::vertex_iterator vit, std::unordered_set <SCIP_CONS*>& eq_cons)
{
  const Fleet_Planning_Graph& graph = fleet_graph.get_graph();
  SCIP_CONS* flow_cons;
  std::ostringstream namebuf;
  namebuf << "flow_cons_c(" << fleet_graph.get_vertex_class(*vit) << ")_a(" << fleet_graph.get_vertex_age(*vit) << ")_t(" << fleet_graph.get_vertex_time(*vit) << ")";

  SCIP_CALL_EXC(SCIPcreateConsLinear(scip,
                                     &flow_cons,
                                     namebuf.str().c_str(),
                                     0,
                                     NULL,
                                     NULL,
                                     -bound, 			 			 									      // lhs
                                     -bound,              							       	// rhs
                                     TRUE,               	      								// initial
                                     TRUE,           						       					// separate
                                     TRUE,                 											// enforce
                                     TRUE,                 											// check
                                     TRUE,          			       								// propagate
                                     FALSE,         						      					// local
                                     FALSE,         									       		// modifiable
                                     FALSE,                											// dynamic
                                     FALSE,                											// removable
                                     FALSE));        			      								// sticking at node

  Fleet_Planning_Graph::out_edge_iterator ei, ei_end;
  for(std::tie(ei,ei_end) = boost::out_edges(*vit, graph); ei != ei_end; ++ei)
  {
    if(fleet_graph.get_edge_type(*ei) != 'f')
    {
      SCIPaddCoefLinear(scip, flow_cons, fleet_graph.get_var_of_edge(*ei), -1.);
    }
  }

  Fleet_Planning_Graph::in_edge_iterator iei, iei_end;
  for(std::tie(iei,iei_end) = boost::in_edges(*vit, graph); iei != iei_end; ++iei)
  {
    if(fleet_graph.get_edge_type(*iei) != 'f')
    {
      SCIPaddCoefLinear(scip, flow_cons, fleet_graph.get_var_of_edge(*iei), 1.);
    }
  }

  SCIP_Bool works = true;
  int n;
  SCIPgetConsNVars(scip, flow_cons, &n, &works);
  if(n > 0)
  {
    SCIP_CALL_EXC(SCIPaddCons(scip, flow_cons));
    eq_cons.insert(flow_cons);
    fleet_graph.flow_cons_to_vertex(flow_cons, *vit);
  }
  SCIPreleaseCons(scip, &flow_cons);
}

//add flow conservation constraints
std::unordered_set <SCIP_CONS*> add_flow_constraints(SCIP* scip, FleetGraph& fleet_graph, Instance& instance)
{
  vector<Aircraft_Properties>& aircrafts = instance.get_aircrafts();
	const Fleet_Planning_Graph& graph      = fleet_graph.get_graph();
  std::unordered_set <SCIP_CONS*> eq_cons;
	Fleet_Planning_Graph::vertex_iterator vit, vend;

	for(boost::tie(vit,vend) = boost::vertices(graph); vit!=vend; vit++)
	{
		if(*vit != fleet_graph.get_p_market_node() && *vit != fleet_graph.get_r_market_node())
		{
      if(fleet_graph.get_vertex_time(*vit) == instance.get_T_max() && fleet_graph.get_vertex_age(*vit) < aircrafts[fleet_graph.get_vertex_class(*vit)].get_service_t())
      {
        continue;
      }
			else if(fleet_graph.get_vertex_time(*vit) == 0)
			{
				SCIP_Real bound = 0.;
				if(aircrafts[fleet_graph.get_vertex_class(*vit)].get_nmb_owned_at_start().size()>0)
				{                                                                       //age in vector nmb_owned_at_start is in period t=0(before optimization starts) and age 1 is in place 1
					bound = aircrafts[fleet_graph.get_vertex_class(*vit)].get_nmb_owned_at_start()[fleet_graph.get_vertex_age(*vit)];
				}
        create_flow_conservation_cons(scip, fleet_graph, bound, vit, eq_cons);
			}
			else
			{
        create_flow_conservation_cons(scip, fleet_graph, 0., vit, eq_cons);
			}
		}
	}
  return(eq_cons);
}

//creates the upper bounds on the number of retrofits doable per period
void create_retrofit_constraints(SCIP* scip, FleetGraph& fleet_graph, vector<Retrofit> retrofits, vector<Aircraft_Properties>& aircrafts, int t, int r)
{
  const Fleet_Planning_Graph& graph      = fleet_graph.get_graph();

  SCIP_CONS* r_cons;
  std::ostringstream namebuf;
  namebuf << "r_cons_t(" << t+1 << ")_r(" << r << ")";

  SCIP_CALL_EXC(SCIPcreateConsLinear( scip,
                                      &r_cons,
                                      namebuf.str().c_str(),
                                      0,
                                      NULL,
                                      NULL,
                                      -SCIPinfinity(scip),                      // lhs
                                      retrofits[r].get_max_per_t(),      	  	  // rhs
                                      TRUE,                                     // initial
                                      TRUE,                                     // separate
                                      TRUE,                                     // enforce
                                      TRUE,                                     // check
                                      TRUE,                                     // propagate
                                      FALSE,                                    // local
                                      FALSE,                                    // modifiable
                                      FALSE,                                    // dynamic
                                      FALSE,                                    // removable
                                      FALSE));                                  // sticking at node

  for(int rc=0; rc<retrofits[r].get_candidates().size(); rc++)
  {
    int service_t = aircrafts[rc].get_service_t()+1;
    if(service_t > t-aircrafts[rc].get_purchasable_since_period()) service_t = t-aircrafts[rc].get_purchasable_since_period();
    for(int a=0;a<service_t;a++)
    {
      //skip types at service_t if available during optimization process
      if(aircrafts[rc].get_purchasable_since_period() > 0 && a == service_t) continue;
      //skip retrofitted aircrafts at age=0
      if(aircrafts[rc].is_retrofitted() && a==0)	continue;

      Fleet_Planning_Graph::in_edge_iterator iei, iei_end;
      for(std::tie(iei,iei_end) = boost::in_edges(fleet_graph.get_node(t+1,retrofits[r].get_candidates()[rc].type,a), graph); iei != iei_end; ++iei)
      {
        if(fleet_graph.get_edge_type(*iei) == 'r')
        {
          SCIPaddCoefLinear(scip, r_cons, fleet_graph.get_var_of_edge(*iei), 1.);
        }
      }
    }
  }
  SCIP_CALL_EXC(SCIPaddCons(scip, r_cons));
  SCIPreleaseCons(scip, &r_cons);
}

//add a strict boundary for the number of possible retrofits per period
void add_retrofit_constraints(SCIP* scip, FleetGraph& fleet_graph, Instance& instance, int t_start, int t_end)
{
  vector<Retrofit> retrofits = instance.get_market().get_retrofits();
  for(int t=t_start; t<t_end; t++)
  {
    for(int r=0; r<retrofits.size(); r++)
    {
      if(retrofits[r].get_candidates().size() == 0) continue;
      create_retrofit_constraints(scip, fleet_graph, retrofits, instance.get_aircrafts(), t, r);
    }
  }
}

//create constraints regarding the flight demand 
void create_flight_demand_cons(SCIP* scip, int D_flights, std::unordered_set <SCIP_CONS*>& demand_cons, vector<vector<vector<vector<vector<SCIP_VAR*>>>>>& flight_var_t_l_q_c_a,  int t, int l, int q)
{
  SCIP_CONS* d_cons;
  std::ostringstream namebuf;
  namebuf << "fd_cons_t(" << t+1 << ")_l(" << l << ")" << "_" << name[q];

  SCIP_CALL_EXC(SCIPcreateConsLinear( scip,
                                      &d_cons,
                                      namebuf.str().c_str(),
                                      0,
                                      NULL,
                                      NULL,
                                      D_flights,      	  	                    // lhs
                                      SCIPinfinity(scip),                       // rhs
                                      TRUE,                                     // initial
                                      TRUE,                                     // separate
                                      TRUE,                                     // enforce
                                      TRUE,                                     // check
                                      TRUE,                                     // propagate
                                      FALSE,                                    // local
                                      FALSE,                                    // modifiable
                                      FALSE,                                    // dynamic
                                      FALSE,                                    // removable
                                      FALSE));                                  // sticking at node


  for(int c=0; c<(int)flight_var_t_l_q_c_a[t][l][q].size(); c++)
  {
    for(int a=0; a<(int)flight_var_t_l_q_c_a[t][l][q][c].size(); a++)
    {
      SCIPaddCoefLinear(scip, d_cons, flight_var_t_l_q_c_a[t][l][q][c][a], 1.);
    }
  }
  SCIP_CALL_EXC(SCIPaddCons(scip, d_cons));
  demand_cons.insert(d_cons);
  SCIPreleaseCons(scip, &d_cons);
}

//add constraints for flight demand 
std::unordered_set <SCIP_CONS*> add_demand_constraints(SCIP* scip, Instance& instance, vector<vector<vector<vector<vector<SCIP_VAR*>>>>>& flight_var_t_l_q_c_a, int t_start, int t_end)
{
  std::unordered_set <SCIP_CONS*> demand_cons;
  for(int t=t_start; t<t_end; t++)
  {
    for(int l=0; l<flight_var_t_l_q_c_a[t].size();l++)
  	{
      for(int q=0; q<flight_var_t_l_q_c_a[t][l].size(); q++)
      {
        if (instance.get_net_classes()[l].get_D_flights_in_as_at_t(q,t) != 0)
          create_flight_demand_cons(scip, instance.get_net_classes()[l].get_D_flights_in_as_at_t(q,t), demand_cons, flight_var_t_l_q_c_a, t, l, q);
  		}
    }
	}
  return(demand_cons);
}

//create the constraints for the supply
void create_supply_constraints(SCIP* scip, vector<Aircraft_Properties>& aircrafts, vector<Net_Class>& net_classes, vector<SCIP_CONS*>& supply_cons, FleetGraph& fleet_graph, vector<vector<vector<vector<vector<SCIP_VAR*>>>>>& flight_var_t_l_q_c_a, int c, int a, int t)
{
  SCIP_CONS* s_cons;
  std::ostringstream namebuf;
  namebuf << "s_cons_c(" << c << ")_a(" << a << ")_t(" << t+1 << ")";

  SCIP_CALL_EXC(SCIPcreateConsLinear( scip,
                                      &s_cons,
                                      namebuf.str().c_str(),
                                      0,
                                      NULL,
                                      NULL,
                                      -SCIPinfinity(scip),                      // lhs
                                      0.,                                       // rhs
                                      TRUE,                                     // initial
                                      TRUE,                                     // separate
                                      TRUE,                                     // enforce
                                      TRUE,                                     // check
                                      TRUE,                                     // propagate
                                      FALSE,                                    // local
                                      FALSE,                                    // modifiable
                                      FALSE,                                    // dynamic
                                      FALSE,                                    // removable
                                      FALSE));                                  // sticking at node

  //add vars to incoming edges raising number of owned aircrafts of type c and age a at t
  Fleet_Planning_Graph::out_edge_iterator ei, ei_end;
  for(std::tie(ei,ei_end) = boost::out_edges(fleet_graph.get_node(t,c,a), fleet_graph.get_graph()); ei != ei_end; ++ei)
  {
    if(fleet_graph.get_edge_type(*ei) == 'o' || fleet_graph.get_edge_type(*ei) == 'r')
    {
      double max_flight_hours = -floor(aircrafts[c].get_hours_p_a_at(a)*pow(10,4))*pow(10,-4);
      SCIPaddCoefLinear(scip, s_cons, fleet_graph.get_var_of_edge(*ei), max_flight_hours);
    }
  }

  for(int l = 0;l<net_classes.size();l++)
  {
    vector<int> usable_aircrafts = net_classes[l].get_usable_aircrafts();
    for(int q=0; q<2; q++)
    {
      for(int c_ = 0;c_<usable_aircrafts.size();c_++)
      {
        if(c==usable_aircrafts[c_])
        {
          double coef = net_classes[l].get_blocked_times()[c_]/60;
          coef        = round(coef*pow(10,4))*pow(10,-4);                       //round to 4 decimal places
          SCIPaddCoefLinear(scip, s_cons, flight_var_t_l_q_c_a[t][l][q][c_][a], coef);
          break;
        }
      }
    }
  }
  SCIP_CALL_EXC(SCIPaddCons(scip, s_cons));
  supply_cons.push_back(s_cons);
  SCIPreleaseCons(scip,&s_cons);
}

//add constraints for the supply
vector<SCIP_CONS*> add_supply_constraints(SCIP* scip, Instance& instance, FleetGraph& fleet_graph, vector<vector<vector<vector<vector<SCIP_VAR*>>>>>& flight_var_t_l_q_c_a, int t_start, int t_end)
{
  vector<Aircraft_Properties>& aircrafts = instance.get_aircrafts();
	vector<Net_Class>& net_classes         = instance.get_net_classes();
  vector<SCIP_CONS*> supply_cons;

	//supply flight hours
  for(int t=t_start; t<t_end; t++)
	{
    for(int c=0;c<aircrafts.size();c++)
		{
			//skip retrofitted aircrafts at t=0 				& retrofitted types if retrofit is not available jet 																				& types which are not available jet
			if((aircrafts[c].is_retrofitted() && t<1) || (aircrafts[c].is_retrofitted() && t <= aircrafts[c].get_retro_purchasable_since_period()) || t <= aircrafts[c].get_purchasable_since_period())	continue;
			//create one node for each possible age of aircraft type (idea is to treat it like an own aircraft type)
			int service_t = aircrafts[c].get_service_t()+1;
			if(service_t > t-aircrafts[c].get_purchasable_since_period()) service_t = t-aircrafts[c].get_purchasable_since_period();
      int age = 0;
			for(int a=0;a<service_t;a++)
			{
				//skip types at service_t if available during optimization process
				if(aircrafts[c].get_purchasable_since_period() > 0 && a == service_t) continue;
				//skip retrofitted aircrafts at age=0
				if(aircrafts[c].is_retrofitted() && a==0)	continue;

        create_supply_constraints(scip, aircrafts, net_classes, supply_cons, fleet_graph, flight_var_t_l_q_c_a, c, age, t);
        age++;
			}
		}
	}
  return(supply_cons);
}

//create the constraints "saving" the amount of emitted CO_2 within a period
void create_emission_constraints(SCIP* scip, Instance& instance, vector<vector<vector<vector<vector<SCIP_VAR*>>>>>& flight_var_t_l_q_c_a, vector<vector<SCIP_VAR*>>& e_t_q, int q, int t)
{
  SCIP_CONS* e_cons;
  std::ostringstream namebuf;
  namebuf << "e_cons_" << name[q] << "_t(" << t+1 << ")";

  SCIP_CALL_EXC(SCIPcreateConsLinear( scip,
                                      &e_cons,
                                      namebuf.str().c_str(),
                                      0,
                                      NULL,
                                      NULL,
                                      0.,                                       // lhs
                                      0.,                                       // rhs
                                      TRUE,                                     // initial
                                      TRUE,                                     // separate
                                      TRUE,                                     // enforce
                                      TRUE,                                     // check
                                      TRUE,                                     // propagate
                                      FALSE,                                    // local
                                      FALSE,                                    // modifiable
                                      FALSE,                                    // dynamic
                                      FALSE,                                    // removable
                                      FALSE));                                  // sticking at node

  SCIPaddCoefLinear(scip, e_cons, e_t_q[t][q], -1.);

  for(int l=0; l<flight_var_t_l_q_c_a[t].size(); l++)
  {
    for(int c = 0;c<flight_var_t_l_q_c_a[t][l][q].size();c++)
    {
      for(int a = 0;a<flight_var_t_l_q_c_a[t][l][q][c].size();a++)
      {
        double coef = 9.75 * instance.get_market().get_prop_em_compensation()[q] * instance.get_net_classes()[l].get_fuel_uses()[c]; //in kg
        SCIPaddCoefLinear(scip, e_cons, flight_var_t_l_q_c_a[t][l][q][c][a], coef/1000);                                             //in tonne
      }
    }
  }
  SCIP_CALL_EXC(SCIPaddCons(scip, e_cons));
  SCIPreleaseCons(scip,&e_cons);

}

//add the constraints regarding the amount of emitted CO_2 per period
void add_emission_constraints(SCIP* scip, Instance& instance, vector<vector<vector<vector<vector<SCIP_VAR*>>>>>& flight_var_t_l_q_c_a, vector<vector<SCIP_VAR*>>& e_t_q, int t_start, int t_end)
{
  for(int t=t_start; t<t_end; t++)
  {
    for(int q=0; q<2; q++)
    {
      create_emission_constraints(scip, instance, flight_var_t_l_q_c_a, e_t_q, q, t);
    }
  }
}

//functions to compute constants in the constraints
double get_AFLAB(double afw, double fh)
{
	double mmh_fh = 1.260+(1.774*afw)*pow(10,-5)-0.1071*pow(afw*pow(10,-5),2);
	double mmh_fc = 1.614+(0.7227*afw)*pow(10,-5)+0.1024*pow(afw*pow(10,-5),2);
	double costs  = (mmh_fh*fh+mmh_fc)*25;

	return(costs);
}

double get_AFMAT(double afw, double fh)
{
	double mat_fh = 12.39+(29.80*afw)*pow(10,-5)+0.1806*pow(afw*pow(10,-5),2);
	double mat_fc = 15.20+(97.33*afw)*pow(10,-5)-2.862*pow(afw*pow(10,-5),2);
	double costs  = (mat_fh*fh)+mat_fc;

	return(costs);
}

double get_ENGLAB(int slst, int nmb_eng, double fh)
{
	double eng_lab = (0.645+(0.05*slst)*pow(10,-4))*(0.566+0.434/fh)*fh*nmb_eng;
	double costs   = eng_lab*25;

	return(costs);
}

double get_ENGMAT(int slst, int nmb_eng, double fh)
{
	double eng_mat = (25+(18*slst)*pow(10,-4))*(0.62+(0.38/fh))*fh*nmb_eng;

	return(eng_mat);
}

double get_costs_cabin(int mtgw, double fh)
{
	return((482+0.590*mtgw/1000)*fh);
}

double get_costs_crew(int nmb_seats, double fh)
{
	return(((nmb_seats/30)*78)*fh);
}

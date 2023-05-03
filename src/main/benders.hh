#ifndef BENDERS_HH
#define BENDERS_HH

#include "functions.hh"

#include<vector>
#include<numeric>

#include <scip/scip.h>
#include <scip/scipdefplugins.h>
#include "scip/cons_benders.h"
#include "scip/cons_benderslp.h"
#include "scip/benders_default.h"

#include "scip_exception.hh"
#include "data/data.hh"
#include "graph/fleet_graph.hh"
#include "data/global.hh"

using namespace std;

class BendersDecomp
{
private:

    FleetGraph& fleet_graph;
    int T_max;
    //scip programs
    SCIP* scip;
    SCIP** sub_scip;
    //emission variables
    vector<vector<SCIP_VAR*>> e_t_q;
    //flight variables
    vector<vector<vector<vector<vector<SCIP_VAR*>>>>> flight_var_t_l_q_c_a;

public:
    //creates a the original formulation of the fleet planning problem
    BendersDecomp(FleetGraph& fleet_graph, Instance& instance);

    //Releases all outstanding variables and frees scip afterwards
    ~BendersDecomp();

    //fleet graph with flow variables inserted
    FleetGraph& get_fleet_graph()
    {
      return(fleet_graph);
    }

    int get_number_subproblems()
    {
      return(T_max);
    }

    SCIP_VAR* get_flight_var_t_l_q_c_a(int t, int l, int q, int c, int a)
    {
      return(flight_var_t_l_q_c_a[t][l][q][c][a]);
    }

    SCIP_VAR* get_e_t_q(int q, int t)
    {
      return(e_t_q[q][t]);
    }

    SCIP* get_scip()
    {
        return(scip);
    }

    SCIP* get_sub_scip(int t)
    {
      return(sub_scip[t]);
    }
};
#endif

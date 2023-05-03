#define SCIP_DEBUG
#ifndef FLEET_PLANNING_PROGRAM_HH
#define FLEET_PLANNING_PROGRAM_HH

#include "functions.hh"

#include<vector>
#include<numeric>

#include <scip/scip.h>
#include <scip/scipdefplugins.h>

#include "scip_exception.hh"
#include "data/data.hh"
#include "graph/fleet_graph.hh"
#include "data/global.hh"

using namespace std;

class OriginalProgram
{
private:

    FleetGraph& fleet_graph;
    //scip program
    SCIP *scip;
    //emission variables
    vector<vector<SCIP_VAR*>> e_t_q;
    //flight variables
    vector<vector<vector<vector<vector<SCIP_VAR*>>>>> flight_var_t_l_q_c_a;

public:
    //creates a the original formulation of the fleet planning problem
    OriginalProgram(FleetGraph& fleet_graph, Instance& instance);

    //Releases all outstanding constraints and variables and frees scip afterwards
    ~OriginalProgram();

    //fleet graph with flow variables inserted
    FleetGraph& get_fleet_graph()
    {
      return(fleet_graph);
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
};


#endif // FLEET_PLANNING_PROGRAM_HH

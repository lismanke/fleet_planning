#include "printer.hh"

using namespace std;

//Creates a class to print all informations for every solution in files
Printer::Printer()
{
  //Checking existence of folder
  boost::filesystem::path dir_zero("../output");
  boost::filesystem::path dir_one("../output/" + date_namebuf.str());

  if(!(boost::filesystem::exists(dir_zero)))
  {
    if (boost::filesystem::create_directory(dir_zero))
    {
        std::cout << "folder output successfully created!\n";
    }
  }
  if(!(boost::filesystem::exists(dir_one)))
  {
    if (boost::filesystem::create_directory(dir_one))
    {
        std::cout << "folder " + date_namebuf.str() + " successfully created!\n";
    }
  }
  ofstream single_results, single_results_orig;
  if(is_benders) single_results.open("../output/" + date_namebuf.str() + "/single_results_benders_feasible.csv", ios::app);
  single_results << "#periods\t|\t#aircraft_types\t|\t#net_classes\t|\tround\t|\t\t\t\ttime\t\t\t\t|\t\t\tnodes\t\t\t|\t\titerations\t|\t\tvariables\t\t|\tconstraints\t\t|\t\t\trows\t\t\t|\tprimalbound\t\t|\t\tdualbound\t\t|\t\t\tgap\t\t\n";
  single_results.close();

  single_results_orig.open("../output/" + date_namebuf.str() + "/single_results_orig_feasible.csv", ios::app);
  single_results_orig << "#periods\t|\t#aircraft_types\t|\t#net_classes\t|\tround\t|\t\t\t\ttime\t\t\t\t|\t\t\tnodes\t\t\t|\t\titerations\t|\t\tvariables\t\t|\tconstraints\t\t|\t\t\trows\t\t\t|\tprimalbound\t\t|\t\tdualbound\t\t|\t\t\tgap\t\t\n";
  single_results_orig.close();
}

void Printer::print_stats(SCIP* scip, int t, int c, int n, int d, int round, bool benders, bool is_benders)
{
  int i;
  if(benders) i=1;
  else i=0;

  if(T_stats != t)
  {
    T_stats = t;
    if(is_benders) reset_full_stats(scip, t, c, n, d, round, 1);
    reset_full_stats(scip, t, c, n, d, round, 0);
    print_all_headers(t, 0);
    if(is_benders) print_all_headers(t, 1);
  }

  if(benders) print_full_stats_benders(scip, t, c, n, d, round, false);
  else  print_full_stats_orig(scip, t, c, n, d, round, 0, false);

  if(SCIPgetStatus(scip)==SCIP_STATUS_TIMELIMIT || SCIPgetStatus(scip)==SCIP_STATUS_OPTIMAL)
  {
    print_single_results(scip, t, c, n, round, i, true);
  }

  if(SCIPgetStatus(scip)==SCIP_STATUS_INFEASIBLE || SCIPgetStatus(scip)==SCIP_STATUS_INFORUNBD)
  {
    print_single_results(scip, t, c, n, round, i, false);
  }
}

//prints a statistical summery of the feasible instances solved with the original formulation
void Printer::print_full_stats_orig(SCIP* scip, int t, int c, int n, int d, int round, int i, bool endstatistics)
{
  ofstream stats;
  if(endstatistics)
    stats.open("../output/" + date_namebuf.str() + "/full_stats_orig.txt", ios::app);
  else
    stats.open("../output/" + date_namebuf.str() + "/" + std::to_string(t) + "_periods" + "/full_stats_orig.txt", ios::trunc);
  stats << "#periods\t|\tub_flight_range\t|\t#aircraft_types\t|\t#net_classes\t|\t#feasible_instances\t|\t#feasible_instances_solved\t|\t\tav_time\t\t|\t\tvariance_time\t\t|\t\t\tav_vars\t\t|\tav_constraints\t\t|\tav_#solving_nodes\t\t|\tav_lp_iterations\t\t|\tav_gap\n";

  int m = 0;
  for(int j=0; j<11;j++)
  {
    if(j==3 || j==6 || j==9)  m++;
    if((lb_flight_range[m]<=d && d<=ub_flight_range[m] && lb_aircraft_types[(int)j%3]<=c && c<=ub_aircraft_types[(int)j%3]) && !endstatistics && j<9)  write_stats(scip, i, j);

    if(j!=10)
      stats << "\t" << setw(7) << t << "\t|";
    else
      stats << "\t" << setw(4) << 1 << "-" << setw(2) << t << "\t|";
    stats << "\t\t" << setw(5) << lb_flight_range[m] << "-" << setw(5) << ub_flight_range[m] << "\t\t|"
          << "\t\t\t\t" << setw(2) << lb_aircraft_types[(int)j%3] << "-" << setw(2) << ub_aircraft_types[j%3] << "\t\t\t|"
          << "\t\t\t" << setw(2) << lb_netclasses[m] << "-" << setw(2) << ub_netclasses[m] << "\t\t\t|"
          << "\t" << setw(12) << feasible_instances[i][j] << "\t\t\t\t|"
          << "\t\t\t\t" << setw(12) << feasible_instances_solved[i][j] << "\t\t\t\t\t|";
    if(feasible_instances_solved[i][j] != 0)
    {
      stats << "\t" << setw(10) << average_solving_time[i][j]/feasible_instances_solved[i][j] << "\t|"
            << "\t" << setw(12) << sqrt(variance_solving_time[i][j]/feasible_instances_solved[i][j] - pow(average_solving_time[i][j]/feasible_instances_solved[i][j], 2)) << "\t\t\t|"
            << "\t" << setw(12) << average_number_vars[i][j]/feasible_instances_solved[i][j] << "\t|"
            << "\t" << setw(12) << average_number_cons[i][j]/feasible_instances_solved[i][j] << "\t\t\t|"
            << "\t" << setw(12) << average_solving_nodes[i][j]/feasible_instances_solved[i][j] << "\t\t\t\t|"
            << "\t" << setw(12) << average_lp_iterations[i][j]/feasible_instances_solved[i][j] << "\t\t\t\t|";
      if((feasible_instances[0][j]-feasible_instances_solved[0][j]) != 0)
        stats << "\t" << setw(12) << average_gaps[0][j]/(feasible_instances[0][j]-feasible_instances_solved[0][j]) << "\t\n";
      else
        stats << "\t" << setw(12) << 0 << "\t\n";
    }
    else
    {
      stats << "\t" << setw(10) << 0 << "\t|"
            << "\t" << setw(12) << 0 << "\t\t\t|"
            << "\t" << setw(12) << 0 << "\t|"
            << "\t" << setw(12) << 0 << "\t\t\t|"
            << "\t" << setw(12) << 0 << "\t\t\t\t|"
            << "\t" << setw(12) << 0 << "\t\t\t\t|"
            << "\t" << setw(12) << 0 << "\t\n";
    }
  }
  stats.close();
}

//prints a statistical summery of the feasible instances solved with the benders decomposition together with the solutions of the original formulation
void Printer::print_full_stats_benders(SCIP* scip, int t, int c, int n, int d, int round, bool endstatistic)
{
  ofstream stats;
  if(endstatistic)
    stats.open("../output/" + date_namebuf.str() + "/full_stats_benders.txt", ios::app);
  else
    stats.open("../output/" + date_namebuf.str() + "/" + std::to_string(t) + "_periods" + "/full_stats_benders.txt", ios::trunc);
  stats << "#periods\t|\tub_flight_range\t|\t#aircraft_types\t|\t#net_classes\t||\t#feasible_instances\t||\t#feasible_instances_solved\t||\t\t\t\t\t\t\taverage_solving_time\t\t\t\t\t\t||\t\tstandard_deviation_solving_time\t\t||\t\t\t\taverage_#vars\t\t\t\t||\t\taverage_#constraints\t\t||\taverage_#solving_nodes\t\t||\t\taverage_lp_iterations\t\t||\t\t\t\t\taverage_gap\t\t\t\t||\tav_ab_solgap(orig-benders)\t||\tav_rel_ito_orig_solgap(orig-benders)\n";
  stats << "\t\t\t\t\t|\t\t\t\t\t\t\t\t\t|\t\t\t\t\t\t\t\t\t|\t\t\t\t\t\t\t\t||\t\torig\t\t|\tbenders\t||\t\t\torig\t\t|\t\t\tbenders\t\t||\t\t\torig\t\t|\t\tbenders\t\t|\t(orig-benders)\t||\t\t\t\torig\t\t\t\t|\t\t\tbenders\t\t\t||\t\t\t\torig\t|\t\t\tbenders\t||\t\t\t\torig\t|\t\t\tbenders\t||\t\t\t\torig\t|\t\t\tbenders\t||\t\t\t\torig\t|\t\t\tbenders\t||\t\t\t\torig\t|\t\t\tbenders\t||\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t||\n";

  int m = 0;
  for(int j=0; j<11;j++)
  {
    if(j==3 || j==6 || j==9)  m++;
    if((lb_flight_range[m]<=d && d<=ub_flight_range[m] && lb_aircraft_types[(int)j%3]<=c && c<=ub_aircraft_types[(int)j%3]) && !endstatistic && j<9)  write_stats(scip, 1, j);

    if(j!=10)
      stats << "\t" << setw(7) << t << "\t|";
    else
      stats << "\t" << setw(4) << 1 << "-" << setw(2) << t << "\t|";
    stats << "\t\t" << setw(5) << lb_flight_range[m] << "-" << setw(5) << ub_flight_range[m] << "\t\t|"
          << "\t\t\t\t" << setw(2) << lb_aircraft_types[(int)j%3] << "-" << setw(2) << ub_aircraft_types[j%3] << "\t\t\t|"
          << "\t\t\t" << setw(2) << lb_netclasses[m] << "-" << setw(2) << ub_netclasses[m] << "\t\t\t||"
          << "\t" << setw(6) << feasible_instances[0][j] << "\t\t|"
          << setw(6) << feasible_instances[1][j] << "\t\t||"
          << "\t\t" << setw(6) << feasible_instances_solved[0][j] << "\t\t|"
          << "\t\t" << setw(6) << feasible_instances_solved[1][j] << "\t\t\t||";
    if(feasible_instances_solved[1][j] != 0 && feasible_instances_solved[0][j] != 0 )
    {
      stats << "\t" << setw(10) << average_solving_time[0][j]/feasible_instances_solved[0][j] << "\t|"
            << "\t" << setw(10) << average_solving_time[1][j]/feasible_instances_solved[1][j] << "\t|"
            << "\t\t" << setw(10) << average_solving_time[0][j]/feasible_instances_solved[0][j] - average_solving_time[1][j]/feasible_instances_solved[1][j] << "\t\t||"
            << "\t" << setw(12) << sqrt(variance_solving_time[0][j]/feasible_instances_solved[0][j] - pow(average_solving_time[0][j]/feasible_instances_solved[0][j], 2)) << "\t\t\t|"
            << "\t" << setw(12) << sqrt(variance_solving_time[1][j]/feasible_instances_solved[1][j] - pow(average_solving_time[1][j]/feasible_instances_solved[1][j], 2)) << "\t\t||"
            << setw(12) << average_number_vars[0][j]/feasible_instances_solved[0][j] << "\t|"
            << setw(12) << average_number_vars[1][j]/feasible_instances_solved[1][j] << "\t||"
            << setw(12) << average_number_cons[0][j]/feasible_instances_solved[0][j] << "\t|"
            << setw(12) << average_number_cons[1][j]/feasible_instances_solved[1][j] << "\t||"
            << setw(12) << average_solving_nodes[0][j]/feasible_instances_solved[0][j] << "\t|"
            << setw(12) << average_solving_nodes[1][j]/feasible_instances_solved[1][j] << "\t||"
            << setw(12) << average_lp_iterations[0][j]/feasible_instances_solved[0][j] << "\t|"
            << setw(12) << average_lp_iterations[1][j]/feasible_instances_solved[1][j] << "\t||";
      if((feasible_instances[0][j]-feasible_instances_solved[0][j]) != 0)
        stats  << setw(12) << average_gaps[0][j]/(feasible_instances[0][j]-feasible_instances_solved[0][j]) << "\t|";
      else
        stats  << setw(12) << 0 << "\t|";
      if((feasible_instances[1][j]-feasible_instances_solved[1][j]))
        stats  << setw(12) << average_gaps[1][j]/(feasible_instances[1][j]-feasible_instances_solved[1][j]) << "\t||";
      else
        stats  << setw(12) << 0 << "\t||";

      stats << "\t\t" << setw(12) << (solution_gap_ab[0][j]/number_both_solved[j])-(solution_gap_ab[1][j]/number_both_solved[j]) << "\t\t\t\t\t\t\t||"
            << "\t\t" << setw(12) << ((solution_gap_ab[0][j]/number_both_solved[j])-(solution_gap_ab[1][j]/number_both_solved[j]))/(solution_gap_ab[0][j]/number_both_solved[j]) << "\t\n";
    }
    else if(feasible_instances_solved[1][j] != 0 && feasible_instances_solved[0][j] == 0 )
    {
      stats << "\t" << setw(10) << 0 << "\t|"
            << "\t" << setw(10) << average_solving_time[1][j]/feasible_instances_solved[1][j] << "\t|"
            << "\t\t" << setw(10) << 0 << "\t\t||"
            << "\t" << setw(12) << 0 << "\t\t\t|"
            << "\t" << setw(12) << sqrt(variance_solving_time[1][j]/feasible_instances_solved[1][j] - pow(average_solving_time[1][j]/feasible_instances_solved[1][j], 2)) << "\t\t||"
            << setw(12) << 0 << "\t|"
            << setw(12) << average_number_vars[1][j]/feasible_instances_solved[1][j] << "\t||"
            << setw(12) << 0 << "\t|"
            << setw(12) << average_number_cons[1][j]/feasible_instances_solved[1][j] << "\t||"
            << setw(12) << 0 << "\t|"
            << setw(12) << average_solving_nodes[1][j]/feasible_instances_solved[1][j] << "\t||"
            << setw(12) << 0 << "\t|"
            << setw(12) << average_lp_iterations[1][j]/feasible_instances_solved[1][j] << "\t||"
            << setw(12) << 0 << "\t|";
      if((feasible_instances[1][j]-feasible_instances_solved[1][j]))
        stats  << setw(12) << average_gaps[1][j]/(feasible_instances[1][j]-feasible_instances_solved[1][j]) << "\t||";
      else
        stats  << setw(12) << 0 << "\t||";

      stats << "\t\t" << setw(12) << 0 << "\t\t\t\t\t\t\t||"
            << "\t\t" << setw(12) << 0 << "\t\n";
    }
    else if(feasible_instances_solved[1][j] == 0 && feasible_instances_solved[0][j] != 0 )
    {
      stats << "\t" << setw(10) << average_solving_time[0][j]/feasible_instances_solved[0][j] << "\t|"
            << "\t" << setw(10) << 0 << "\t|"
            << "\t\t" << setw(10) << average_solving_time[0][j]/feasible_instances_solved[0][j] - average_solving_time[1][j]/feasible_instances_solved[1][j] << "\t\t||"
            << "\t" << setw(12) << sqrt(variance_solving_time[0][j]/feasible_instances_solved[0][j] - pow(average_solving_time[0][j]/feasible_instances_solved[0][j], 2)) << "\t\t\t|"
            << "\t" << setw(12) << 0 << "\t\t||"
            << setw(12) << average_number_vars[0][j]/feasible_instances_solved[0][j] << "\t|"
            << setw(12) << 0 << "\t||"
            << setw(12) << average_number_cons[0][j]/feasible_instances_solved[0][j] << "\t|"
            << setw(12) << 0 << "\t||"
            << setw(12) << average_solving_nodes[0][j]/feasible_instances_solved[0][j] << "\t|"
            << setw(12) << 0 << "\t||"
            << setw(12) << average_lp_iterations[0][j]/feasible_instances_solved[0][j] << "\t|"
            << setw(12) << 0 << "\t||";
      if((feasible_instances[0][j]-feasible_instances_solved[0][j]) != 0)
        stats  << setw(12) << average_gaps[0][j]/(feasible_instances[0][j]-feasible_instances_solved[0][j]) << "\t|";
      else
        stats  << setw(12) << 0 << "\t|";

      stats  << setw(12) << 0 << "\t||"
             << "\t\t" << setw(12) << (solution_gap_ab[0][j]/number_both_solved[j])-(solution_gap_ab[1][j]/number_both_solved[j]) << "\t\t\t\t\t\t\t||"
             << "\t\t" << setw(12) << ((solution_gap_ab[0][j]/number_both_solved[j])-(solution_gap_ab[1][j]/number_both_solved[j]))/(solution_gap_ab[0][j]/number_both_solved[j]) << "\t\n";
    }
    else
    {
      stats << "\t" << setw(10) << 0 << "\t|"
            << "\t" << setw(10) << 0 << "\t|"
            << "\t" << setw(10) << 0 << "\t\t\t||"
            << "\t" << setw(12) << 0 << "\t\t\t|"
            << "\t" << setw(12) << 0 << "\t\t||"
            << setw(12) << 0 << "\t|"
            << setw(12) << 0 << "\t||"
            << setw(12) << 0 << "\t|"
            << setw(12) << 0 << "\t||"
            << setw(12) << 0 << "\t|"
            << setw(12) << 0 << "\t||"
            << setw(12) << 0 << "\t|"
            << setw(12) << 0 << "\t||"
            << setw(12) << 0 << "\t|"
            << setw(12) << 0 << "\t||"
            << "\t\t" << setw(12) << 0 << "\t\t\t\t\t\t\t||"
            << "\t\t" << setw(12) << 0 << "\t\n";
    }
  }
  stats.close();
}

//prints all interesting results regarding an instance (both feasible and infeasible) 
void Printer::print_single_results(SCIP* scip, int t, int c, int n, int round, int i, bool feasible)
{
  ofstream single_results;
  if(feasible)
  {
    if (i == 0) single_results.open("../output/" + date_namebuf.str() + "/" + std::to_string(t) + "_periods" + "/single_results_orig_feasible.csv", ios::app);
    if (i == 1) single_results.open("../output/" + date_namebuf.str() + "/" + std::to_string(t) + "_periods" + "/single_results_benders_feasible.csv", ios::app);
  }
  else
  {
    if (i == 0) single_results.open("../output/" + date_namebuf.str() + "/" + std::to_string(t) + "_periods" + "/single_results_orig_infeasible.csv", ios::app);
    if (i == 1) single_results.open("../output/" + date_namebuf.str() + "/" + std::to_string(t) + "_periods" + "/single_results_benders_infeasible.csv", ios::app);
  }

  single_results << "\t" << setw(7) << t << "\t|"
                    << "\t\t\t" << setw(9) << c << "\t\t|"
                    << "\t\t" << setw(9) << n << "\t\t|"
                    << "\t" << setw(5) << round+1 << "\t|"
                    << "\t" << setw(16) << SCIPgetSolvingTime(scip) << "\t|"
                    << "\t" << setw(12) << SCIPgetNNodes(scip) << "\t|"
                    << "\t" << setw(12) << SCIPgetNLPIterations(scip) << "\t|"
                    << "\t" << setw(12) << SCIPgetNVars(scip) << "\t|"
                    << "\t" << setw(12) << SCIPgetNConss(scip) << "\t|"
                    << "\t" << setw(12) << SCIPgetNLPRows(scip) << "\t|"
                    << "\t" << setw(12) << SCIPgetPrimalbound(scip) << "\t|"
                    << "\t" << setw(12) << SCIPgetDualbound(scip) << "\t|"
                    << "\t" << setw(12) << SCIPgetGap(scip) << "\t\n";
  single_results.close();

  if(feasible)
  {
    ofstream single_results_all;
    if(i == 0) single_results_all.open("../output/" + date_namebuf.str() + "/single_results_orig_feasible.csv", ios::app);
    if(i == 1) single_results_all.open("../output/" + date_namebuf.str() + "/single_results_benders_feasible.csv", ios::app);
    single_results_all << "\t" << setw(7) << t << "\t|"
                        << "\t\t\t" << setw(9) << c << "\t\t|"
                        << "\t\t" << setw(9) << n << "\t\t|"
                        << "\t" << setw(5) << round+1 << "\t|"
                        << "\t" << setw(16) << SCIPgetSolvingTime(scip) << "\t|"
                        << "\t" << setw(12) << SCIPgetNNodes(scip) << "\t|"
                        << "\t" << setw(12) << SCIPgetNLPIterations(scip) << "\t|"
                        << "\t" << setw(12) << SCIPgetNVars(scip) << "\t|"
                        << "\t" << setw(12) << SCIPgetNConss(scip) << "\t|"
                        << "\t" << setw(12) << SCIPgetNLPRows(scip) << "\t|"
                        << "\t" << setw(12) << SCIPgetPrimalbound(scip) << "\t|"
                        << "\t" << setw(12) << SCIPgetDualbound(scip) << "\t|"
                        << "\t" << setw(12) << SCIPgetGap(scip) << "\t\n";
    single_results_all.close();
  }
}

//function that computes the average values for the full_stats
void Printer::write_stats(SCIP* scip, int i, int j)
{
  if(SCIPgetStatus(scip)==SCIP_STATUS_TIMELIMIT)
  {
    feasible_instances[i][j]  += 1;
    feasible_instances[i][9]  += 1;
    feasible_instances[i][10] += 1;
    average_gaps[i][j]        += SCIPgetGap(scip);
    average_gaps[i][9]        += SCIPgetGap(scip);
    average_gaps[i][10]       += SCIPgetGap(scip);
    if(i==0) orig_time = true;
  }
  else if(SCIPgetStatus(scip)==SCIP_STATUS_OPTIMAL)
  {
    feasible_instances[i][j]        += 1;
    feasible_instances_solved[i][j] += 1;
    average_solving_time[i][j]      += SCIPgetSolvingTime(scip);
    variance_solving_time[i][j]     += SCIPgetSolvingTime(scip)*SCIPgetSolvingTime(scip);
    average_number_vars[i][j]       += SCIPgetNVars(scip);
    average_number_cons[i][j]       += SCIPgetNConss(scip);
    average_solving_nodes[i][j]     += SCIPgetNNodes(scip);
    average_lp_iterations[i][j]     += SCIPgetNLPIterations(scip);
    
    if(i==1 && !orig_time)
    {
      solution_gap_ab[i][j]  += SCIPgetPrimalbound(scip);
      solution_gap_ab[i][9]  += SCIPgetPrimalbound(scip);
      solution_gap_ab[i][10] += SCIPgetPrimalbound(scip);
      solution_gap_ab[0][j]  += orig_sol_buffer;
      solution_gap_ab[0][9]  += orig_sol_buffer;
      solution_gap_ab[0][10] += orig_sol_buffer;
      number_both_solved[j]  += 1;
      number_both_solved[9]  += 1;
      number_both_solved[10] += 1;
    }
    else if(i==1 && orig_time)
      orig_time = false;
    else if(i==0)
      orig_sol_buffer = SCIPgetPrimalbound(scip);

    feasible_instances[i][9]        += 1;
    feasible_instances_solved[i][9] += 1;
    average_solving_time[i][9]      += SCIPgetSolvingTime(scip);
    variance_solving_time[i][9]     += SCIPgetSolvingTime(scip)*SCIPgetSolvingTime(scip);
    average_number_vars[i][9]       += SCIPgetNVars(scip);
    average_number_cons[i][9]       += SCIPgetNConss(scip);
    average_solving_nodes[i][9]     += SCIPgetNNodes(scip);
    average_lp_iterations[i][9]     += SCIPgetNLPIterations(scip);

    feasible_instances[i][10]        += 1;
    feasible_instances_solved[i][10] += 1;
    average_solving_time[i][10]      += SCIPgetSolvingTime(scip);
    variance_solving_time[i][10]     += SCIPgetSolvingTime(scip)*SCIPgetSolvingTime(scip);
    average_number_vars[i][10]       += SCIPgetNVars(scip);
    average_number_cons[i][10]       += SCIPgetNConss(scip);
    average_solving_nodes[i][10]     += SCIPgetNNodes(scip);
    average_lp_iterations[i][10]     += SCIPgetNLPIterations(scip);
  }
}

//preperation to be able to write down the single results in one (or more) files
void Printer::print_all_headers(int t, int i)
{
  ofstream single_results;
  if (i == 0) single_results.open("../output/" + date_namebuf.str() + "/" + std::to_string(t) + "_periods" + "/single_results_orig_feasible.csv", ios::app);
  if (i == 1) single_results.open("../output/" + date_namebuf.str() + "/" + std::to_string(t) + "_periods" + "/single_results_benders_feasible.csv", ios::app);
  single_results << "#periods\t|\t#aircraft_types\t|\t#net_classes\t|\tround\t|\t\t\t\ttime\t\t\t\t|\t\t\tnodes\t\t\t|\t\titerations\t|\t\tvariables\t\t|\tconstraints\t\t|\t\t\trows\t\t\t|\tprimalbound\t\t|\t\tdualbound\t\t|\t\t\tgap\t\t\n";
  single_results.close();

  ofstream single_results_in;
  if (i == 0) single_results_in.open("../output/" + date_namebuf.str() + "/" + std::to_string(t) + "_periods" + "/single_results_orig_infeasible.csv", ios::app);
  if (i == 1) single_results_in.open("../output/" + date_namebuf.str() + "/" + std::to_string(t) + "_periods" + "/single_results_benders_infeasible.csv", ios::app);
  single_results_in << "#periods\t|\t#aircraft_types\t|\t#net_classes\t|\tround\t|\t\t\t\ttime\t\t\t\t|\t\t\tnodes\t\t\t|\t\titerations\t|\t\tvariables\t\t|\tconstraints\t\t|\t\t\trows\t\t\t|\tprimalbound\t\t|\t\tdualbound\t\t|\t\t\tgap\t\t\n";
  single_results_in.close();
}

//for nicer names of the different rounds; just defined until 30
string stringify_number(int n)
{
  vector<string> special = {"first", "second", "third", "fourth", "fifth", "sixth", "seventh", "eighth", "ninth", "tenth", "eleventh", "twelfth", "thirteenth", "fourteenth", "fifteenth", "sixteenth", "seventeenth", "eighteenth", "nineteenth", "twentieth", "twenty-first", "twenty-seccond", "twenty-third", "twenty-fourth", "twenty-fifth", "twenty-sixth", "twenty-seventh", "twenty-eighth", "twenty-ninth", "thirtieth"};
  return(special[n]);
}

//prints the problem formulation scip solves
void print_scip_problem(SCIP *scip, string problem_name)
{
  FILE * pFile = fopen((Path + std::string("/problem_formulations/") + problem_name + std::string(".lp")).c_str(), "w");
  SCIPprintOrigProblem(scip, pFile, "lp", false);
  fclose (pFile);
}

//prints the solution of the solved problem
void print_scip_solution(SCIP* scip, string problem_name)
{
  FILE * pFile = fopen((Path + std::string("/solutions/") + problem_name + std::string(".lp")).c_str(), "w");
  SCIP_CALL_EXC(SCIPprintBestSol(scip, pFile, false));
  fclose(pFile);
}

void print_benders_statistics(SCIP* scip, string name)
{
  FILE * pFile = fopen((Path + std::string("/log/") + name + std::string(".lp")).c_str(), "w");
  SCIPprintBendersStatistics(scip, pFile);
  fclose(pFile);
}

//opens necessary folder to print data
void print_dataset(int t, int c, int n, int r, Instance& instance)
{
  boost::filesystem::path dir_four("../output/" + date_namebuf.str() + "/" + std::to_string(t) + "_periods");
  boost::filesystem::path dir_six("../output/" + date_namebuf.str() + "/" + std::to_string(t) + "_periods/" + std::to_string(n) + "_netclasses");
  boost::filesystem::path dir_five("../output/" + date_namebuf.str() + "/" + std::to_string(t) + "_periods/" + std::to_string(n) + "_netclasses/"  + std::to_string(c) + "_base_aircraft_types");
  boost::filesystem::path dir_two("../output/" + date_namebuf.str() + "/" + std::to_string(t) + "_periods/" + std::to_string(n) + "_netclasses/"  + std::to_string(c) + "_base_aircraft_types/" + stringify_number(r) + "_round");
  boost::filesystem::path dir_three("../output/" + date_namebuf.str() + "/" + std::to_string(t) + "_periods/" + std::to_string(n) + "_netclasses/"  + std::to_string(c) + "_base_aircraft_types/" + stringify_number(r) + "_round/problem_formulations");
  boost::filesystem::path dir_one("../output/" + date_namebuf.str() + "/" + std::to_string(t) + "_periods/" + std::to_string(n) + "_netclasses/"  + std::to_string(c) + "_base_aircraft_types/" + stringify_number(r) + "_round/variables");
  boost::filesystem::path dir_seven("../output/" + date_namebuf.str() + "/" + std::to_string(t) + "_periods/" + std::to_string(n) + "_netclasses/"  + std::to_string(c) + "_base_aircraft_types/" + stringify_number(r) + "_round/solutions");
  boost::filesystem::path dir_eight("../output/" + date_namebuf.str() + "/" + std::to_string(t) + "_periods/" + std::to_string(n) + "_netclasses/"  + std::to_string(c) + "_base_aircraft_types/" + stringify_number(r) + "_round/log");
  boost::filesystem::path dir_nine("../output/" + date_namebuf.str() + "/" + std::to_string(t) + "_periods/" + std::to_string(n) + "_netclasses/"  + std::to_string(c) + "_base_aircraft_types/" + stringify_number(r) + "_round/data");

  if(!(boost::filesystem::exists(dir_four)))
  {
    if (boost::filesystem::create_directory(dir_four))
    {
      std::cout << "folder periods successfully created!\n";
    }
  }
  if(!(boost::filesystem::exists(dir_six)))
  {
    if (boost::filesystem::create_directory(dir_six))
    {
      std::cout << "folder netclasses successfully created!\n";
    }
  }
  if(!(boost::filesystem::exists(dir_five)))
  {
    if (boost::filesystem::create_directory(dir_five))
    {
      std::cout << "folder base aircraft types successfully created!\n";
    }
  }
  if(!(boost::filesystem::exists(dir_two)))
  {
      if (boost::filesystem::create_directory(dir_two))
      {
          std::cout << "folder " + stringify_number(r) + "_round successfully created!\n";
      }
  }
  if(!(boost::filesystem::exists(dir_three)))
  {
      if (boost::filesystem::create_directory(dir_three))
      {
          std::cout << "folder problem_formulations successfully created!\n";
      }
  }
  if(!(boost::filesystem::exists(dir_one)))
  {
      if (boost::filesystem::create_directory(dir_one))
      {
          std::cout << "folder variables successfully created!\n";
      }
  }
  if(!(boost::filesystem::exists(dir_seven)))
  {
      if (boost::filesystem::create_directory(dir_seven))
      {
          std::cout << "folder solutions successfully created!\n";
      }
  }
  if(!(boost::filesystem::exists(dir_eight)))
  {
      if (boost::filesystem::create_directory(dir_eight))
      {
          std::cout << "folder log successfully created!\n";
      }
  }
  if(!(boost::filesystem::exists(dir_nine)))
  {
      if (boost::filesystem::create_directory(dir_nine))
      {
          std::cout << "folder data successfully created!\n";
      }
  }
  Path = std::string("../output/") + date_namebuf.str() + "/" + std::to_string(t) + std::string("_periods/") + std::to_string(n) + std::string("_netclasses/") + std::to_string(c) + std::string("_base_aircraft_types/") + stringify_number(r) + std::string("_round/");

  Market_Situation& market               = instance.get_market();
  vector<Retrofit>& retrofits            = market.get_retrofits();
  vector<Aircraft_Properties>& aircrafts = instance.get_aircrafts();
  vector<Net_Class>& net_classes         = instance.get_net_classes();
  ofstream data;

  data.open("../output/" + date_namebuf.str() + "/" + std::to_string(t) + "_periods/" + std::to_string(n) + "_netclasses/" + std::to_string(c) + "_base_aircraft_types/" + stringify_number(r) + "_round/data/market_situation.txt", ios::app);
  print_market_situation(t, c, n, r, market, aircrafts, data);
  data.close();

  data.open("../output/" + date_namebuf.str() + "/" + std::to_string(t) + "_periods/" + std::to_string(n) + "_netclasses/"  + std::to_string(c) + "_base_aircraft_types/" + stringify_number(r) + "_round/data/aircraft_types.txt", ios::app);
  print_aircraft_types_age_indept(t, c, n, r, aircrafts, data);
  data.close();

  data.open("../output/" + date_namebuf.str() + "/" + std::to_string(t) + "_periods/" + std::to_string(n) + "_netclasses/"  + std::to_string(c) + "_base_aircraft_types/" + stringify_number(r) + "_round/data/age_dependent_properties.txt", ios::app);
  print_age_dept_props(t, c, n, r, market, aircrafts, data);
  data.close();

  data.open("../output/" + date_namebuf.str() + "/" + std::to_string(t) + "_periods/" + std::to_string(n) + "_netclasses/"  + std::to_string(c) + "_base_aircraft_types/" + stringify_number(r) + "_round/data/retrofits.txt", ios::app);
  print_retrofits(t, c, n, r, retrofits, data);
  data.close();

  data.open("../output/" + date_namebuf.str() + "/" + std::to_string(t) + "_periods/" + std::to_string(n) + "_netclasses/"  + std::to_string(c) + "_base_aircraft_types/" + stringify_number(r) + "_round/data/net_classes.txt", ios::app);
  print_net_classes(t, c, n, r, net_classes, data);
  data.close();
}

//print the market situation
void print_market_situation(int t, int c, int n, int rounds, Market_Situation& market, vector<Aircraft_Properties>& aircrafts, ofstream& data_ms)
{
  for (int i = 0; i < t; i++)
  {
    if(i == 0)
    {
    data_ms << "#periods\t|\t#base_aircraft_types\t|\t#net_classes\t|\tround\t|\temission_proportion_to_compensate\t|\tperiod\t|\tfuel_price\t|\temission_price_EU\t|\temission_price_I\t|\taircraft_base_type\t|\tpurchase_price\t|\t#purchasable_aircrafts\t|\tfuel_reduction\t|\tretrofittable_in\t|\tretrofit_price\n";
    data_ms << "\t" << setw(7) << t << "\t|"
            << "\t\t\t" << setw(9) << c << "\t\t\t\t\t|"
            << "\t\t" << setw(9) << n << "\t\t|"
            << "\t" << setw(5) << rounds+1 << "\t|"
            << "\t" << setw(20) << market.get_prop_em_compensation()[1] << "\t\t\t\t\t\t\t|";
    }
    else
    {
      data_ms << "\t\t\t\t\t|\t\t\t\t\t\t\t\t\t\t\t\t|\t\t\t\t\t\t\t\t|\t\t\t\t|\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t|\tperiod\t|\tfuel_price\t|\temission_price_EU\t|\temission_price_I\t|\taircraft_base_type\t|\tpurchase_price\t|\t#purchasable_aircrafts\t|\tfuel_reduction\t|\tretrofittable_in\t|\tretrofit_price\n";
      data_ms << "\t\t\t\t\t|\t\t\t\t\t\t\t\t\t\t\t\t|\t\t\t\t\t\t\t\t|\t\t\t\t|\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t|"; //\t\t\t\t\t\t\t\t|\t\t\t\t\t|\t\t\t\t\t|";
    }
    data_ms << "\t" << setw(6) << i+1 << "\t|"
            << "\t" << setw(10) << market.get_fuel_price_at(i) << "\t|"
            << "\t\t\t" << setw(10) << market.get_emission_price_in_as_at(0,i) << "\t\t|"
            << "\t\t\t" << setw(10) << market.get_emission_price_in_as_at(1,i) << "\t\t|";
    for (int j = 0; j < aircrafts.size(); j++)
    {
      if (j != 0)
      {
        data_ms << "\t\t\t\t\t|\t\t\t\t\t\t\t\t\t\t\t\t|\t\t\t\t\t\t\t\t|\t\t\t\t|\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t|\t\t\t\t\t|\t\t\t\t\t\t\t|\t\t\t\t\t\t\t\t\t\t|\t\t\t\t\t\t\t\t\t\t|";
      }
      if(!aircrafts[j].is_retrofitted())
      {
        data_ms << "\t\t\t\t" << setw(7) << j << "\t\t\t\t|"
                << "\t" << setw(14) << market.get_purchase_price_for_at(j,i) << "\t|"
                << "\t\t\t" << setw(14) << market.get_nmb_purchasable_aircrafts_of_at(j,i) << "\t\t\t|"
                << "\t\t\t\t\t\t\t\t\t|";
      }
      else{
        data_ms << "\t\t\t(r:\t" << setw(3) << j << ")\t\t\t\t|"
                << "\t\t\t\t\t\t\t\t\t|\t\t\t\t\t\t\t\t\t\t\t\t\t|"
                << setw(3) << aircrafts[j].get_fuel_reduction().first << ", " << setw(10) << aircrafts[j].get_fuel_reduction().second << "\t|";
      }
      for(int h = 0; h < aircrafts[j].get_retrofit_aims().size(); h++){
        if(h != 0 && !aircrafts[j].is_retrofitted()){
          data_ms << "\t\t\t\t\t|\t\t\t\t\t\t\t\t\t\t\t\t|\t\t\t\t\t\t\t\t|\t\t\t\t|\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t|\t\t\t\t\t|\t\t\t\t\t\t\t|\t\t\t\t\t\t\t\t\t\t|\t\t\t\t\t\t\t\t\t\t|\t\t\t\t\t\t\t\t\t\t\t|\t\t\t\t\t\t\t\t\t|\t\t\t\t\t\t\t\t\t\t\t\t\t|\t\t\t\t\t\t\t\t\t|";
        }
        else if(h != 0){
          data_ms << "\t\t\t\t\t|\t\t\t\t\t\t\t\t\t\t\t\t|\t\t\t\t\t\t\t\t|\t\t\t\t|\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t|\t\t\t\t\t|\t\t\t\t\t\t\t|\t\t\t\t\t\t\t\t\t\t|\t\t\t\t\t\t\t\t\t\t|";
        }
        data_ms << "\t\t" << setw(7) << aircrafts[j].get_retrofit_aims()[h] << "\t\t\t\t\t|";
        if(!aircrafts[j].is_retrofitted()){
          data_ms << "\t" << setw(7) << aircrafts[j].get_retrofit_price(h,i) << "\t\n";
        }
        else{
          data_ms << "is retrofitted\n";
        }
      }
    }
  }
  data_ms.flush();
}

// print aircraft types -- age independent
void print_aircraft_types_age_indept(int t, int c, int n, int rounds, vector<Aircraft_Properties>& aircrafts, ofstream& data_a)
{
  data_a << "#periods\t|\t#base_aircraft_types\t|\t#net_classes\t|\tround\t|\taircraft_type\t|\tmax_service_time\t|\t#seats\t|\t#aisles\t|\tflight_range\t|\t\t\t\t\t\t\t\t\t\tfuel_parameter\t\t\t\t\t\t\t\t\t\t\t|\tblocked_time_parameter\t|\t\t\t\t\t\t\t\ttechnical_details\t\t\t\t\t\t\t\t|\tpurchasable_since\t|\tis_retrofitted?\t|\torigin\t|\tretrofit_purchasable_since\t|\tretrofittable_in\n";

  data_a << "\t" << setw(7) << t << "\t|"
          << "\t\t\t" << setw(9) << c << "\t\t\t\t\t|"
          << "\t\t" << setw(9) << n << "\t\t|"
          << "\t" << setw(5) << rounds+1 << "\t|";

  for(int j = 0; j < aircrafts.size(); j++){
    if(j != 0){
      data_a << "\t\t\t\t\t|\t\t\t\t\t\t\t\t\t\t\t\t|\t\t\t\t\t\t\t\t|\t\t\t\t|";
    }
    data_a << "\t\t\t\t" << setw(7) << j << "\t|"
           << "\t\t\t\t" << setw(10) << aircrafts[j].get_service_t() << "\t|"
           << "\t" << setw(7) << aircrafts[j].get_nmb_seats() << "\t|"
           << "\t" << setw(7) << aircrafts[j].get_nmb_aisles() << "\t|"
           << "\t" << setw(10) << aircrafts[j].get_range() << "\t\t|"
           << "\t" << "e_i: " << setw(11) << aircrafts[j].get_fuel_use_param().e_i << ", f_i: " << setw(7) << aircrafts[j].get_fuel_use_param().f_i << ", g_i: " << setw(4) << aircrafts[j].get_fuel_use_param().g_i << ", h_i: " << setw(4) << aircrafts[j].get_fuel_use_param().h_i << "\t|"
           << "\t" << "c_i: " << setw(7) << aircrafts[j].get_block_time_param().c_i << ", d_i: " << setw(2) << aircrafts[j].get_block_time_param().d_i << "\t|"
           << "\t" << "nmb_eng: " << setw(3) << aircrafts[j].get_tech_data().nmb_eng << ", slst: " << setw(4) << aircrafts[j].get_tech_data().slst << ", mtgw: " << setw(3) << aircrafts[j].get_tech_data().mtgw << ", afw: " << setw(3) << aircrafts[j].get_tech_data().afw << "\t|"
           << "\t" << setw(15) << aircrafts[j].get_purchasable_since_period() << "\t\t|"
           << "\t\t\t\t" << setw(7) << std::boolalpha << aircrafts[j].is_retrofitted() << "\t\t|";
    if(aircrafts[j].is_retrofitted()){
      data_a << "\t" << setw(7) << aircrafts[j].get_origin() << "\t|"
       << "\t\t\t" << setw(10) << aircrafts[j].get_retro_purchasable_since_period() << "\t\t\t\t\t\t\t|";
    }
    else {
      data_a << "\t\t\t\t\t|\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t|";
    }
    for(int k = 0; k < aircrafts[j].get_retrofit_aims().size(); k++){
      data_a << setw(2) << aircrafts[j].get_retrofit_aims()[k] << ", ";
    }
  data_a << "\n";
  }
  data_a.flush();
}

//print age dependent properties
void print_age_dept_props(int t, int c, int n, int rounds, Market_Situation& market, vector<Aircraft_Properties>& aircrafts, ofstream& data_ad)
{
  data_ad << "\t#periods:\t" << t << "\t|"
          << "\t#base_aircraft_types:\t" << c << "\t|"
          << "\t#net_classes:\t" << setw(3) << n << "\t|"
          << "\tround:\t" << rounds+1 << "\n";

  for(int j = 0; j < aircrafts.size(); j++)
  {
    data_ad << "aircraft_type\t|\tage\t|\t\t#service_hours\t|\t\t#owned_at_start\t|\tperiod: retail_price_at_age\n"
                   << "\t\t\t\t\t\t\t|\t\t\t|\t\t\t\t\t\t\t\t\t\t|\t\t\t\t\t\t\t\t\t\t|";
    for(int l = 0; l < t; l++)
    {
      data_ad << "\t\t\t" << setw(8) << l+1 << "\t\t|";
      }
    data_ad << "\n\t\t\t" << setw(7) << j << "\t|";
    for(int i = 0; i < aircrafts[j].get_service_t()+1; i++)
    {
      if(i != 0)
      {
        data_ad << "\t\t\t\t\t\t\t|";
      }
        data_ad << "\t" << setw(2) << i << "\t|"
                << "\t\t\t" << setw(12) << aircrafts[j].get_hours_p_a_at(i) << "\t|";
        if (aircrafts[j].get_nmb_owned_at_start().size() > 0)
        {
          data_ad << "\t\t\t" << setw(12) << aircrafts[j].get_nmb_owned_at_start()[i] << "\t|";
        }
        else{
          data_ad << "\t\t\t\t\t\t\t\t\t\t|";
        }
      for(int l = 0; l < t; l++)
      {
        data_ad << "\t" << setw(14) << market.get_retail_price_for_at_at(j,l,i) << "\t|";
      }
       data_ad << "\n";
    }
  }
  data_ad.flush();
}

//print retrofit options
void print_retrofits(int t, int c, int n, int rounds, vector<Retrofit>& retrofits, ofstream& data_r)
{
  data_r << "#periods\t|\t#base_aircraft_types\t|\t#net_classes\t|\tround\t|\tretrofit\t|\t#max_retrofits_per_period\t|\tretrofit_candidate\t|\t\tidx\t\t|\t\ttype\t|\tprice\t|\tpurchasable_since\n"
         << "\t" << setw(7) << t << "\t|"
         << "\t\t\t" << setw(9) << c << "\t\t\t\t\t|"
         << "\t\t" << setw(9) << n << "\t\t|"
         << "\t" << setw(5) << rounds+1 << "\t|";

  for(int i = 0; i < retrofits.size();  i++){
    if(i != 0){
      data_r << "\t\t\t\t\t|\t\t\t\t\t\t\t\t\t\t\t\t|\t\t\t\t\t\t\t\t|\t\t\t\t|";
    }
    data_r << "\t" << setw(7) << i << "\t\t|"
           << "\t\t\t\t\t\t" << setw(10) << retrofits[i].get_max_per_t() << "\t\t\t|";
    if(retrofits[i].get_candidates().size() == 0){
      data_r << "\n";
    }
    else{
      for(int j = 0; j < retrofits[i].get_candidates().size(); j++){
        if(j != 0){
          data_r << "\t\t\t\t\t|\t\t\t\t\t\t\t\t\t\t\t\t|\t\t\t\t\t\t\t\t|\t\t\t\t|\t\t\t\t\t\t|\t\t\t\t\t\t\t\t\t\t\t\t\t\t|";
        }
        data_r << "\t\t\t" << setw(7) << j << "\t\t\t\t\t|"
               << "" << setw(7) << retrofits[i].get_candidates()[j].idx << "\t|"
               << "" << setw(7) << retrofits[i].get_candidates()[j].type << "\t|"
               << "" << setw(5) << retrofits[i].get_candidates()[j].price << "\t|"
               << "" << setw(7) << retrofits[i].get_candidates()[j].purchasable_since << "\t\n";
      }
    }
  }
  data_r.flush();
}

//print net_classes
void print_net_classes(int t, int c, int n, int rounds, vector<Net_Class>& net_classes, ofstream& data_nc)
{
  data_nc << "#periods\t|\t#base_aircraft_types\t|\t#net_classes\t|\tround\t|\tperiod\t|\tnetclass\t|\t\t\t\t\t\t\t#flights\t\t\t\t\t\t\t|\t\tdistance_range\t\t|\t#seats_per_flight\t|\t\t\tfull_demand_of_seats_per_period\t\t\t|\t#usable_aircrafts\t|\tfuel_use_per_flight\t|\tblocked_time_per_flight_in_min\n";
  data_nc << "\t\t\t\t\t|\t\t\t\t\t\t\t\t\t\t\t\t|\t\t\t\t\t\t\t\t|\t\t\t\t|\t\t\t\t\t|\t\t\t\t\t\t|\t\t\tall\t\t|\t\t\tEU\t\t|\t\t\tI\t\t\t|\t\t\t\t\t\t\t\t\t\t\t|\t\t\t\t\t\t\t\t\t\t|\t\t\tall\t\t\t|\t\t\tEU\t\t\t|\t\t\tI\t\t\t\t|\t\t\t\t\t\t\t\t\t\t|\t\t\t\t\t\t\t\t\t\t\t|\n";
  data_nc << "\t" << setw(7) << t << "\t|"
          << "\t\t\t" << setw(9) << c << "\t\t\t\t\t|"
          << "\t\t" << setw(9) << n << "\t\t|"
          << "\t" << setw(5) << rounds+1 << "\t|";

  for (int i = 0; i < t; i++){
    if (i != 0){
      data_nc << "#periods\t|\t#base_aircraft_types\t|\t#net_classes\t|\tround\t|\tperiod\t|\tnetclass\t|\t\t\t\t\t\t\t#flights\t\t\t\t\t\t\t|\t\tdistance_range\t\t|\t#seats_per_flight\t|\t\t\tfull_demand_of_seats_per_period\t\t\t|\t#usable_aircrafts\t|\tfuel_use_per_flight\t|\tblocked_time_per_flight_in_min\n";
      data_nc << "\t\t\t\t\t|\t\t\t\t\t\t\t\t\t\t\t\t|\t\t\t\t\t\t\t\t|\t\t\t\t|\t\t\t\t\t|\t\t\t\t\t\t|\t\t\tall\t\t|\t\t\tEU\t\t|\t\t\tI\t\t\t|\t\t\t\t\t\t\t\t\t\t\t|\t\t\t\t\t\t\t\t\t\t|\t\t\tall\t\t\t|\t\t\tEU\t\t\t|\t\t\tI\t\t\t\t|\t\t\t\t\t\t\t\t\t\t|\t\t\t\t\t\t\t\t\t\t\t|\n";
      data_nc << "\t\t\t\t\t|\t\t\t\t\t\t\t\t\t\t\t\t|\t\t\t\t\t\t\t\t|\t\t\t\t|";
    }
    data_nc << "\t" << setw(7) << i+1 << "\t|";
    for(int j = 0; j < n; j++){
      if (j != 0){
        data_nc << "\t\t\t\t\t|\t\t\t\t\t\t\t\t\t\t\t\t|\t\t\t\t\t\t\t\t|\t\t\t\t|\t\t\t\t\t|";
      }
      data_nc << "\t" << setw(8) << j << "\t|"
              << "\t" << setw(8) << net_classes[j].get_D_flights()[i] << "\t|"
              << "\t" << setw(8) << net_classes[j].get_D_flights_in_as_at_t(0,i) << "\t|"
              << "\t" << setw(8) << net_classes[j].get_D_flights_in_as_at_t(1,i) << "\t|"
              << "\t\t(" << setw(5) << net_classes[j].get_dist_range().second << ", " << setw(5) << net_classes[j].get_dist_range().first << ")\t\t|"
              << "\t\t" << setw(9) << net_classes[j].get_seats_per_flight().first << "-" << setw(3) << net_classes[j].get_seats_per_flight().second << "\t\t|"
              << "\t" << setw(10) << net_classes[j].get_D_seats()[i] << "\t|"
              << "\t" << setw(10) << net_classes[j].get_D_seats_EU()[i] << "\t|"
              << "\t" << setw(10) << net_classes[j].get_D_seats_I()[i] << "\t|";
      if((int)net_classes[j].get_usable_aircrafts().size() == 0)
      {
        data_nc << "no usable aircraft types\n";
      }
      for(int k = 0; k < (int)net_classes[j].get_usable_aircrafts().size() ; k++)
      {
        if(k != 0)
        {
          data_nc << "\t\t\t\t\t|\t\t\t\t\t\t\t\t\t\t\t\t|\t\t\t\t\t\t\t\t|\t\t\t\t|\t\t\t\t\t|\t\t\t\t\t\t|\t\t\t\t\t\t|\t\t\t\t\t\t|\t\t\t\t\t\t|\t\t\t\t\t\t\t\t\t\t\t|\t\t\t\t\t\t\t\t\t\t|\t\t\t\t\t\t\t|\t\t\t\t\t\t\t|\t\t\t\t\t\t\t|";
        }
        data_nc << "\t\t\t\t" << setw(3) << net_classes[j].get_usable_aircrafts()[k] << "\t\t\t\t\t|"
                << "\t\t" << setw(15) << net_classes[j].get_fuel_uses()[k] << "\t\t|"
                << "\t" << setw(10) << net_classes[j].get_blocked_times()[k] << "\t\n";
      }
    }
  }
  data_nc.flush();
}

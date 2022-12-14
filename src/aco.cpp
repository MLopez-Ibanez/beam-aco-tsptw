/*************************************************************************

 Beam-ACO

 ---------------------------------------------------------------------

                       Copyright (c) 2008
                  Christian Blum <christian.blum@ehu.es>
             Manuel Lopez-Ibanez <manuel.lopez-ibanez@manchester.ac.uk>

 This program is free software (software libre); you can redistribute
 it and/or modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2 of the
 License, or (at your option) any later version.

 This program is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, you can obtain a copy of the GNU
 General Public License at: http://www.gnu.org/licenses/gpl.html

 ----------------------------------------------------------------------

 Relevant literature:

*************************************************************************/

#ifdef HAVE_CONFIG_H
#include <misc-math.h>
#endif

#include <cerrno>
// Requires #define _GNU_SOURCE
extern char * program_invocation_short_name;

#include "Random.h"
#include "Timer.h"
#include "ant.h"
#include <string>
#include <cstring>
#include <list>
#include <vector>
#include <climits>

unsigned int random_seed;

// the following five variables are involved in termination criteria issues
int n_of_iter = INT_MAX;
double time_limit = DBL_MAX;
double time_taken;
double time_localsearch;
double time_init;

// n_of_ants: the number of ants
int n_of_ants = 1;

// l_rate: the learning rate (used for updating the pheromone values)
double l_rate = 0.1;

// tau_min, tau_max: lower, respecively upper, limit for the pheromone values
double tau_min = 0.001;
double tau_max = 0.999;

// det_rate: rate of determinism in the solution construction
double det_rate = 0.9;

// n_of_trials: the number of trials that is to be executed for the
// given problem instance
int n_of_trials = 1;

// parameters for the beam procedure
int beam_width = 1;
double mu = 2.0;
int max_children = 100;

// stochastic sampling parameter
int n_samples = 10;
int sample_percent = 100;
int sample_rate = -1;


// variable that holds the name of the input file
string input_filename;

FILE *trace_stream;
string trace_filename;


/* Initialization of the three solutions 'best_so_far', 'restart_best'
   and 'iteration_best', which are used to update the pheromone
   values */
Solution* best_so_far = NULL;
Solution* restart_best = NULL;
Solution* iteration_best = NULL;

#include "common.h"

static void usage(void)
{
  printf("\n"
         "Usage: %s -i FILE [OPTIONS]\n\n", program_invocation_short_name);

    printf(
"Description.\n"
"\n\n"

"Options:\n"
" -h, --help          print this summary and exit.                          \n"
" -v, --version       print version number and exit.                        \n"
" -i, --input   FILE  instance file.                                        \n"
" -s, --seed          random seed.                                          \n"
" -t, --time    REAL  time limit of each trial (seconds).                   \n"
" -n, --iterations INT number of iterations per trial.                      \n"
" -T, --trace   FILE  trace file.                                           \n"
" -r, --trials  INT   number of trials to be run on one instance.           \n"
" -a, --ants INTEGER  number of ants (defaut: %d).                          \n"
" -b, --beamwidth INTEGER  width of the beam search (default: %d).          \n"
" -m, --mu            (default: %g).                                        \n"
" -w, --weights       heuristic information weights (default: random).      \n"
" -S, --samples       number of stochastic samples per partial solution     \n"
"                     (default: %d).                                        \n"
"     --sample-rate   percentage of variables generated by stochastic sample\n"
"                     (default: %d).                                        \n"
"     --maxchild      maximum number of children (default: %d).             \n"
"     --lrate      learning rate used for updating pheromones (default: %g).\n"
"     --detrate    rate of determinism in the solution construction         \n"
"                  (default: %g).                                           \n"
"     --ls=<no | first | best> local search type.                           \n"
"\n",
n_of_ants, beam_width, mu, n_samples, sample_percent, max_children, l_rate, det_rate);
}

static void print_version(void)
{
#ifdef VERSION
  printf (" version %s", VERSION);
#endif
#ifdef MARCH
  printf (" (optimised for %s)", MARCH);
#endif
#if DEBUG > 0
  printf (" [DEBUG = %d]", DEBUG);
#endif
}

static void version(void)
{
  printf ("%s", program_invocation_short_name);
  print_version();
  printf ("\n  Problem: ");
  Solution::print_compile_parameters();
  printf("\n\n"
"Copyright (C) 2008\n"
"Christian Blum (christian.blum@ehu.es) and\n"
"Manuel Lopez-Ibanez (manuel.lopez-ibanez@manchester.ac.uk)\n"
"\n"
"This is free software, and you are welcome to redistribute it under certain\n"
"conditions.  See the GNU General Public License for details. There is NO   \n"
"warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n"
"\n"        );
}

static void read_parameters(int argc, char **argv)
{
  for (int iarg = 1; iarg < argc; iarg++)   {

    if (strequal (argv[iarg],"-i")
        || strequal (argv[iarg],"--input")) {
      input_filename = argv[++iarg];
    }
    else if (strequal (argv[iarg],"-T")
        || strequal (argv[iarg],"--trace")) {
      trace_filename = argv[++iarg];
    }
    else if (strequal (argv[iarg],"-v")
             || strequal (argv[iarg],"--version")) {
      version ();
      exit (0);
    }
    else if (strequal (argv[iarg],"-h")
             || strequal (argv[iarg],"--help")) {
      usage ();
      exit (0);
    }
    else if (strequal (argv[iarg],"-t")
             || strequal (argv[iarg],"--time")) {
      time_limit = atof (argv[++iarg]);
    }
    else if (strequal(argv[iarg],"--iterations")
             || strequal(argv[iarg],"-n")) {
      n_of_iter = atoi (argv[++iarg]);
    }
    else if (strequal (argv[iarg],"--trials")
             || strequal (argv[iarg],"-r")) {
      n_of_trials=atoi(argv[++iarg]);
    }
    else if (strequal (argv[iarg], "-a")
             || strequal (argv[iarg], "--ants")) {
      n_of_ants = atoi (argv[++iarg]);
    }
    else if (strequal (argv[iarg], "-s")
             || strequal (argv[iarg], "--seed")) {
      random_seed = atoi (argv[++iarg]);
    }
    else if (strequal(argv[iarg],"--beamwidth")
             || strequal (argv[iarg],"-b")) {
      beam_width = atoi (argv[++iarg]);
    }
    else if (strequal (argv[iarg],"-m")
             or strequal (argv[iarg],"--mu")) {
      mu = atof(argv[++iarg]);
    }
    else if (strequal (argv[iarg], "--maxchild")) {
      max_children = atoi(argv[++iarg]);
    }
    else if (strequal (argv[iarg],"-S")
             || strequal (argv[iarg],"--samples")) {
      n_samples = atoi(argv[++iarg]);
    }
    else if (strequal (argv[iarg],"--sample-rate")) {
      sample_percent = atoi(argv[++iarg]);
      if (sample_percent < 0 or sample_percent > 100) {
        printf ("error: --sample-rate must be within [0, 100]\n");
        exit (1);
      }
    }
    else if (strequal (argv[iarg],"-w")
             || strequal (argv[iarg],"--weights")) {
      if (!Solution::set_heuristic_weights (argv[++iarg])) {
        printf ("error: invalid value for --weights\n");
        exit (1);
      }
    }
    else if (strequal (argv[iarg],"--lrate")
             or strequal (argv[iarg],"-lrate")) {
      l_rate = atof(argv[++iarg]);
    }
    else if (strequal (argv[iarg],"--detrate")
             or strequal (argv[iarg],"-detrate")) {
      det_rate = atof(argv[++iarg]);
    }
    else if (strequal (argv[iarg],"--ls=no")
             or strequal (argv[iarg],"-ls=no")) {
      Solution::localsearch_type = LOCALSEARCH_NONE;
    }
    else if (strequal (argv[iarg],"--ls=best")
             or strequal (argv[iarg],"-ls=best")) {
      Solution::localsearch_type = LOCALSEARCH_BEST;
    }
    else if (strequal (argv[iarg],"--ls=first")
             or strequal (argv[iarg],"-ls=first")) {
      Solution::localsearch_type = LOCALSEARCH_FIRST;
    }
    else {
      printf ("error: unknown parameter: %s\n", argv[iarg]);
      printf ("use --help for usage.\n");
      exit (1);
    }
  }

  if (input_filename.empty()) {
    printf ("error: no input file given (use parameter %s|%s).\n",
            "-i", "--input");
    exit(1);
  }

  if (trace_filename.empty()) 
    trace_stream = stderr;
  else if (NULL == (trace_stream = fopen (trace_filename.c_str(), "w"))) {
    printf ("error: trace file %s cannot be opened", trace_filename.c_str());
    exit (1);
  }

  if (time_limit == DBL_MAX && n_of_iter == INT_MAX) {
    printf ("error: no time limit or number of interations given."
            " Please specify:\n\n"
            " * a time limit in seconds (e.g., --time 20), or\n"
            " * an iteration limit (e.g., -maxiter 1000), or\n"
            " * both.\n");
    exit(1);
  }
}

static void 
print_trace_header (void)
{
  fprintf (trace_stream, "# Trial Iteration     Cost  Cviols     Time"
           "  %8s  %8s\n", "TimeLS", "TimeSampling");
}
static void 
print_trace (Solution *s, int trial_counter, int iter, double time_taken)
{
  fprintf (trace_stream, "%7d %9d %8.2f  %6d  %8.1f  %8.1f  %8.1f\n", 
           trial_counter, iter,
           double(s->cost()), s->constraint_violations(), time_taken,
           time_localsearch, Ant::time_sampling);
  //  s->print_one_line(trace_stream);
}

static void
trial_begin (int trial_counter)
{
  printf ("# begin try %d\n", trial_counter);
  print_trace_header();
}

static void
trial_end (int trial_counter, int best_iter, double best_time, 
           int total_iter, double total_time)
{
  printf("%.2f\t%.1f\t", double(best_so_far->cost()), best_time);
  best_so_far->print_one_line();
  printf("#end try %d"
         ", best_iterations = %d, best_time = %.1f"
         ", evaluations = %u, iterations = %d, total_time = %.1f"
         ", Time_init = %.1f, Time_ls = %.1f, Time_sampling = %.1f"
         "\n",
         trial_counter,
         best_iter, best_time,
         Solution::evaluations, total_iter, total_time,
         time_init, time_localsearch, Ant::time_sampling);
}




static void
UpdatePheromoneValues(bool bs_update, double cf)
{
  /* i_weight, r_weight, g_weight are the weights of influence
     for updating the pheromone values they are set depending on
     the convergence factor cf */
  double i_weight = 0.0;
  double r_weight = 0.0;
  double g_weight = 0.0;

  if (bs_update) {
    // if bs_update = TRUE we use the best_so_far solution for
    // updating the pheromone values
    i_weight = 0.0;
    r_weight = 0.0;
    g_weight = 1.0;
  }
  else {
    if (cf < 0.4) {
      i_weight = 1.0;
      r_weight = 0.0;
      g_weight = 0.0;
    }
    else if (cf < 0.6) {
      i_weight = 2.0 / 3.0;
      r_weight = 1.0 / 3.0;
      g_weight = 0.0;
    }
    else if (cf < 0.8) {
      i_weight = 1.0 / 3.0;
      r_weight = 2.0 / 3.0;
      g_weight = 0.0;
    }
    else {
      i_weight = 0.0;
      r_weight = 1.0;
      g_weight = 0.0;
    }
  }
  /* We specifiy vector d, and then we update the pheromone
     vector towards vector d depending on the learning rate
     l_rate */

  //cout << "before update" << endl;
  int n = Solution::n;

  double d[n][n];
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      d[i][j] = 0.0;
    }
  }

  vector<int> &ib = iteration_best->permutation;
  vector<int> &rb = restart_best->permutation;
  vector<int> &bf = best_so_far->permutation;

  for (int i = 1; i < n; i++) {
    d[ ib[i-1] ][ ib[i] ] += i_weight;
    d[ rb[i-1] ][ rb[i] ] += r_weight;
    d[ bf[i-1] ][ bf[i] ] += g_weight;
  }

  vector<vector<double> > &ph = Ant::pheromone;

  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      ph[i][j] += l_rate * (d[i][j] - ph[i][j]);
      if (ph[i][j] > tau_max) {
        ph[i][j] = tau_max;
      }
      if (ph[i][j] < tau_min) {
        ph[i][j] = tau_min;
      }
    }
  }
}

/* The method computeConvergenceFactor computes the convergence factor
   cf, which gives an indication about the current state of the system
   in terms of its convergence */

static double computeConvergenceFactor() {

  double ret_val = 0.0;
  int n = Solution::n;
  int count = n * n;

  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      //      count++;
      ret_val = ret_val + max (tau_max - Ant::pheromone[i][j],
                               Ant::pheromone[i][j] - tau_min);
    }
  }
  ret_val = ret_val / (count * (tau_max - tau_min));
  ret_val = (ret_val - 0.5) * 2.0;
  return ret_val;
}

void check_valid (Solution *s, string ok, string fail)
{
  if (s->check_solution()) {
    cerr << ok << endl;
    s->print_verbose (stderr);
  } else {
    cerr << fail << endl;
    s->print_verbose (stderr);
  }
}

static void print_commandline (int argc, char *argv[])
{
  printf ("#");
  for (int c = 0; c < argc; ++c)
    printf (" %s", argv[c]);
}

static void print_parameters (int argc, char *argv[])
{
  printf ("# Beam-ACO ");
  printf ("%s", program_invocation_short_name);
  print_version ();
  printf ("\n#\n");
  print_commandline (argc, argv);
  printf ("\n#\n");

  Solution::print_parameters ("#");

  printf ("#\n");
  printf ("# number trials : %d\n", n_of_trials);
  printf ("# number iterations : %d\n", n_of_iter);
  printf ("# time limit : %g\n", time_limit);
  printf ("# seed : %u\n", random_seed);

  printf ("#\n");

  printf ("# number of ants : %d\n", n_of_ants);
  printf ("# learning rate : %g\n", l_rate);
  printf ("# determinism rate : %g\n", det_rate);
  printf ("# heuristic type : %s\n", Solution::get_heuristic_type().c_str());
  printf ("# localsearch : %s\n", Solution::get_localsearch_type().c_str());
  printf ("#\n");

  printf ("# beam width : %d\n", beam_width);
  printf ("# mu : %g\n", mu);
  printf ("# maximum children : %d\n", max_children);
  printf ("# stochastic samples : %d\n", n_samples);
  printf ("# sampling rate : %d (%d%%)\n", sample_rate, sample_percent);
  printf ("#\n");
  printf ("\n");
}

#define update_best_so_far()                                    \
do {                                                            \
  delete best_so_far;                                           \
  best_so_far = iteration_best->clone();                        \
  time_taken = timer.elapsed_time_virtual();                    \
                                                                \
  print_trace (best_so_far, trial_counter, iter, time_taken);   \
                                                                \
  DEBUG2 (check_valid (best_so_far, "best_so_far is valid",     \
               "best_so_far is NOT valid"));                    \
                                                                \
  results[trial_counter-1] = best_so_far->cost();               \
  viols[trial_counter-1] = best_so_far->constraint_violations();\
  times_best_found[trial_counter-1] = time_taken;               \
  iter_best_found[trial_counter-1] = iter;                      \
} while(0)

/* 'main' is the main body of the program */

int main( int argc, char **argv )
{
  double trial_time;

  // upon declaration of a variable of type 'Timer' the time is running ...
  Timer timer;
  
  // a variable that is involved in initializing the random generator
  random_seed = (unsigned) time(NULL);

  read_parameters (argc,argv);

  cout.precision (10);

  // rnd: a random generator
  Random rnd(random_seed);
  // initialization of the random generator
  rnd.next();
  // rnd = new Random (random_seed);
  // rnd->next();

  Ant::Init(input_filename, &rnd);

  int to_choose = int(double(beam_width) * mu);
  sample_rate = int((double(sample_percent) * (Solution::n - 1) / 100.0) + 0.5) + 1;

  //  fprintf (stderr, "too_choose = %d, sample_rate = %d\n",
  //           to_choose, sample_rate);

  print_parameters (argc, argv);

  /* The following variables are for collecting statistics on several
     trials.  */
  Solution* best = NULL;
  vector<double> results;
  vector<double> viols;
  vector<double> times_best_found;
  vector<int> iter_best_found;

  fprintf (trace_stream, "# Initialization Time %g\n", 
           timer.elapsed_time_virtual());

  /* The following for loop is for controlling the number of trials as
     specified by command line parameters.  */
  for (int trial_counter = 1; trial_counter <= n_of_trials; trial_counter++) {

    trial_begin (trial_counter);

    timer.reset();

    // 'iter' is the iteration counter
    int iter = 1;

    /* if the three solutions that are used for updating the pheromone
       values are initialized by a previous trial, we delete them */
    if (best_so_far != NULL) {
      delete best_so_far;
    }
    best_so_far = NULL;

    if (restart_best != NULL) {
      delete restart_best;
    }
    restart_best = NULL;

    /* for every trial we reinitialize the pheromone values to 0.5
       each */
    if (trial_counter == 1) {
      Ant::initUniformPheromoneValues();
    }
    else {
      Ant::resetUniformPheromoneValues();
    }

    /* the following four variable are for controlling the update and
       the restart of the algorithm cf is the convergence factor
       bs_update regulates the use of the best_so_far solution for
       updating program_stop controls the termination of a trial
       restart controls the restart mechanism of the algorithm
       time_taken is a variable for keeping the CPU times when
       checked */
    bool bs_update = false;
    bool restart = false;
    time_taken = 0.0;
    time_localsearch = 0.0;
    time_init = 0.0;
    Ant::time_sampling = 0.0;
    Solution::evaluations = 0;

    /* this is the main loop of the algorithm. At each iteration ants
       produce a solution each and the pheromone values are
       updated. */
    trial_time = timer.elapsed_time_virtual();
    time_init = trial_time;

    while (trial_time < time_limit
           && iter <= n_of_iter) {

      if (iteration_best != NULL) {
        delete iteration_best;
      }
      iteration_best = NULL;
      double avg_cost = 0.0;
      double avg_viols = 0.0;

      // our ant constructs a number of <n_of_ants> solutions
      for (int i = 0; i < n_of_ants; i++) {
        Ant ant;
        Solution* newSol = NULL;
        if (beam_width > 1) {
          newSol = ant.beam_construct (det_rate, beam_width,
                                       max_children,
                                       to_choose,
                                       n_samples, sample_rate);
        } else {
	  newSol = ant.construct (det_rate);
        }

        if (Solution::localsearch_type) {
          double time_localsearch_stop = timer.elapsed_time_virtual ();
          Solution *lsSol = newSol->localsearch();
          //printf ("newSol:"); newSol->print_one_line();
          //printf ("lsSol :"); lsSol->print_one_line();
          while (lsSol->better_than (newSol)) {
            delete newSol;
            newSol = lsSol;
            lsSol = newSol->localsearch();
            //printf ("lsSol :"); lsSol->print_one_line();
            // FIXME: Make localsearch_2opt_first work with asymmetric instances
            if (Solution::is_symmetric) {
                do {
                    Solution *lsSol2 = lsSol->localsearch_2opt_first();
                    if (!lsSol2->better_than(lsSol)) {
                        delete lsSol2;
                        break;
                    }
                    delete lsSol;
                    lsSol = lsSol2;
                } while(true);
            }
          }
          delete newSol;
          newSol = lsSol;
          time_localsearch += timer.elapsed_time_virtual () - time_localsearch_stop;
        }

        avg_cost = avg_cost + newSol->cost();
        avg_viols = avg_viols + newSol->constraint_violations();
        if (iteration_best == NULL) {
          iteration_best = newSol;
        }
        else if (newSol->better_than (iteration_best)) {
          delete iteration_best;
          iteration_best = newSol;
        }
        else {
          delete newSol;
        }
      }

      avg_cost = avg_cost / double(n_of_ants);
      avg_viols = avg_viols / double(n_of_ants);

      if (iter == 1) {
	// if we are in the first iteration then we can initialize all
	// the variables
	if (best_so_far != NULL) {
	  delete best_so_far;
	}
        best_so_far = iteration_best->clone();
	if (restart_best != NULL) {
	  delete restart_best;
	}
	restart_best = iteration_best->clone();
	time_taken = timer.elapsed_time (Timer::VIRTUAL);
	results.push_back (best_so_far->cost());
        viols.push_back (best_so_far->constraint_violations());
	times_best_found.push_back (time_taken);
	iter_best_found.push_back (iter);

        print_trace (best_so_far, trial_counter, iter, time_taken);

        DEBUG2 (check_valid (best_so_far, "best_so_far is valid",
                             "best_so_far is NOT valid"));
      }
      else if (restart) {
        // if this is the first iteration after a restart, then we do
        // the following:
        restart = false;
        delete restart_best;
        restart_best = iteration_best->clone();

        if (iteration_best->better_than(best_so_far)) {
          update_best_so_far ();
        }
      }
      else {
        if (iteration_best->better_than (restart_best)) {
          delete restart_best;
          restart_best = iteration_best->clone();
        }

        if (iteration_best->better_than (best_so_far)) {
          update_best_so_far();
        }
      }

      if (best == NULL) {
        best = best_so_far->clone();
      }
      else if (best_so_far->better_than(best)) {
        delete best;
        best = best_so_far->clone();
      }

      // computation of the convergence factor
      double cf = computeConvergenceFactor();
      DEBUG2(cerr << "cf: " << cf << endl);

      /* if the best_so_far solution was used for updating the
         pheromone values and the convergence factor is greater than
         0.99 we do a restart ... */
      if (bs_update && (cf > 0.99)) {
	bs_update = false;
	restart = true;
	Ant::resetUniformPheromoneValues();
      }
      else {
        /* ... otherwise: if convergence factor is greater than 0.99
           we use the best_so_far solution from now on for updating */
        if (cf > 0.99)
          bs_update = true;

        UpdatePheromoneValues (bs_update, cf);
      }

      iter = iter + 1;
      trial_time = timer.elapsed_time_virtual();
    }

    trial_end (trial_counter, iter_best_found[trial_counter-1],
               times_best_found[trial_counter-1], iter, trial_time);
  }

  /* The following lines are for writing the statistics about the
     experiments onto the screen

     1) the best solution found in all the trials
     2) the average of the best solutions found in all the trials
     3) the standard deviation of the average in 2)
     4) the average CPU at which the best solutions of the trials were found
     5) the standard deviation of the average in 4)
  */

  double r_mean = 0.0;
  double v_mean = 0.0;
  double t_mean = 0.0;
  for (size_t i = 0; i < results.size(); i++) {
    r_mean = r_mean + results[i];
    v_mean = v_mean + viols[i];
    t_mean = t_mean + times_best_found[i];
  }
  r_mean = r_mean / double(results.size());
  v_mean = v_mean / double(viols.size());
  t_mean = t_mean / double(times_best_found.size());
  double rsd = 0.0;
  double vsd = 0.0;
  double tsd = 0.0;
  for (size_t i = 0; i < results.size(); i++) {
    rsd = rsd + pow(results[i] - r_mean, 2.0);
    vsd = vsd + pow(viols[i] - v_mean, 2.0);
    tsd = tsd + pow(times_best_found[i] - t_mean, 2.0);
  }
  rsd = rsd / (double(results.size()) - 1.0);
  if (rsd > 0.0) {
    rsd = sqrt(rsd);
  }
  vsd = vsd / (double(viols.size())-1.0);
  if (vsd > 0.0) {
    vsd = sqrt(vsd);
  }
  tsd = tsd / (double(times_best_found.size())-1.0);
  if (tsd > 0.0) {
    tsd = sqrt(tsd);
  }
  printf("# statistics\t(%g,%d)\t(%f,%f)\t(%f,%f)\t%f\t%f\n",
         double(best->cost()), best->constraint_violations(),
         r_mean, v_mean, rsd, vsd, t_mean, tsd);

  delete best;

  return 0;
}

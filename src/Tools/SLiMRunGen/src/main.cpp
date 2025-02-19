///////////////////////////////////////////////////////////////////////////////////////
// This program autogenerates R and PBS scripts for running SLiM on a super computer //
///////////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <random>
#include "getopt.h"
#include "generators.cpp"
#include "main.hpp"
#include "string.h"

#define no_arg 0
#define arg_required 1
#define arg_optional 2

using std::vector;
using std::string;


    // struct of long names for options

    const struct option longopts[] =
    {
        { "destination",    no_argument,        0,  'd' },
        { "nimrod",         required_argument,  0,  'n' },
        { "help",           no_argument,        0,  'h' },
        { "verbose",        no_argument,        0,  'v' },
        { "top",            optional_argument,  0,  't' },
        { "slim-pars",      required_argument,  0,  'p' },
        { "nodes",          required_argument,  0,  'o' },
        { "jobname",        required_argument,  0,  'N' },
        { "jobarray",       required_argument,  0,  'J' },
        { "walltime",       required_argument,  0,  'w' },
        { "ncpus",          required_argument,  0,  'c' },
        { "mem",            required_argument,  0,  'm' },
        { "R-only",         no_argument,        0,  'R' },
        { "PBS-only",       no_argument,        0,  'P' },
        { "combo-dir",      required_argument,  0,  'l' },
        { "seed-dir",       required_argument,  0,  'S' },
        {0,0,0,0}
    };


void doHelp(char* appname) {
    std::fprintf(stdout,
    "SLiM Runner: A tool for generating HPC files for a SLiM job.\n"
    "\n"
    "This program generates .PBS and .R files to run SLiM in parallel on UQ's Tinaroo HPC.\n"
    "Usage: %s [OPTION]...\n"
    "Example: %s -h\n"
    "\n"
    "-h             Print this help manual.\n"
    "\n"
    "-N NAME        Provide a job name for your HPC job.\n"
    "\n"
    "-v             Turn on verbose mode.\n"    
    "\n"
    "-J '1-N'       Sets the job to a job array using 1-N nodes\n"
    "               Example: -J='1-3' OR -J'1-3'\n"
    "\n"
    "-d FILEPATH    Specify a filepath and name for the generated scripts. Defaults to ./slim_job.\n"
    "               Example: -d ~/Desktop/slim_job\n"
    "\n"
    "-w HH:MM:SS    Specify a walltime for your job in hours, minutes seconds.\n"
    "               -w '10:00:00'\n"
    "\n"
    "-c N           Specify the number of cores to use per node.\n"
    "\n"
    "-m N           Specify the amount of memory to use per node in GB.\n"
    "               Example: -m '50\n"
    "\n"
    "-p LIST        Provide a list of SLiM parameters to vary, delimited by commas.\n"
    "               Example: -p \"nloci,Ne,param1,param2\"\n"
    "\n"
    "-n             Specify if you would like to generate a Nimrod script.\n"
    "\n"
    "-o N           Specify the number of nodes to use in your Nimrod script.\n"
    "               Example: -o 10\n"
    "\n"
    "-R             Specify if you would like to only generate a .R file.\n"
    "\n"
    "-P             Specify if you would like to only generate a .PBS file.\n"
    "\n"
    "-l             Specify the filepath of the parameter combos file.\n"
    "               Example: -l ~/combos.csv\n"
    "\n"
    "-S             Specify the filepath of the seeds file.\n"
    "               Example: -S ~/Seeds.csv\n"
    "\n",
    appname,
    appname
    );
}

int main(int argc, char* argv[]) {

   const struct option voptions[] = 
    {
        { "jobname",        required_argument,  0,  'N' },
        { "filename",       required_argument,  0,  'd' },
        { "jobarray",       required_argument,  0,  'J' },
        { "help",           no_argument,        0,  'h' },
        { "verbose",        no_argument,        0,  'v' },
        { "walltime",       required_argument,  0,  'w' },
        { "nimrod",         optional_argument,  0,  'n' },
        { "nodes",          required_argument,  0,  'o' },
        { "cores",          required_argument,  0,  'c' },
        { "memory",         required_argument,  0,  'm' },
        { "parameters",     required_argument,  0,  'p' },
        { "R-Only",         no_argument,        0,  'R' },
        { "PBS-Only",       no_argument,        0,  'P' },
        { "combo-dir",      required_argument,  0,  'l' },
        { "seed-dir",       required_argument,  0,  'S' },
        {0,0,0,0}
    };

    int optionindex = 0;
    int options;
    FileGenerator fileinit; // Initialiser for PBSGen and RGen


    while (options != -1) {


        options = getopt_long(argc, argv, "N:d:hvJ:w:nc:m:s:RPS:l:", voptions, &optionindex);

        switch (options) {
            case 'N':
                fileinit._jobname = fileinit.NameWrap(optarg); // job name in PBS script, -N
                continue;

            case 'd':
                fileinit._filename = optarg; // filepath for output
                continue;

            case 'h':
                doHelp(argv[0]);
                return 1;

            case 'v':
                fileinit._verbose = true; // verbose mode
                continue;

            case 'J':
                fileinit._jobarray = optarg; // job array range
                continue;

            case 'w':
                fileinit._walltime = optarg; // job walltime in hh:mm:ss
                continue;

            case 'n':
            {
                fileinit._nimrod = true; // nimrod yes or no
                fileinit._comboSize = strlen(optarg) ? std::stoi(optarg) : 100; // set combosize to the argument if it is given
                continue;
            }
            case 'c':
                fileinit._cores = std::stoi(optarg); // how many cores to use
                continue;

            case 'm':
                fileinit._mem = fileinit.MemG(optarg); // how much memory to use
                continue;

            case 'p':
                fileinit._parameters = optarg; // list of parameters, delimited by commas and encapsulated in ""
                continue;

            case 'o':
                fileinit._nodes = std::stoi(optarg); // how many nodes to use (Nimrod only)
                continue;

            case 'R':
                fileinit._r_only = true; // 
                continue;
                
            case 'P':
                fileinit._pbs_only = true;
                continue;

            case 'l':
                fileinit._combos_dir = optarg; // path to combos file
                continue;
            
            case 'S':
                fileinit._seeds_dir = optarg; // path to seeds file

            case -1:
                break;
            }
        }


    if ( fileinit._pbs_only == true ) {
        PBSGenerator PBS(fileinit);
        PBS.FileGenerate();
        return 0;
    }

    else if ( fileinit._r_only == true ) {
        RGenerator RScript(fileinit);
        RScript.FileGenerate();
        return 0;
    }

    else {
        PBSGenerator PBS(fileinit);
        RGenerator RScript(fileinit);
        PBS.FileGenerate();
        RScript.FileGenerate();
    }


    return 0;
}
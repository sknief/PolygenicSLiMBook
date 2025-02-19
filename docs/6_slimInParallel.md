



# Running SLiM in Parallel

## Overview

Now that we have an understanding of SLiM and a couple of models that might be useful, you're probably wondering: how do we make this useful?
To answer a question, we need multiple simulations with different variable inputs (e.g. different recombination rates), and replicates of each.
The problem here is that the more simulations you need to do, the longer the time required if you were to run them sequentially.
Luckily, SLiM is designed with parallelism in mind. That is, you can have many SLiM simulations going at once, each running on a separate CPU core. 
This can all be done in whatever programming language you like, as long as it has a function to invoke system operating system commands 
(like `system()` in R), and support for multicore processing. For more on parallelism, I'd recommend reading [this crass, but entertaining guide](http://librestats.com/RparallelGuide/), 
particularly the first few sections. This will get you up to speed on the strengths and limitations of parallel processing.

In this section, I'll go over how to run SLiM at the command line in a number of languages, and how to parallelise in each of them. 
I even provide a C++ implementation which is overkill and not necessary at all, but it was fun[^fn4] to program! 

Note that more information (and indeed some of the scripts you will see below are based upon these) is available at the 
[SLiM-Extras repository](https://github.com/MesserLab/SLiM-Extras) under 'sublaunching'. Source code for all of the examples shown 
in this document is available in the `/src/Parallelisation` folder.


## SLiM at the Command Line

To use SLiM from the command line, you simply call the `slim` command, followed by a path to a file. For example:


```bash
slim ~/Desktop/example_script.slim
```

This would run the script 'example_script.slim' with it's set parameters. Say you want to do some replicates. 
To do this you might want to change the random seed, which adjusts state of the random number generator that determines when, 
where, and how mutations, recombination, and mating between certain individuals might happen. You can do this using the `-s` flag:


```bash
slim -s 123 ~/Desktop/example_script.slim
```

This would run the above script with the seed `123`. Note that you shouldn't choose numbers like this, 
but instead use a random number generator to generate random numbers for you. 



Similarly, you can adjust parameters in your model with the `-d` flag, of which you can have as many as you want. 
For example, say you want to change the population size which is defined in your script as a parameter called `Ne`. You could run:


```bash
slim -s 123 -d Ne=1000 ~/Desktop/example_script.slim
```

Here, we've set Ne to be 1000, so `slim` will run the script with 1000 individuals and a seed of 123.

:::: {.extrabox data-latex=""}
::: {.center data-latex=""}
**Box 6.2.1**
:::
When it comes to random numbers, it comes as little surprise that humans are bad at generating them. 
We have biases towards certain numbers by conscious (or unconscious!) recognition of those numbers. 
But it turns out that computers have just as hard a time as we do! Computers are designed to be deterministic, 
so generating random numbers can be quite difficult. To do it, various algorithms have been written to generate 
numbers from some distribution, fed with a random 'seed' to spit out that number. This seed is typically a signed 
32-bit integer (of the range -2,147,483,648 to 2,147,483,648), but realistically any size or type can be used 
(and 64-bit integers are becoming increasingly common as the range of possible values is massive). 

But how do you generate a seed? 
With another random number generator. Which must have come from some deterministic process. 
So no matter how many generators you have generating seeds for your random output, you can always go back to a 
deterministic value that seeded it all. It isn't ever truly random, just pseudo-random. Of course, certain trickery 
has allowed for more complex randomness: some of the latest has to do with quantum mechanics of a laser reflecting 
or absorbing photons and treating the output of the number absorbed vs reflected as a random number, which is 
theoretically truly random. But do you really need a laser to seed a pseudo-random process? Or a system of lava 
lamps (a la [CloudFlare](https://www.youtube.com/watch?v=1cUUfMeOijg))? Probably not. As long as you are blind to 
the original seed that created your random numbers, it might as well be random: the chance that you are going to be 
able to find that original seed is astronomically small ($\frac{1}{2^{63}-1}$ for a signed 64-bit seed). 
`/dev/random`, which we saw in Box 2.1.1, generates random numbers based on environmental noise from your computer 
hardware drivers, which is supposedly close to 'true' randomness. [But is 'true' randomness a thing at all](https://en.wikipedia.org/wiki/Determinism)? 
Is the universe deterministic? What is the universe's seed? If you put the universe's seed into a Minecraft 
world generator, would that world have many diamonds? 
Food for thought.

The point is that you should be careful with generating random numbers and make sure you know that the 
ultimate source of randomness is as close to 'true' random as possible, but if it isn't, as long as it is secure and 
probabilistically close to impossible to crack, it is probably a good enough source of randomness for whatever you are trying to do.
::::


OK, so we can run SLiM via the command line for a single seed or variable combination. Now we could run that command multiple times, 
changing the seed or parameters manually every time we run it, but that's inefficient and makes it difficult to use all your available 
resources on your computer (notably, cores for running separate SLiM instances in parallel). This is where sublaunching 
scripts can come in handy. These enable us to use a pre-written script to vary all the parameters we would like automatically, 
and run many SLiM experiments at once as possible.

As a quick aside, these template scripts I'm providing require a `seeds.csv` file to be in the format of a single column 
with a header called 'Seed'. My SeedGenerator program which I have provided (as part of the "Tools" section of the [main GitHub branch] (https://github.com/nobrien97/PolygenicSLiMBook/tree/main/src/Tools)), 
(along with an install script) will generate these for you. To use SeedGenerator, simply run `seedgen_install.sh`, 
and then `./seedgenerator`. There are a variety of options, listed with the -h or --help flag:


```bash
# Shows the help file with instructions on each of the options
 ../Tools/SeedGenerator/seedgenerator --help
```

```
## Uniformly Distributed Seed Generator
## 
## This program generates a .csv of uniformly distributed 32-bit integers for use as RNG seeds.
## Usage: ../Tools/SeedGenerator/seedgenerator [OPTION]...
## Example: ../Tools/SeedGenerator/seedgenerator -h
## 
## -h             Print this help manual.
## 
## -n N           Generate N random samples. Defaults to 10.
## 
## -v             Turn on verbose mode.
## 
## -t NAME        Choose a header name. Defaults to 'Seed'. Enter nothing to have no header.
##                Example: -t=Number OR -tNumber
## 
## -d FILEPATH    Specify a filepath and name for the generated seeds to be saved. Defaults to ./seeds.csv.
##                Example: -d ~/Desktop/seeds.csv
```

Now onto sublaunching SLiM. We'll start with Bash, which is perhaps the simplest.


## Running SLiM via Bash

The simplest way to run SLiM at the command line is through Bash, the standard command line scripting language of Linux based systems. 
Note that the SLiM Online Workshop - '_Running SLiM from the command line_' tutorial goes over much of what I'll be presenting below.
The Bash syntax is pretty similar to the standard way of running SLiM on the command line, and in fact you'll be using almost identical commands.


```bash
for seed in seeds.csv; do
  echo "Running with seed == " $(seed):
  slim -s $(seed) ~/Desktop/example_script.slim &
  echo 
done
```

Here, we do a simple for loop over the seeds in a file called seeds.csv (generated by Tools/SeedGenerator/seedgenerator). 
For this to work in bash, make sure you have the header disabled (`./seedgenerator -t`)
The `&` character tells Bash to run the slim process as a background task, meaning it is put on an available core. 
This results in SLiM processes running parallelised across multiple cores!

Bash can also be used to parallelise over many parameters, but it quickly becomes difficult to read. For example, 
here is an example script with two parameters and a seed variable:


```bash
for param1 in 0.1 0.2 0.3
  do
  for param2 in "Low" "Medium" "High"
    do
    for seed in seeds.csv
      do
      echo "Seed = " $(seed) " param1 = " $(param1) " param2 = " $(param2):
      slim -s $(seed) -d param1=$(param1) -d param2=$(param2) ~/Desktop/example_script.slim &
      echo
      done
    done
  done

```

While it is also possible to read in files and split columns into different variables, it's much easier to do this in R or Python. 
So for more complex scripts, I would highly suggest using either of those to sublaunch your SLiM jobs. I will provide examples of both below.

## Running SLiM via R

This is how I normally sublaunch SLiM. R has a variety of packages that make it easy to parallelise across cores, 
and because of R's rich feature set and the breadth of user-made libraries, the options are endless when it comes to 
feeding parameters to SLiM, loading output, or integrating with SLiM.


```r
# Parallelisation libraries 

library(foreach)
library(doParallel)
library(future)


seeds <- read.csv("~/Desktop/seeds.csv", header = T)


cl <- makeCluster(future::availableCores())
registerDoParallel(cl)

#Run SLiM

foreach(i=seeds$Seed) %dopar% {
	# Use string manipulation functions to configure the command line args, feeding from a data frame of seeds
	# then run SLiM with system(),
    	slim_out <- system(sprintf("/path/to/slim -s %s ~/Desktop/example_script.slim", 
                                  as.character(i), intern=T))
  }
stopCluster(cl)
```

This script first loads a series of libraries that allow R to run a for loop across multiple cores. 
Each iteration of the for loop is assigned to a free core when it becomes available. We then load in `seeds.csv` as a dataframe, 
and set up a local 'cluster', which basically lets R know how many cores are available on your system that it can use. 
The seeds are then fed into our for loop (`foreach()`, which is implemented specifically to be parallel). For each seed, we run the for loop, 
with the `%dopar%` operator saying that we should do those iterations across as many cores as are available. 
The `system()` command tells the operating system to run a command, given as a string. `sprintf()` creates a string 
from its inputs, with support for variables to be added to the string on the fly. This is done with the `%` symbol 
followed by the type that is being fed to `sprintf()`. For example, `%s`  provides a placeholder for a string variable. 
These variables are listed at the end of the line _in the order that they are mentioned in the `sprintf()` command. 
For example, here `%s`, being the first variable mentioned in the string, is replaced by the first variable mentioned after the 
string ends with the closing \". In this case, we feed SLiM a seed with the `-s` command, replacing `%s` with `as.character(i)`, 
which is the seed given by the for loop. As well as `%s`, there is also `i` and `f` for integer and floating point 
(or double, since R only supports doubles) numbers. 
So why do we feed SLiM a string as a seed instead of an integer? It's to ensure it gets read properly. R only supports signed 32-bit integers, 
which are a fair bit smaller than SLiM's 64-bit integers. Since the idea of randomly choosing seeds is to uniformly sample 
across the entire range of possible values to avoid any kind of correlations, we sample across the range of 64 bit values. 
However, R can't handle numbers so big as integers, so it automatically coerces them to doubles. 
Doubles are stored in computers very differently to integers, and having R treat this number as an integer (or as a float) and then feed it to 
SLiM that way results in unexpected behaviour. I've found it's safest to just treat the seed as a string so no coercion happens -
SLiM automatically will treat that string as an integer when it is loaded anyway. 

Now, the R script is a little more complex at base-level than a Bash script, but I find it much easier to expand upon. 
Say for example we have two parameters as before. We could use nested `foreach` loops just like we did in Bash, but 
it's easier to exploit R's dataframe support to have a dataframe of possible parameter combinations, and then use each 
iteration to choose the right combination. This way, we simply need one level of nesting regardless of how many parameters 
we have: one `foreach` loop for seeds, and a nested one for parameter combinations.

```r
# Create a list of parameters
p <- list()
p$param1 <- c(0.1, 0.2, 0.3)
p$param2 <- c('"Low"', '"Medium"', '"High"') #'" is necessary for SLiM to read them as strings

# Save the list as a data frame with all possible combinations
df.p <- expand.grid(p)

# You can also save df.p as a csv file and import it later, as with seeds: write.csv(df.p, row.names = F)

# Now we can use those data frame rows as inputs for our script

# Parallelisation libraries 

library(foreach)
library(doParallel)
library(future)


seeds <- read.csv("~/Desktop/seeds.csv", header = T)


cl <- makeCluster(future::availableCores())
registerDoParallel(cl)

#Run SLiM
foreach(i=1:nrow(df.p)) %:%
  foreach(j=seeds$Seed) %dopar% {
    	slim_out <- system(sprintf("/path/to/slim -s %s -d param1=%f -d param2=%s  ~/Desktop/example_script.slim", 
                                  as.character(j), df.p[i,]$param1, df.p[i,]$param2, intern=T))
  }
stopCluster(cl)
```

Here we do the exact thing as before: creating a local cluster and running a `foreach` loop. However, in this case we 
nest a second `foreach` loop so we can include both seeds and the parameter combinations. Each `i` in this loop is a 
different row in the `df.p` dataframe of parameter combinations. Each `j` is a different seed. Notice we only ues `%dopar%` 
once, and use that on the innermost `foreach` loop. This means for each parameter combination (`i`), we will parallelise 
across seeds (`j`). This can be extended to as many parameter values as you want, as we simply fill each parameter using 
the `sprintf()` variable-filling functionality as before, this time referencing `df.p` and choosing the appropriate column (parameter value).

## Running SLiM in Python

Running SLiM in Python is similarly straightforward, and due to its plethora of libraries, quite powerful also.
Here's an example


```python
from os import system
from multiprocessing import Pool
from pandas import read_csv

# Open the seeds file

seeds = read_csv(r'../Inputs/seeds.csv')


def slim_call(seed):
    system('slim -s {} ~/Desktop/example_script.slim'.format(seed)) 

# Open a new 'pool' - like makeCluster() in R

cluster = Pool()

# Do an operation on the pool - this is like mclapply() in R
if __name__ == "__main__":
    cluster.map(slim_call, seeds['Seed'].tolist())

cluster.close()
cluster.join()

```

In this script, we iterate over seeds only, using the built-in `os` and `multiprocessing` libraries, and the popular `pandas` library.
To install pandas, run `python -m pip install pandas` in a Terminal and restart Python if you have it open.

We first use the `pandas` function `read_csv()` to load our seeds into a dataframe. From there we create a new pool - this is analogous
to creating a new cluster with `makeCluster()` in R. The default setting as shown creates a pool with all available cores, however using 
`Pool(x)` where x is the number of cores you would like to use. 
We define a function to call slim via the `os.system()` command, and use our Pool's own function `map` to map a list of imputs to our SLiM
function. This will run the function for all values in that vector, and on as many cores available to the Pool.
Using `cluster.close()` followed by `cluster.join()` is good practice, just like closing the cluster in R with `stopCluster()`


Now lets expand this to our list of combinations like in R:


```python
from os import system
from multiprocessing import Pool
from itertools import product
from pandas import read_csv, DataFrame
from joblib import Parallel, delayed

# Open the seeds file as a list

seeds = read_csv(r'../Inputs/seeds.csv')['Seed'].to_list()

# Create a list of parameters and generate unique combinations, then form a list of tuples
# You can also use the pandas.write_csv() function to store this output and read it later like
# we have been doing with seeds

p = {'param1' : [0.1, 0.2, 0.3], 'param2' : ['"Low"', '"Medium"', '"High"']}
keys, values = zip(*p.items())
# https://stackoverflow.com/a/61335465/13586824
combos = [dict(zip(keys, v)) for v in product(*values)]
combos = DataFrame.from_dict(combos)
combos = list(combos.itertuples(index=False, name=None))


# Do an operation on the pool - this is like foreach() in R
# \" is an escaped string, which we need for the command line to be able to feed SLiM string parameters properly

def slim_call(param1, param2):
    for seed in seeds:
        system('/path/to/slim -s {se} -d param1={p1} -d \"param2=\'{p2}\'\" ~/Desktop/example_script.slim'.format(se=seed, p1=param1, p2=param2)) 

# Open a new 'pool' - like makeCluster() in R

cluster = Pool()

if __name__ == "__main__":
    cluster.starmap(slim_call, combos)
cluster.close()
cluster.join()

```

This is a little bit more painful than in R. We construct a list of tuples of every combination of our parameters. We do this by first
constructing a dictionary of the possible values for the parameters. We then use `zip()` to construct an iterator of tuples: basically
a pairing of param1 to param2. Then we get the product of each key's values with the other key's values for all possible combinations in a 
list of dictionaries. From this list of dictionaries, we construct a `pandas` DataFrame, and then convert that to a list of tuples.

Here, we encapsulate our SLiM call in a function which includes a for loop over seeds. We then use the `Pool.starmap()` function to run in 
parallel these SLiM runs across combinations. Indeed, this ordering could also be reversed so that the for loop contains the combinations 
and `starmap()` iterates over seeds. This might be a good idea if the number of combinations is less than the number of seeds, to leverage 
as much parallel power as possible and reduce wasted time.


## Running SLiM via a C++ Program

Before we go on, I should mention that this part is pretty much always overkill. There really isn't much of an overhead from running SLiM in 
parallel via interpreted languages, so there is little reason to use any C-based language for running SLiM. It's clunky and kind of a pain.
However, it is an interesting exercise, and in the cases where you need to eke out every iota of performance during a particularly slow 
simulation, it might be worthwhile.

This program uses an open source header library to read csv files into vectors. That can be found in the /includes/ folder, and also at the 
[original GitHub repository](https://github.com/ben-strasser/fast-cpp-csv-parser). The program uses OpenMP to parallelise a for loop across
multiple cores.


```cpp
#include "includes/csv.h" // https://github.com/awdeorio/csvstream
#include "stdlib.h"
#include <iostream>
#include <utility>
#include <vector>
#include <string>
#include "omp.h"
#include <map>

using std::vector; using std::string;

#define THREAD_NUM 4 //omp_get_thread_num(); // Max CPUs on machine


// Escaped " and ' are so the command line doesn't freak out with quotations in the middle of the line for string input
// Feed in a single row of the combos.csv and a single seed at a time: the parallelised for loop will do this
void runSLiM(std::pair<float, string> combo, string seed) {
    float param1 = combo.first;     // Access the pair's first value, which is param1
    string param2 = combo.second;
    string callLine = "/path/to/slim -s " + seed + " -d param1=" + std::to_string(param1) + " -d \"param2=\'" + param2 + "\'\" ~/Desktop/example_script.slim";
    std::system(callLine.c_str());
}

int main() {
    // Read the seeds and combos
    io::CSVReader<1> seeds("./seeds.csv");
    seeds.read_header(io::ignore_extra_column, "Seed");
    vector<string> vSeeds;
    int64_t curSeed;
    // For each row in the file, fill variables with that row's values
    while (seeds.read_row(curSeed)) {
        vSeeds.emplace_back(std::to_string(curSeed)); // Stick it into a vector of all seeds
    }
    io::CSVReader<2> combos("./combos.csv");
    combos.read_header(io::ignore_extra_column, "param1", "param2");
    vector<std::pair<float, string>> vCombos;
    float curP1;
    string curP2;
    while (combos.read_row(curP1, curP2)) {
        vCombos.emplace_back(curP1, curP2); // Same as above, but we are constructing a vector of pairs, where the pair is param1 and param2
        }
    
    
    // Start of parallel processing code
    omp_set_num_threads(THREAD_NUM); // How many cores to use?
    #pragma omp parallel for collapse(2) // 2 for loops, so collapse those loops into one parallelisable structure
    { 
        for (int i=0; i < vSeeds.size() - 1; ++i) {
            for (int j=0; j < vCombos.size() - 1; ++j) {
                runSLiM(vCombos[j], vSeeds[i]); // run SLiM with a given seed and parameter combination
            }
        }
    }

    return 0;
}
```

Most of this code is just sorting out csv files into structures that we can perform for loops on. We use openMP to parallelise 
a nested for loop structure and automatically dish out seed-parameter combinations to the runSLiM() function out to available cores.
This code isn't particularly nice to look at, nor is it going to work as-is for more than two parameters (`std::pair` will need 
to be another `std::vector`), however it could be expanded upon to be more customisable (e.g. accepting user input for
filepaths). In addition, it could be part of a larger GUI app to launch parallel SLiM jobs in a more user-friendly way (coming soon?).

## Writing SLiM code with parallelism in mind

When it comes to writing your SLiM code, there are a few factors to keep in mind. Perhaps the most important is how you will deal with 
output. Will your models all write into the same output file or separate files? Each option has its pros and cons, so I'll give descriptions
of both.

### Single file output

If you want to write your output into a single file, you will need to ensure that all `writeFile()` calls in your script have the 
`append` flag set to `T`. This way, when writing to the file, SLiM will write a new line character followed by what you are writing
rather than overwriting the file. Be warned that this method isn't completely safe and can introduce you to race conditions: if two simulations
try to write at the same time, which of the two will write? Will one line of results be lost, will they both be glued together and become a pain
to format, or will everything go as planned? This may or may not be a problem. For example, if you are tracking evolution over long periods of time,
a single missed data point isn't going to cause you any problems realistically. However, if your data is very detailed and written multiple times
during a generation, then the chances for files to be written at the same time increases. The lesson really is if you write this way, write as little
as you can to minimise the risk of race conditions becoming a problem. I have written output this way through my Honours, and had no issues with overwriting 
or missing output. This was with 1152 concurrent simulations, with data being written 200 times per run. However, I have encountered one error since then,
where a single row was appended with no newline character to another, likely due to a similar race condition. 

In summary, the chances of problems are low, but never zero[^fn5].

### Multiple file output

If you want to write to multiple files, then you will need to write your output into generated folders or file names based on your seed and parameter
combinations. For this, I recommend defining a 'model index' parameter in SLiM, which is unique for each parameter combination. In fact, this parameter 
will be useful regardless of your output protocol, as we will see later. This can simply be set asthe row number in your parameter combination table. 
For example, in R:


```r
#Run SLiM
foreach(i=1:nrow(df.p)) %:%
  foreach(j=seeds$Seed) %dopar% {
    	slim_out <- system(sprintf("/home/$USER/SLiM/slim -s %s -d param1=%f -d modelindex=%i  ~/Desktop/example_script.slim", as.character(j), df.p[i,]$param1, i, intern=T))
  }
```

Here we set the SLiM constant `modelindex` equal to `i`, which is the row number of the parameter combination dataframe in our for loop.

Now to name the output files, we need to combine the seed and modelindex in our SLiM script and specify a filename, like so:


```cpp
initialize() {
	if (!exists("slimgui")) {
		setCfgParam("seed", getSeed()); // Set the seed as a constant if we've run from the command line
	}
	else {
		setSeed(asInteger(round(runif(1, 1, 2^62 - 1))));
		setCfgParam("seed", getSeed());
		catn("Actual seed: " + asInteger(seed)); // Reset the seed to a more controlled value if we've run from SLiMgui
	}

	setCfgParam("modelindex", 1); // Identifier for the combination of predictors used in latin hypercube: this is the row number in the lscombos.csv file
	
	
	setCfgParam("outputFilepath", './out_' + modelindex + '_' + seed + '_slim.csv'); // Output filename/path

```

The output filename of this simulation will be `out_<modelindex>_<seed>_slim.csv`. Since each simulation is writing to a different file,
there is no chance of the output being overwritten[^fn6]. At the end of your simulations, you can join all these files into one using the 
`cat` bash command:


```bash
cat ./* > slim_out.csv
```
This will grab all files in the selected folder `./` and concatenate them together. Note that if newlines aren't at the bottom
of each file, then this won't work. However, SLiM should automatically do this, and if not you can use the script:

```bash
for file in /path/*.csv
do
    echo "" >> "$file"
done
```
Which will append a newline character to the end of each file[^fn7].


The main problem with this method is writing to many different files creates overhead that will slow your simulation down, and has the potential
to overload the filesystem. This can cause problems for other users if you are on a HPC. My recommendation is for large simulations you should use
one single large file that you constantly append to, whereas smaller runs are probably fine to do using multiple files.


## Footnotes
[^fn4]: https://xkcd.com/612/
[^fn5]: Much like the chances of your cat eating you in your sleep.
[^fn6]: There is still a small chance of `stdout` losing your output if you have a massive amount of I/O that is overloading the filesystem. But just don't do that.
[^fn7]: https://stackoverflow.com/a/31053205/13586824

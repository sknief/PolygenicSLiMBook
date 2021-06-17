



# Introduction

## Preface

The past couple of decades have seen the development of powerful computer systems that have become pervasive 
in every human endeavour. Scientists have long regarded computation as a helpful tool for tackling difficult problems, 
however leaps in computational power have opened up research opportunities that simply did not exist before. 
Such is the case with forward-genetics simulation. Although such simulations have existed since the mid-2000s[^fn1], 
their use was limited due to the computational requirements of forward-time simulations being unmatched by the hardware 
of the time. SLiM changes that, taking advantage of modern computers and an easy-to-learn scripting language that makes 
forward-genetics simulations relatively straightforward. While thorough documentation on introducing biologists to SLiM 
already exists in the form of the [SLiM manual](http://benhaller.com/slim/SLiM_Manual.pdf), for those studying complex, 
polygenic traits, there is a fair amount of trial and error. Unless you have happened across this document, in which I will 
provide a plethora of templates, tutorials, and tips that I have learned through my time experimenting with SLiM, as well 
as provide detail on where to find additional help.


## Overview

In this book, I'll cover:

- Installation of a Linux environment for Windows 10,
    Why Linux?
    WSL: Running a fast Linux VM in Windows
    Installing WSL & Ubuntu 20.04 LTS
    Linux Desktops - pros and cons
    Installing Gnome for Ubuntu
- Installing SLiM (with some fixes for errors I have come across),
    Build tools necessary
    CMake and make (extra box with info on compilation)
    Installing SLiM - with install script
    Additional useful software - R, RStudio, FileZilla, Visual Studio Code
- Using the terminal, basic shortcuts (with hyperlinked video tutorials),
    cd, pwd, mkdir, and creating a custom mkcdir command
    installing and updating software (apt get) - cowsay fortune
    shortcuts (tab, arrow keys, !, etc.)
    Linux filesystem, file descriptors, input/output
    sed, grep, | and < and > 
- Polygenic adaptation crash course - featuring videos, animations, shiny apps
    What is polygenic adaptation?
        How is it studied? Population genetics vs quantitative genetics
    Quantitative genetics models
        Infinitesimal model
        Geometrical model
    Population Genetics theories
        Neutral theory of molecular evolution
        Nearly-neutral theory
    Combining population and quantitative genetics models
        Extreme value theory
    Background selection
    Directional selection vs stabilising selection + balancing selection
    Hard vs soft sweeps
    Neutral evolution: Brownian motion and Uhrbeck-Ornstein
    Linkage
    Adaptability vs Adaptedness: Effect sizes and mutation rates (HoC vs Gaussian)
- Modelling polygenic adaptation in SLiM, 
    What can SLiM offer for studying polygenic adaptation?
    The SLiM online workshop
    Templates: evolution of a single polygenic trait with variable recombination, selection strength, polygenicity, 
    mutability, population size, background selection, and effect size distributions with drift, stabilising, and directional selection options 
    Template 4: Stabilising selection model of two traits with the above parameters, and a level of pleiotropy between them, and configurable fitness impacts for each
- Running SLiM in parallel on your computer,
    Appending output from multiple simultaneous runs
    Keeping track of models in a combined output file
    Seed generator program
    Running SLiM in parallel via bash script - Template Bash script
    Running SLiM in parallel via R (future, doParallel, foreach) - Template R script
    Running SLiM in parallel via Python (joblib) - Template Python script
- Running SLiM on a remote computer (such as a computing cluster or HPC),
    Connecting to a remote computer
    Building SLiM on a remote computer
    Transferring files to a remote computer
    Writing a PBS script to run SLiM - Template Script
    Saving output
- Using Latin Hypercube Sampling to properly sample a range of genetic parameters,
    What is Latin Hypercube Sampling? 3D figure of 3D samples
    Why LHS over Factorial designs?
    LHS in R - DoE.Wrapper: Template R script
    Diagnostics
    Feeding your hypercube combinations as inputs to SLiM - Template R script
- Running SLiM on a larger computing cluster (connecting across multiple nodes),
    Why would you need more than 24 cores?
    Job Array system - Template script
    Embedded Nimrod system - Template script
    Queue times - avoiding problems
    Managing RAM
    Big output - compression etc.
- Using regex to sort data,
    sed, grep for filtering for what you want
- and statistics for SLiM data in R.

- Genome assembly pipelines,
- Statistics for genome assembly data,

Throughout, I'll put additional information in boxes, so those interested can learn more.

:::: {.extrabox data-latex=""}
::: {.center data-latex=""}
**Box 1.2.1**
:::
Boxes look like this!
::::

## Prerequisites

The tutorials in here assume you are at least somewhat familiar with quantitative and population genetics.

## References
[^fn1]: B. Peng, Kimmel M., simuPOP: a forward-time population genetics simulation environment, Bioinformatics, Volume 21, Issue 18, Pages 3686-3687, https://doi.org/10.1093/bioinformatics/bti584
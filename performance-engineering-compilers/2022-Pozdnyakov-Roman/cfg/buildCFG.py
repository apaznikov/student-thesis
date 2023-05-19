# MARK imports

import angr, sys
from angrutils import *

# MARK util functions

def buildCFG(binName, outputFileName):
    project = angr.Project(binName, load_options={'auto_load_libs':False})  # load binary file
    main = project.loader.main_object.get_symbol("main")                    # find main
    startState = project.factory.blank_state(addr=main.rebased_addr)        # create state from main
    cfg = project.analyses.CFGEmulated( fail_fast=True,                     # crrate graph
                                        starts=[main.rebased_addr],
                                        initial_state=startState)
    plot_cfg(   cfg,                                                        # plot graph
                outputFileName,
                asminst=True,
                remove_imports=True,
                remove_path_terminator=True)

# MARK MAIN

def main(args):
    if len(args) == 1:
        binName = args[0]
        buildCFG(binName,
                 binName)
    elif len(args) > 1:
        binName = args[0]
        outName = args[1]
        buildCFG(binName,
                 outName)

if __name__ == "__main__":
   main(sys.argv[1:])

//
// Created by Eric Irrgang on 11/23/18.
//

#include <iostream>

#include "gromacs/mdtypes/inputrec.h"
#include "gromacs/topology/topology.h"
#include "gromacs/mdtypes/state.h"
#include "gromacs/fileio/oenv.h"
#include "gromacs/fileio/tpxio.h"
#include "gromacs/fileio/trxio.h"
#include "gromacs/options/timeunitmanager.h"
#include "gromacs/utility/cstringutil.h"
#include "gromacs/utility/programcontext.h"
//#include "gmxapicompat/gromacs2019.h"

int main()
{
    t_inputrec irInstance;
    t_inputrec* irRef = &irInstance;

//    gmxapi_compat::GmxMdParams params;
//    for (const auto& key : params.keys())
//    {
//        std::cout << key << "\n";
//    }
//    std::cout << std::flush;

    return 0;
}

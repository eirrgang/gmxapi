/* -*- mode: c; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; c-file-style: "stroustrup"; -*-
 *
 * 
 *                This source code is part of
 * 
 *                 G   R   O   M   A   C   S
 * 
 *          GROningen MAchine for Chemical Simulations
 * 
 * Written by David van der Spoel, Erik Lindahl, Berk Hess, and others.
 * Copyright (c) 1991-2000, University of Groningen, The Netherlands.
 * Copyright (c) 2001-2010, The GROMACS development team,
 * check out http://www.gromacs.org for more information.

 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * If you want to redistribute modifications, please consider that
 * scientific software is very special. Version control is crucial -
 * bugs must be traceable. We will be happy to consider code for
 * inclusion in the official distribution, but derived work must not
 * be called official GROMACS. Details are found in the README & COPYING
 * files - if they are missing, get the official version at www.gromacs.org.
 * 
 * To help us fund GROMACS development, we humbly ask that you cite
 * the papers on the package - you can find them in the top README file.
 * 
 * For more info, check our website at http://www.gromacs.org
 * 
 * And Hey:
 * Gallium Rubidium Oxygen Manganese Argon Carbon Silicon
 */

/*
 * Note that parts of this source code originate from the Simtk release 
 * of OpenMM accelerated Gromacs, for more details see: 
 * https://simtk.org/project/xml/downloads.xml?group_id=161#package_id600
 */

#include <cmath>
#include <set>
#include <iostream>
#include <sstream>
#include <fstream>
#include <map>
#include <vector>
#include <cctype>
#include <algorithm>

using namespace std;

#include "OpenMM.h"

#include "gmx_fatal.h"
#include "typedefs.h"
#include "mdrun.h"
#include "physics.h"
#include "string2.h"
#include "gmx_gpu_utils.h"
#include "mtop_util.h"
#include "warninp.h"

#include "openmm_wrapper.h"

using namespace OpenMM;

/*! \cond */
#define MEM_ERR_MSG(str) \
    "The %s-simulation GPU memory test detected errors. As memory errors would cause incorrect " \
    "simulation results, gromacs has aborted execution.\n Make sure that your GPU's memory is not " \
    "overclocked and that the device is properly cooled.\n", (str)
/*! \endcond */

/*! 
 * \brief Convert string to integer type.
 * \param[in]  s    String to convert from.
 * \param[in]  f    Basefield format flag that takes any of the following I/O
 *                  manipulators: dec, hex, oct.
 * \param[out] t    Destination variable to convert to.
 */
template <class T>
static bool from_string(T& t, const string& s, ios_base& (*f)(ios_base&))
{
    istringstream iss(s);
    return !(iss >> f >> t).fail();
}

/*!
 * \brief Split string around a given delimiter.
 * \param[in] s      String to split.
 * \param[in] delim  Delimiter character.
 * \returns          Vector of strings found in \p s.
 */
static vector<string> split(const string &s, char delim)
{
    vector<string> elems;
    stringstream ss(s);
    string item;
    while (getline(ss, item, delim))
    {
        if (item.length() != 0)
            elems.push_back(item);
    }
    return elems;
}

/*!
 * \brief Split a string of the form "option=value" into "option" and "value" strings.
 * This string corresponds to one option and the associated value from the option list 
 * in the mdrun -device argument.
 *
 * \param[in]  s    A string containing an "option=value" pair that needs to be split up.
 * \param[out] opt  The name of the option.
 * \param[out] val  Value of the option. 
 */
static void splitOptionValue(const string &s, string &opt, string &val)
{
    size_t eqPos = s.find('=');
    if (eqPos != string::npos)
    {
        opt = s.substr(0, eqPos);
        if (eqPos != s.length())  val = s.substr(eqPos+1);
    }
}

/*!
 * \brief Compare two strings ignoring case.
 * This function is in fact a wrapper around the gromacs function gmx_strncasecmp().
 * \param[in] s1 String. 
 * \param[in] s2 String.
 * \returns      Similarly to the C function strncasecmp(), the return value is an  
                 integer less than, equal to, or greater than 0 if \p s1 less than, 
                 identical to, or greater than \p s2.
 */
static bool isStringEqNCase(const string s1, const string s2)
{
    return (gmx_strncasecmp(s1.c_str(), s2.c_str(), max(s1.length(), s2.length())) == 0);
}

/*!
 * \brief Convert string to upper case.
 *
 * \param[in]  s    String to convert to uppercase.
 * \returns         The given string converted to uppercase.
 */
static string toUpper(const string &s)
{
    string stmp(s);
    std::transform(stmp.begin(), stmp.end(), stmp.begin(), static_cast < int(*)(int) > (toupper));
    return stmp;
}

/*! 
  \name Sizes of constant device option arrays GmxOpenMMPlatformOptions#platforms, 
  GmxOpenMMPlatformOptions#memtests, GmxOpenMMPlatformOptions#deviceid, 
  GmxOpenMMPlatformOptions#force_dev.  */
/* {@ */
#define SIZEOF_PLATFORMS    1  // 2
#define SIZEOF_MEMTESTS     3 
#define SIZEOF_DEVICEIDS    1 
#define SIZEOF_FORCE_DEV    2 
/* @} */

/*! Possible platform options in the mdrun -device option. */
static const char *devOptStrings[] = { "platform", "deviceid", "memtest", "force-device" }; 

/*! Enumerated platform options in the mdrun -device option. */
enum devOpt
{
    PLATFORM     = 0,
    DEVICEID     = 1,
    MEMTEST      = 2,
    FORCE_DEVICE = 3
};

/*!
 * \brief Class to extract and manage the platform options in the mdrun -device option.
 * 
 */
class GmxOpenMMPlatformOptions
{
public:
    GmxOpenMMPlatformOptions(const char *opt);
    ~GmxOpenMMPlatformOptions() { options.clear(); }
    string getOptionValue(const string &opt);
    void remOption(const string &opt);
    void print();
private:
    void setOption(const string &opt, const string &val);

    map<string, string> options; /*!< Data structure to store the option (name, value) pairs. */

    static const char * const platforms[SIZEOF_PLATFORMS];  /*!< Available OpenMM platforms; size #SIZEOF_PLATFORMS */
    static const char * const memtests[SIZEOF_MEMTESTS];    /*!< Available types of memory tests, also valid 
                                                                 any positive integer >=15; size #SIZEOF_MEMTESTS */
    static const char * const deviceid[SIZEOF_DEVICEIDS];   /*!< Possible values for deviceid option; 
                                                                 also valid any positive integer; size #SIZEOF_DEVICEIDS */
    static const char * const force_dev[SIZEOF_FORCE_DEV];  /*!< Possible values for for force-device option; 
                                                                 size #SIZEOF_FORCE_DEV */
};

const char * const GmxOpenMMPlatformOptions::platforms[SIZEOF_PLATFORMS]
                    = {"CUDA"};
                    //= { "Reference", "CUDA" /*,"OpenCL"*/ };
const char * const GmxOpenMMPlatformOptions::memtests[SIZEOF_MEMTESTS]
                    = { "15", "full", "off" };
const char * const GmxOpenMMPlatformOptions::deviceid[SIZEOF_DEVICEIDS]
                    = { "0" };
const char * const GmxOpenMMPlatformOptions::force_dev[SIZEOF_FORCE_DEV]
                    = { "no", "yes" };

/*!
 * \brief Contructor.
 * Takes the option list, parses it, checks the options and their values for validity.
 * When certain options are not provided by the user, as default value the first item  
 * of the respective constant array is taken (GmxOpenMMPlatformOptions#platforms, 
 * GmxOpenMMPlatformOptions#memtests, GmxOpenMMPlatformOptions#deviceid, 
 * GmxOpenMMPlatformOptions#force_dev). 
 * \param[in] optionString  Option string part of the mdrun -deviceoption parameter.
 */
GmxOpenMMPlatformOptions::GmxOpenMMPlatformOptions(const char *optionString)
{
    // set default values
    setOption("platform",       platforms[0]);
    setOption("memtest",        memtests[0]);
    setOption("deviceid",       deviceid[0]);
    setOption("force-device",   force_dev[0]);

    string opt(optionString);

    // remove all whitespaces
    opt.erase(remove_if(opt.begin(), opt.end(), ::isspace), opt.end());
    // tokenize around ","-s
    vector<string> tokens = split(opt, ',');

    for (vector<string>::iterator it = tokens.begin(); it != tokens.end(); ++it)
    {
        string opt = "", val = "";
        splitOptionValue(*it, opt, val);

        if (isStringEqNCase(opt, "platform"))
        {
            /* no check, this will fail if platform does not exist when we try to set it */
            setOption(opt, val);
            continue;
        }

        if (isStringEqNCase(opt, "memtest"))
        {
            /* the value has to be an integer >15(s) or "full" OR "off" */
            if (!isStringEqNCase(val, "full") && !isStringEqNCase(val, "off")) 
            {
                int secs;
                if (!from_string<int>(secs, val, std::dec))
                {
                    gmx_fatal(FARGS, "Invalid value for option memtest option: \"%s\"!", val.c_str());
                }
                if (secs < 15)
                {
                    gmx_fatal(FARGS, "Incorrect value for memtest option (%d). "
                            "Memtest needs to run for at least 15s!", secs);
                }
            }
            setOption(opt, val);
            continue;
        }

        if (isStringEqNCase(opt, "deviceid"))
        {
            int id;
            if (!from_string<int>(id, val, std::dec) )
            {
                gmx_fatal(FARGS, "Invalid device id: \"%s\"!", val.c_str());
            }
            setOption(opt, val);
            continue;
        }

        if (isStringEqNCase(opt, "force-device"))
        {
            /* */
            if (!isStringEqNCase(val, "yes") && !isStringEqNCase(val, "no"))
            {
                gmx_fatal(FARGS, "Invalid OpenMM force option: \"%s\"!", val.c_str());
            }
            setOption(opt, val);
            continue;
        }

        // if we got till here something went wrong
        gmx_fatal(FARGS, "Invalid OpenMM platform option: \"%s\"!", (*it).c_str());
    }
}


/*!
 * \brief Returns the value of an option. 
 * \param[in] opt   Name of the option.
 */
string GmxOpenMMPlatformOptions::getOptionValue(const string &opt)
{
	map<string, string> :: const_iterator it = options.find(toUpper(opt));
	if (it != options.end())
    {
		// cout << "@@@>> " << it->first << " : " << it->second << endl;
		return it->second;
    }
    else
    {
        return NULL;
    }
}

/*!
 * \brief Setter function - private, only used from contructor.
 * \param[in] opt   Name of the option.
 * \param[in] val   Value for the option. 
 */
void GmxOpenMMPlatformOptions::setOption(const string &opt, const string &val)
{
    options[toUpper(opt)] = val;
}

/*!
 * \brief Removes an option with its value from the map structure. If the option 
 * does not exist, returns without any action.
 * \param[in] opt   Name of the option.
 */
void GmxOpenMMPlatformOptions::remOption(const string &opt) 
{ 
    options.erase(toUpper(opt)); 
}

/*!
 * \brief Print option-value pairs to a file (debugging function). 
 */
void GmxOpenMMPlatformOptions::print()
{
    cout << ">> Platform options: " << endl 
         << ">> platform     = " << getOptionValue("platform") << endl
         << ">> deviceID     = " << getOptionValue("deviceid") << endl
         << ">> memtest      = " << getOptionValue("memtest") << endl
         << ">> force-device = " << getOptionValue("force-device") << endl;
}

/*!
 * \brief Container for OpenMM related data structures that represent the bridge 
 *        between the Gromacs data-structures and the OpenMM library and is but it's 
 *        only passed through the API functions as void to disable direct access. 
 */
class OpenMMData
{
public:
    System* system;      /*! The system to simulate. */
    Context* context;   /*! The OpenMM context in which the simulation is carried out. */
    Integrator* integrator; /*! The integrator used in the simulation. */
    bool removeCM;          /*! If \true remove venter of motion, false otherwise. */
    GmxOpenMMPlatformOptions *platformOpt; /*! Platform options. */
};

/*!
 *  \brief Runs memtest on the GPU that has alreaby been initialized by OpenMM.
 *  \param[in] fplog    Pointer to gromacs log file.
 *  \param[in] devId    Device id of the GPU to run the test on. TODO: this can be removed!
 *  \param[in] pre_post Contains either "Pre" or "Post" just to be able to differentiate in 
 *                      stdout messages/log between memtest carried out before and after simulation.
 *  \param[in] opt      Pointer to platform options object.
 */
static void runMemtest(FILE* fplog, int devId, const char* pre_post, GmxOpenMMPlatformOptions *opt)
{
    char strout_buf[STRLEN];
    int which_test;
    int res = 0;
	string s = opt->getOptionValue("memtest");
	const char * test_type = 
			s.c_str(); 
		/* NOTE: thie code below for some misterious reason does NOT work 
		with MSVC, but the above "fix" seems to solve the problem - not sure why though */
			// opt->getOptionValue("memtest").c_str();


    if (!gmx_strcasecmp(test_type, "off"))
    {
        which_test = 0;
    }
    else
    {
        if (!gmx_strcasecmp(test_type, "full"))
        {
            which_test = 2;
        }
        else
        {
            from_string<int>(which_test, test_type, std::dec);
        }
    }

    if (which_test < 0) 
    {
        gmx_fatal(FARGS, "Amount of seconds for memetest is negative (%d). ", which_test);
    }

    switch (which_test)
    {
        case 0: /* no memtest */
            sprintf(strout_buf, "%s-simulation GPU memtest skipped. Note, that faulty memory can cause "
                "incorrect results!", pre_post);
            fprintf(fplog, "%s\n", strout_buf);
            gmx_warning(strout_buf);
            break; /* case 0 */

        case 1: /* quick memtest */
            fprintf(fplog,  "%s-simulation %s GPU memtest in progress...\n", pre_post, test_type);
            fprintf(stdout, "\n%s-simulation %s GPU memtest in progress...", pre_post, test_type);
            fflush(fplog);
            fflush(stdout);
            res = do_quick_memtest(-1);
            break; /* case 1 */

        case 2: /* full memtest */
            fprintf(fplog,  "%s-simulation %s memtest in progress...\n", pre_post, test_type);
            fprintf(stdout, "\n%s-simulation %s memtest in progress...", pre_post, test_type);
            fflush(fplog);
            fflush(stdout);
            res = do_full_memtest(-1);
            break; /* case 2 */

        default: /* timed memtest */
            fprintf(fplog,  "%s-simulation ~%ds memtest in progress...\n", pre_post, which_test);
            fprintf(stdout, "\n%s-simulation ~%ds memtest in progress...", pre_post, which_test);
            fflush(fplog);
            fflush(stdout);
            res = do_timed_memtest(-1, which_test);
        }

        if (which_test != 0 && res != 0)
        {
            gmx_fatal(FARGS, MEM_ERR_MSG(pre_post));
        }
        else
        {
            fprintf(fplog,  "Memory test completed without errors.\n");
            fflush(fplog);
            fprintf(stdout, "done, no errors detected\n");
            fflush(stdout);           
        }
}

/*!
 * \brief Does gromacs option checking.
 *
 * Checks the gromacs mdp options for features unsupported in OpenMM, case in which 
 * interrupts the execution. It also warns the user about pecularities of OpenMM 
 * implementations.
 * \param[in] ir    Gromacs structure for input options, \see ::t_inputrec
 * \param[in] top   Gromacs local topology, \see ::gmx_localtop_t
 * \param[in] state Gromacs state structure \see ::t_state
 */
static void checkGmxOptions(t_inputrec *ir, gmx_localtop_t *top, t_state *state)
{
    char warn_buf[STRLEN];

    /* Abort if unsupported critical options are present */

    /* Integrator */
    if (ir->eI ==  eiMD)
    {
        gmx_warning( "OpenMM does not support leap-frog, will use velocity-verlet integrator.");
    }

    if (    (ir->eI !=  eiMD)   &&
            (ir->eI !=  eiVV)   &&
            (ir->eI !=  eiVVAK) &&
            (ir->eI !=  eiSD1)  &&
            (ir->eI !=  eiSD2)  &&
            (ir->eI !=  eiBD) )
    {
        gmx_fatal(FARGS, "OpenMM supports only the following integrators: md/md-vv/md-vv-avek, sd/sd1, and bd.");
    }

    /* Electroctstics */
    if (    (ir->coulombtype != eelPME) &&
            (ir->coulombtype != eelRF) &&
            (ir->coulombtype != eelEWALD) &&
            // no-cutoff
            ( !(ir->coulombtype == eelCUT && ir->rcoulomb == 0 &&  ir->rvdw == 0)) )
    {
        gmx_fatal(FARGS,"OpenMM supports only the following methods for electrostatics: "
                "NoCutoff (i.e. rcoulomb = rvdw = 0 ),Reaction-Field, Ewald or PME.");
    }

    if (ir->etc != etcNO &&
        ir->eI  != eiSD1 &&
        ir->eI  != eiSD2 &&
        ir->eI  != eiBD )
    {
        gmx_warning("OpenMM supports only Andersen thermostat with the md/md-vv/md-vv-avek integrators.");
    }

    if (ir->opts.ngtc > 1)
        gmx_fatal(FARGS,"OpenMM does not support multiple temperature coupling groups.");

    if (ir->epc != etcNO)
        gmx_fatal(FARGS,"OpenMM does not support pressure coupling.");

    if (ir->opts.annealing[0])
        gmx_fatal(FARGS,"OpenMM does not support simulated annealing.");
    
    if (top->idef.il[F_CONSTR].nr > 0 && ir->eConstrAlg != econtSHAKE)
        gmx_warning("OpenMM provides contraints as a combination "
                    "of SHAKE, SETTLE and CCMA. Accuracy is based on the SHAKE tolerance set "
                    "by the \"shake_tol\" option.");

    if (ir->nwall != 0)
        gmx_fatal(FARGS,"OpenMM does not support walls.");

    if (ir->ePull != epullNO)
        gmx_fatal(FARGS,"OpenMM does not support pulling.");

    /* check for restraints */
    int i;
    for (i = 0; i < F_EPOT; i++)
    {
        if (i != F_CONSTR &&
            i != F_SETTLE &&
            i != F_BONDS  &&
            i != F_ANGLES &&
            i != F_PDIHS &&
            i != F_RBDIHS &&
            i != F_LJ14 &&
            top->idef.il[i].nr > 0)
        {
            /* TODO fix the message */
            gmx_fatal(FARGS, "OpenMM does not support (some) of the provided restaint(s).");
        }
    }

    if (ir->efep != efepNO)
        gmx_fatal(FARGS,"OpenMM does not support free energy calculations.");

    if (ir->opts.ngacc > 1)
        gmx_fatal(FARGS,"OpenMM does not support non-equilibrium MD (accelerated groups).");

    if (IR_ELEC_FIELD(*ir))
        gmx_fatal(FARGS,"OpenMM does not support electric fields.");

    if (ir->bQMMM)
        gmx_fatal(FARGS,"OpenMM does not support QMMM calculations.");

    if (ir->rcoulomb != ir->rvdw)
        gmx_fatal(FARGS,"OpenMM uses a single cutoff for both Coulomb "
                  "and VdW interactions. Please set rcoulomb equal to rvdw.");
    
    if (EEL_FULL(ir->coulombtype))
    {
        if (ir->ewald_geometry == eewg3DC)
            gmx_fatal(FARGS,"OpenMM supports only Ewald 3D geometry.");
        if (ir->epsilon_surface != 0)
            gmx_fatal(FARGS,"OpenMM does not support dipole correction in Ewald summation.");
    }

    if (TRICLINIC(state->box))        
    {
        gmx_fatal(FARGS,"OpenMM does not support triclinic unit cells.");
    }

}


/*!
 * \brief Convert Lennard-Jones parameters c12 and c6 to sigma and epsilon.
 * 
 * \param[in] c12
 * \param[in] c6
 * \param[out] sigma 
 * \param[out] epsilon
 */
static void convert_c_12_6(double c12, double c6, double *sigma, double *epsilon)
{
    if (c12 == 0 && c6 == 0)
    {
        *epsilon    = 0.0;        
        *sigma      = 1.0;
    }
    else if (c12 > 0 && c6 > 0)
    {
        *epsilon    = (c6*c6)/(4.0*c12);
        *sigma      = pow(c12/c6, 1.0/6.0);
    }
    else 
    {
        gmx_fatal(FARGS,"OpenMM does only supports c6 > 0 and c12 > 0 or both 0.");
    } 
}

/*!
 * \brief Initialize OpenMM, run sanity/consistency checks, and return a pointer to 
 * the OpenMMData.
 * 
 * Various gromacs data structures are passed that contain the parameters, state and 
 * other porperties of the system to simulate. These serve as input for initializing 
 * OpenMM. Besides, a set of misc action are taken:
 *  - OpenMM plugins are loaded;
 *  - platform options in \p platformOptStr are parsed and checked; 
 *  - Gromacs parameters are checked for OpenMM support and consistency;
 *  - after the OpenMM is initialized memtest executed in the same GPU context.
 * 
 * \param[in] fplog             Gromacs log file handler.
 * \param[in] platformOptStr    Platform option string. 
 * \param[in] ir                The Gromacs input parameters, see ::t_inputrec
 * \param[in] top_global        Gromacs whole system toppology, \see ::gmx_mtop_t
 * \param[in] top               Gromacs node local topology, \see gmx_localtop_t
 * \param[in] mdatoms           Gromacs atom parameters, \see ::t_mdatoms
 * \param[in] fr                \see ::t_forcerec
 * \param[in] state             Gromacs systems state, \see ::t_state
 *
 */
void* openmm_init(FILE *fplog, const char *platformOptStr,
                  t_inputrec *ir,
                  gmx_mtop_t *top_global, gmx_localtop_t *top,
                  t_mdatoms *mdatoms, t_forcerec *fr, t_state *state)
{

    char warn_buf[STRLEN];
    static bool hasLoadedPlugins = false;
    string usedPluginDir;
    int devId;

    try
    {
        if (!hasLoadedPlugins)
        {
            vector<string> loadedPlugins;
            /*  Look for OpenMM plugins at various locations (listed in order of priority):
                - on the path in OPENMM_PLUGIN_DIR environment variable if this is specified
                - on the path in the OPENMM_PLUGIN_DIR macro that is set by the build script
                - at the default location assumed by OpenMM
            */
            /* env var */
            char *pluginDir = getenv("OPENMM_PLUGIN_DIR");
            trim(pluginDir);
            /* no env var or empty */
            if (pluginDir != NULL && *pluginDir != '\0')
            {
                loadedPlugins = Platform::loadPluginsFromDirectory(pluginDir);
                if (loadedPlugins.size() > 0)
                {
                    hasLoadedPlugins = true;
                    usedPluginDir = pluginDir;
                }
                else
                {
                    gmx_fatal(FARGS, "The directory provided in the OPENMM_PLUGIN_DIR environment variable "
                              "(%s) does not contain valid OpenMM plugins. Check your OpenMM installation!", 
                              pluginDir);
                }
            }

            /* macro set at build time  */
#ifdef OPENMM_PLUGIN_DIR
            if (!hasLoadedPlugins)
            {
                loadedPlugins = Platform::loadPluginsFromDirectory(OPENMM_PLUGIN_DIR);
                if (loadedPlugins.size() > 0)
                {
                    hasLoadedPlugins = true;
                    usedPluginDir = OPENMM_PLUGIN_DIR;
                }
            }
#endif
            /* default loocation */
            if (!hasLoadedPlugins)
            {
                loadedPlugins = Platform::loadPluginsFromDirectory(Platform::getDefaultPluginsDirectory());
                if (loadedPlugins.size() > 0)
                {
                    hasLoadedPlugins = true;
                    usedPluginDir = Platform::getDefaultPluginsDirectory();
                }
            }

            /* if there are still no plugins loaded there won't be any */
            if (!hasLoadedPlugins)
            {
                gmx_fatal(FARGS, "No OpenMM plugins were found! You can provide the"
                          " plugin directory in the OPENMM_PLUGIN_DIR environment variable.", pluginDir);
            }

            fprintf(fplog, "\nPlugins loaded from directory %s:\t", usedPluginDir.c_str());
            for (int i = 0; i < loadedPlugins.size(); i++)
            {
                fprintf(fplog, "%s, ", loadedPlugins[i].c_str());
            }
            fprintf(fplog, "\n");
        }

        /* parse option string */
        GmxOpenMMPlatformOptions *opt = new GmxOpenMMPlatformOptions(platformOptStr);

        if (debug)
        {
            opt->print();
        }

        /* check wheter Gromacs options compatibility with OpenMM */
        checkGmxOptions(ir, top, state);

        // Create the system.
        const t_idef& idef = top->idef;
        const int numAtoms = top_global->natoms;
        const int numConstraints = idef.il[F_CONSTR].nr/3;
        const int numSettle = idef.il[F_SETTLE].nr/2;
        const int numBonds = idef.il[F_BONDS].nr/3;
        const int numUB = idef.il[F_UREY_BRADLEY].nr/3;
        const int numAngles = idef.il[F_ANGLES].nr/4;
        const int numPeriodic = idef.il[F_PDIHS].nr/5;
        const int numRB = idef.il[F_RBDIHS].nr/5;
        const int num14 = idef.il[F_LJ14].nr/3;
        System* sys = new System();
        if (ir->nstcomm > 0)
            sys->addForce(new CMMotionRemover(ir->nstcomm));

        // Set bonded force field terms.
        const int* bondAtoms = (int*) idef.il[F_BONDS].iatoms;
        HarmonicBondForce* bondForce = new HarmonicBondForce();
        sys->addForce(bondForce);
        int offset = 0;
        for (int i = 0; i < numBonds; ++i)
        {
            int type = bondAtoms[offset++];
            int atom1 = bondAtoms[offset++];
            int atom2 = bondAtoms[offset++];
            bondForce->addBond(atom1, atom2,
                               idef.iparams[type].harmonic.rA, idef.iparams[type].harmonic.krA);
        }
        // Urey-Bradley includes both the angle and bond potential for 1-3 interactions
        const int* ubAtoms = (int*) idef.il[F_UREY_BRADLEY].iatoms;
        HarmonicBondForce* ubBondForce = new HarmonicBondForce();
        HarmonicAngleForce* ubAngleForce = new HarmonicAngleForce();
        sys->addForce(ubBondForce);
        sys->addForce(ubAngleForce);
        offset = 0;
        for (int i = 0; i < numUB; ++i)
        {
            int type = ubAtoms[offset++];
            int atom1 = ubAtoms[offset++];
            int atom2 = ubAtoms[offset++];
            int atom3 = ubAtoms[offset++];
            ubBondForce->addBond(atom1, atom3,
                               idef.iparams[type].u_b.r13, idef.iparams[type].u_b.kUB);
            ubAngleForce->addAngle(atom1, atom2, atom3, 
                    idef.iparams[type].u_b.theta*M_PI/180.0, idef.iparams[type].u_b.ktheta);
        }
        const int* angleAtoms = (int*) idef.il[F_ANGLES].iatoms;
        HarmonicAngleForce* angleForce = new HarmonicAngleForce();
        sys->addForce(angleForce);
        offset = 0;
        for (int i = 0; i < numAngles; ++i)
        {
            int type = angleAtoms[offset++];
            int atom1 = angleAtoms[offset++];
            int atom2 = angleAtoms[offset++];
            int atom3 = angleAtoms[offset++];
            angleForce->addAngle(atom1, atom2, atom3, 
                    idef.iparams[type].harmonic.rA*M_PI/180.0, idef.iparams[type].harmonic.krA);
        }
        const int* periodicAtoms = (int*) idef.il[F_PDIHS].iatoms;
        PeriodicTorsionForce* periodicForce = new PeriodicTorsionForce();
        sys->addForce(periodicForce);
        offset = 0;
        for (int i = 0; i < numPeriodic; ++i)
        {
            int type = periodicAtoms[offset++];
            int atom1 = periodicAtoms[offset++];
            int atom2 = periodicAtoms[offset++];
            int atom3 = periodicAtoms[offset++];
            int atom4 = periodicAtoms[offset++];
            periodicForce->addTorsion(atom1, atom2, atom3, atom4,
                                      idef.iparams[type].pdihs.mult,
                                      idef.iparams[type].pdihs.phiA*M_PI/180.0, 
                                      idef.iparams[type].pdihs.cpA);
        }
        const int* rbAtoms = (int*) idef.il[F_RBDIHS].iatoms;
        RBTorsionForce* rbForce = new RBTorsionForce();
        sys->addForce(rbForce);
        offset = 0;
        for (int i = 0; i < numRB; ++i)
        {
            int type = rbAtoms[offset++];
            int atom1 = rbAtoms[offset++];
            int atom2 = rbAtoms[offset++];
            int atom3 = rbAtoms[offset++];
            int atom4 = rbAtoms[offset++];
            rbForce->addTorsion(atom1, atom2, atom3, atom4,
                                idef.iparams[type].rbdihs.rbcA[0], idef.iparams[type].rbdihs.rbcA[1],
                                idef.iparams[type].rbdihs.rbcA[2], idef.iparams[type].rbdihs.rbcA[3],
                                idef.iparams[type].rbdihs.rbcA[4], idef.iparams[type].rbdihs.rbcA[5]);
        }

        // Set nonbonded parameters and masses.
        int ntypes = fr->ntype;
        int* types = mdatoms->typeA;
        real* nbfp = fr->nbfp;
        real* charges = mdatoms->chargeA;
        real* masses = mdatoms->massT;
        NonbondedForce* nonbondedForce = new NonbondedForce();
        sys->addForce(nonbondedForce);
        
        switch (ir->ePBC)
        {
        case epbcNONE:
            if (ir->rcoulomb == 0)
            {
                nonbondedForce->setNonbondedMethod(NonbondedForce::NoCutoff);
            }
            else
            {
                nonbondedForce->setNonbondedMethod(NonbondedForce::CutoffNonPeriodic);
            }
            break;
        case epbcXYZ:
            switch (ir->coulombtype)
            {
            case eelRF:
                nonbondedForce->setNonbondedMethod(NonbondedForce::CutoffPeriodic);
                break;

            case eelEWALD:
                nonbondedForce->setNonbondedMethod(NonbondedForce::Ewald);
                break;

            case eelPME:
                nonbondedForce->setNonbondedMethod(NonbondedForce::PME);
                break;

            default:
                gmx_fatal(FARGS,"Internal error: you should not see this message, it that the"
                          "electrosatics option check failed. Please report this error!");
            }        
            sys->setPeriodicBoxVectors(Vec3(state->box[0][0], 0, 0),
                                       Vec3(0, state->box[1][1], 0), Vec3(0, 0, state->box[2][2]));                    
            nonbondedForce->setCutoffDistance(ir->rcoulomb);
           
            break;
        default:            
            gmx_fatal(FARGS,"OpenMM supports only full periodic boundary conditions "
                              "(pbc = xyz), or none (pbc = no).");
        }

        for (int i = 0; i < numAtoms; ++i)
        {
            double c12 = nbfp[types[i]*2*ntypes+types[i]*2+1];
            double c6 = nbfp[types[i]*2*ntypes+types[i]*2];
            double sigma, epsilon;
            convert_c_12_6(c12, c6, &sigma, &epsilon);
            nonbondedForce->addParticle(charges[i], sigma, epsilon);
            sys->addParticle(masses[i]);
        }

        // Build a table of all exclusions.
        vector<set<int> > exclusions(numAtoms);
        for (int i = 0; i < numAtoms; i++)
        {
            int start = top->excls.index[i];
            int end = top->excls.index[i+1];
            for (int j = start; j < end; j++)
                exclusions[i].insert(top->excls.a[j]);
        }

        // Record the 1-4 interactions, and remove them from the list of exclusions.
        const int* nb14Atoms = (int*) idef.il[F_LJ14].iatoms;
        offset = 0;
        for (int i = 0; i < num14; ++i)
        {
            int type = nb14Atoms[offset++];
            int atom1 = nb14Atoms[offset++];
            int atom2 = nb14Atoms[offset++];
            double sigma, epsilon;
            convert_c_12_6(idef.iparams[type].lj14.c12A, 
                    idef.iparams[type].lj14.c6A,
                    &sigma, &epsilon);
            nonbondedForce->addException(atom1, atom2,
                                         fr->fudgeQQ*charges[atom1]*charges[atom2], sigma, epsilon);
            exclusions[atom1].erase(atom2);
            exclusions[atom2].erase(atom1);
        }

        // Record exclusions.
        for (int i = 0; i < numAtoms; i++)
        {
            for (set<int>::const_iterator iter = exclusions[i].begin(); iter != exclusions[i].end(); ++iter)
            {
                if (i < *iter)
                {
                    nonbondedForce->addException(i, *iter, 0.0, 1.0, 0.0);
                }
            }
        }

        // Add GBSA if needed.
        if (ir->implicit_solvent == eisGBSA)
        {
            t_atoms atoms       = gmx_mtop_global_atoms(top_global);
            GBSAOBCForce* gbsa  = new GBSAOBCForce();

            sys->addForce(gbsa);
            gbsa->setSoluteDielectric(ir->epsilon_r);
            gbsa->setSolventDielectric(ir->gb_epsilon_solvent);
            gbsa->setCutoffDistance(nonbondedForce->getCutoffDistance());
            if (nonbondedForce->getNonbondedMethod() == NonbondedForce::NoCutoff)
                gbsa->setNonbondedMethod(GBSAOBCForce::NoCutoff);
            else if (nonbondedForce->getNonbondedMethod() == NonbondedForce::CutoffNonPeriodic)
                gbsa->setNonbondedMethod(GBSAOBCForce::CutoffNonPeriodic);
            else if (nonbondedForce->getNonbondedMethod() == NonbondedForce::CutoffPeriodic)
                gbsa->setNonbondedMethod(GBSAOBCForce::CutoffPeriodic);
            else
                gmx_fatal(FARGS,"OpenMM supports only Reaction-Field electrostatics with OBC/GBSA.");

            for (int i = 0; i < numAtoms; ++i)
            {
                gbsa->addParticle(charges[i],
                                  top_global->atomtypes.gb_radius[atoms.atom[i].type],
                                  top_global->atomtypes.S_hct[atoms.atom[i].type]);
            }
            free_t_atoms(&atoms, FALSE);
        }

        // Set constraints.
        const int* constraintAtoms = (int*) idef.il[F_CONSTR].iatoms;
        offset = 0;
        for (int i = 0; i < numConstraints; ++i)
        {
            int type = constraintAtoms[offset++];
            int atom1 = constraintAtoms[offset++];
            int atom2 = constraintAtoms[offset++];
            sys->addConstraint(atom1, atom2, idef.iparams[type].constr.dA);
        }
        const int* settleAtoms = (int*) idef.il[F_SETTLE].iatoms;
        offset = 0;
        for (int i = 0; i < numSettle; ++i)
        {
            int type = settleAtoms[offset++];
            int oxygen = settleAtoms[offset++];
            sys->addConstraint(oxygen, oxygen+1, idef.iparams[type].settle.doh);
            sys->addConstraint(oxygen, oxygen+2, idef.iparams[type].settle.doh);
            sys->addConstraint(oxygen+1, oxygen+2, idef.iparams[type].settle.dhh);
        }

        // Create an integrator for simulating the system.
        double friction = (ir->opts.tau_t[0] == 0.0 ? 0.0 : 1.0/ir->opts.tau_t[0]);
        Integrator* integ;
        if (ir->eI == eiBD)
        {
            integ = new BrownianIntegrator(ir->opts.ref_t[0], friction, ir->delta_t);
            static_cast<BrownianIntegrator*>(integ)->setRandomNumberSeed(ir->ld_seed); 
        }
        else if (EI_SD(ir->eI))
        {
            integ = new LangevinIntegrator(ir->opts.ref_t[0], friction, ir->delta_t);
            static_cast<LangevinIntegrator*>(integ)->setRandomNumberSeed(ir->ld_seed); 
        }
        else 
        {
            integ = new VerletIntegrator(ir->delta_t);
            if ( ir->etc != etcNO)
            {
                real collisionFreq = ir->opts.tau_t[0] / 1000; /* tau_t (ps) / 1000 = collisionFreq (fs^-1) */
                AndersenThermostat* thermostat = new AndersenThermostat(ir->opts.ref_t[0], friction); 
                sys->addForce(thermostat);
            }           
        }
        integ->setConstraintTolerance(ir->shake_tol);

        // Create a context and initialize it.
        Context* context = NULL;

        /*      
        OpenMM could automatically select the "best" GPU, however we're not't 
        going to let it do that for now, as the current algorithm is very rudimentary
        and we anyway support only CUDA.        
        if (platformOptStr == NULL || platformOptStr == "")
        {
            context = new Context(*sys, *integ);
        }
        else
        */        
        {
            // Find which platform is it.
            for (int i = 0; i < Platform::getNumPlatforms() && context == NULL; i++)
            {
                if (isStringEqNCase(opt->getOptionValue("platform"), Platform::getPlatform(i).getName()))
                {
                    Platform& platform = Platform::getPlatform(i);
                    // set standard properties
                    platform.setPropertyDefaultValue("CudaDevice", opt->getOptionValue("deviceid"));
                    // TODO add extra properties
                    context = new Context(*sys, *integ, platform);
                }
            }
            if (context == NULL)
            {
                gmx_fatal(FARGS, "The requested platform \"%s\" could not be found.", 
                        opt->getOptionValue("platform").c_str());
            }
        }

        Platform& platform = context->getPlatform();
        fprintf(fplog, "Gromacs will use the OpenMM platform: %s\n", platform.getName().c_str());

        const vector<string>& properties = platform.getPropertyNames();
        if (debug)
        {
            for (int i = 0; i < properties.size(); i++)
            {
                printf(">> %s: %s\n", properties[i].c_str(), 
                        platform.getPropertyValue(*context, properties[i]).c_str());
                fprintf(fplog, ">> %s: %s\n", properties[i].c_str(), 
                        platform.getPropertyValue(*context, properties[i]).c_str());
            }
        }

        /* only for CUDA */
        if (isStringEqNCase(opt->getOptionValue("platform"), "CUDA"))
        {
            /* For now this is just to double-check if OpenMM selected the GPU we wanted,
            but when we'll let OpenMM select the GPU automatically, it will query the devideId.
            */
            int tmp;
            if (!from_string<int>(tmp, platform.getPropertyValue(*context, "CudaDevice"), std::dec))
            {
                gmx_fatal(FARGS, "Internal error: couldn't determine the device selected by OpenMM");
                if (tmp != devId)
                {
                    gmx_fatal(FARGS, "Internal error: OpenMM is using device #%d"
                        "while initialized for device #%d", tmp, devId);
                }
            }
            
            /* check GPU compatibility */
            char gpuname[STRLEN];
            devId = atoi(opt->getOptionValue("deviceid").c_str());
            if (!is_supported_cuda_gpu(-1, gpuname))
            {
                if (!gmx_strcasecmp(opt->getOptionValue("force-device").c_str(), "yes"))
                {
                    sprintf(warn_buf, "Non-supported GPU selected (#%d, %s), forced continuing."
                            "Note, that the simulation can be slow or it migth even crash.", 
                            devId, gpuname);
                    fprintf(fplog, "%s\n", warn_buf);
                    gmx_warning(warn_buf);
                }
                else
                {
                    gmx_fatal(FARGS, "The selected GPU (#%d, %s) is not supported by Gromacs! "
                              "Most probably you have a low-end GPU which would not perform well, " 
                              "or new hardware that has not been tested yet with Gromacs-OpenMM. "
                              "If you still want to try using the device, use the force-device=yes option.", 
                              devId, gpuname);
                }
            }
            else
            {
                fprintf(fplog, "Gromacs will run on the GPU #%d (%s).\n", devId, gpuname);
            }
        }
        
        /* only for CUDA */
        if (isStringEqNCase(opt->getOptionValue("platform"), "CUDA"))
        {
            /* pre-simulation memtest */
            runMemtest(fplog, -1, "Pre", opt);
        }

        vector<Vec3> pos(numAtoms);
        vector<Vec3> vel(numAtoms);
        for (int i = 0; i < numAtoms; ++i)
        {
            pos[i] = Vec3(state->x[i][0], state->x[i][1], state->x[i][2]);
            vel[i] = Vec3(state->v[i][0], state->v[i][1], state->v[i][2]);
        }
        context->setPositions(pos);
        context->setVelocities(vel);

        // Return a structure containing the system, integrator, and context.
        OpenMMData* data = new OpenMMData();
        data->system = sys;
        data->integrator = integ;
        data->context = context;
        data->removeCM = (ir->nstcomm > 0);
        data->platformOpt = opt;
        return data;

    }
    catch (std::exception& e)
    {
        gmx_fatal(FARGS, "OpenMM exception caught while initializating: %s", e.what());
    }
}

/*!
 * \brief Integrate one step.
 *
 * \param[in] data  OpenMMData object created by openmm_init().
 */
void openmm_take_one_step(void* data)
{
    // static int step = 0; printf("----> taking step #%d\n", step++);
    try
    {
        static_cast<OpenMMData*>(data)->integrator->step(1);
    }
    catch (std::exception& e)
    {
        gmx_fatal(FARGS, "OpenMM exception caught while taking a step: %s", e.what());
    }
}

/*!
 * \brief Integrate n steps.
 *
 * \param[in] data  OpenMMData object created by openmm_init().
 */
void openmm_take_steps(void* data, int nstep)
{
    try
    {
        static_cast<OpenMMData*>(data)->integrator->step(nstep);
    }
    catch (std::exception& e)
    {
        gmx_fatal(FARGS, "OpenMM exception caught while taking a step: %s", e.what());
    }
}

/*!
 * \brief Clean up the data structures cretead for OpenMM.
 *
 * \param[in] log   Log file pointer.
 * \param[in] data  OpenMMData object created by openmm_init().
 */
void openmm_cleanup(FILE* fplog, void* data)
{
    OpenMMData* d = static_cast<OpenMMData*>(data);
    /* only for CUDA */
    if (isStringEqNCase(d->platformOpt->getOptionValue("platform"), "CUDA"))
    {
        /* post-simulation memtest */
        runMemtest(fplog, -1, "Post", d->platformOpt);
    }
    delete d->system;
    delete d->integrator;
    delete d->context;
    delete d->platformOpt;
    delete d;
}

/*!
 * \brief Copy the current state information from OpenMM into the Gromacs data structures.
 * 
 * This function results in the requested proprties to be copied from the 
 * GPU to host. As this represents a bottleneck, the frequency of pulling data
 * should be minimized. 
 *
 * \param[in]   data        OpenMMData object created by openmm_init().
 * \param[out]  time        Simulation time for which the state was created.
 * \param[out]  state       State of the system: coordinates and velocities.
 * \param[out]  f           Forces.
 * \param[out]  enerd       Energies.
 * \param[in]   includePos  True if coordinates are requested.
 * \param[in]   includeVel  True if velocities are requested. 
 * \param[in]   includeForce True if forces are requested. 
 * \param[in]   includeEnergy True if energies are requested. 
 */
void openmm_copy_state(void *data,
                       t_state *state, double *time,
                       rvec f[], gmx_enerdata_t *enerd,
                       bool includePos, bool includeVel, bool includeForce, bool includeEnergy)
{
    int types = 0;
    if (includePos)
        types += State::Positions;
    if (includeVel)
        types += State::Velocities;
    if (includeForce)
        types += State::Forces;
    if (includeEnergy)
        types += State::Energy;
    if (types == 0)
        return;
    try
    {
        State currentState = static_cast<OpenMMData*>(data)->context->getState(types);
        int numAtoms =  static_cast<OpenMMData*>(data)->system->getNumParticles();
        if (includePos)
        {
            for (int i = 0; i < numAtoms; i++)
            {
                Vec3 x = currentState.getPositions()[i];
                state->x[i][0] = x[0];
                state->x[i][1] = x[1];
                state->x[i][2] = x[2];
            }
        }
        if (includeVel)
        {
            for (int i = 0; i < numAtoms; i++)
            {
                Vec3 v = currentState.getVelocities()[i];
                state->v[i][0] = v[0];
                state->v[i][1] = v[1];
                state->v[i][2] = v[2];
            }
        }
        if (includeForce)
        {
            for (int i = 0; i < numAtoms; i++)
            {
                Vec3 force = currentState.getForces()[i];
                f[i][0] = force[0];
                f[i][1] = force[1];
                f[i][2] = force[2];
            }
        }
        if (includeEnergy)
        {
            int numConstraints = static_cast<OpenMMData*>(data)->system->getNumConstraints();
            int dof = 3*numAtoms-numConstraints;
            if (static_cast<OpenMMData*>(data)->removeCM)
                dof -= 3;
            enerd->term[F_EPOT] = currentState.getPotentialEnergy();
            enerd->term[F_EKIN] = currentState.getKineticEnergy();
            enerd->term[F_ETOT] = enerd->term[F_EPOT] + enerd->term[F_EKIN];
            enerd->term[F_TEMP] = 2.0*enerd->term[F_EKIN]/dof/BOLTZ;
        }
        *time = currentState.getTime();
    }
    catch (std::exception& e)
    {
        gmx_fatal(FARGS, "OpenMM exception caught while retrieving state information: %s", e.what());
    }
}

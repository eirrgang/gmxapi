#ifndef GMXAPICOMPAT_GROMACS2019_H
#define GMXAPICOMPAT_GROMACS2019_H

/*! \file
 * \brief GROMACS 2019 compatibility header.
 *
 * Provide compatibility for gmxapi extensions not available in GROMACS 2019
 * installations.
 *
 * Note that in GROMACS 2019, the `gromacs` installed header location is available
 * transitively through the imported gmxapi target because it has the same parent
 * directory as the installed gmxapi headers.
 *
 * \note This header is should not be included directly. See gmxapicompat.h
 *
 * \author M. Eric Irrgang <ericirrgang@gmail.com>
 * \ingroup gmxapi_compat
 */

// Check whether we are compiling against GROMACS 2019 and gmxapi 0.0.7
#ifdef GROMACS2019

#include <map>
#include <string>
#include <vector>
#include <ldap.h>

#include "gromacs/mdtypes/inputrec.h"

#include "exceptions.h"
#include "gmxapitypes.h"

namespace gmxapi_compat
{

/*!
 * \brief Static map of GROMACS 2019 mdp file entries to normalized "type".
 *
 * \return
 */
const std::map<std::string, GmxapiType> simulationParameterTypeMap()
{
    return {
            {"integrator", GmxapiType::gmxString},
            {"tinit",      GmxapiType::gmxFloat64},
            {"dt",         GmxapiType::gmxFloat64},
            {"nsteps",     GmxapiType::gmxInt64},
            {"init-step",     GmxapiType::gmxInt64},
            {"simulation-part",     GmxapiType::gmxInt64},
            {"comm-mode",     GmxapiType::gmxString},
            {"nstcomm",     GmxapiType::gmxInt64},
            {"comm-grps",     GmxapiType::gmxMDArray}, // Note: we do not have processing for this yet.
            {"bd-fric",     GmxapiType::gmxFloat64},
            {"ld-seed",     GmxapiType::gmxInt64},
            {"emtol",     GmxapiType::gmxFloat64},
            {"emstep",     GmxapiType::gmxFloat64},
            {"niter",     GmxapiType::gmxInt64},
            {"fcstep",     GmxapiType::gmxFloat64},
            {"nstcgsteep",     GmxapiType::gmxInt64},
            {"nbfgscorr",     GmxapiType::gmxInt64},
            {"rtpi",     GmxapiType::gmxFloat64},
            {"nstxout",     GmxapiType::gmxInt64},
            {"nstvout",     GmxapiType::gmxInt64},
            {"nstfout",     GmxapiType::gmxInt64},
            {"nstlog",     GmxapiType::gmxInt64},
            {"nstcalcenergy",     GmxapiType::gmxInt64},
            {"nstenergy",     GmxapiType::gmxInt64},
            {"nstxout-compressed",     GmxapiType::gmxInt64},
            {"compressed-x-precision",     GmxapiType::gmxFloat64},
//            {"compressed-x-grps",     GmxapiType::gmxMDArray},
//            {"energygrps",     GmxapiType::gmxInt64},
            {"cutoff-scheme",     GmxapiType::gmxString},
            {"nstlist",     GmxapiType::gmxInt64},
            {"ns-type",     GmxapiType::gmxString},
            {"pbc",     GmxapiType::gmxString},
            {"periodic-molecules",     GmxapiType::gmxBool},
//            ...

    };
};

/*
 * Visitor for predetermined known types.
 *
 * Development sequence:
 * 1. map pointers
 * 2. map setters ()
 * 3. template the Visitor setter for compile-time extensibility of type and to prune incompatible types.
 * 4. switch to Variant type for handling (setter templated on caller input)
 * 5. switch to Variant type for input as well? (Variant in public API?)
 */

const std::map<std::string, bool t_inputrec::*> boolParams()
{
    return {
            {"periodic-molecules", &t_inputrec::bPeriodicMols},
//            ...
    };
}

const std::map<std::string, int t_inputrec::*> int32Params()
{
    return {
            {"simulation-part",     &t_inputrec::simulation_part},
            {"nstcomm",     &t_inputrec::nstcomm},
            {"niter",     &t_inputrec::niter},
            {"nstcgsteep",     &t_inputrec::nstcgsteep},
            {"nbfgscorr",     &t_inputrec::nbfgscorr},
            {"nstxout",     &t_inputrec::nstxout},
            {"nstvout",     &t_inputrec::nstvout},
            {"nstfout",     &t_inputrec::nstfout},
            {"nstlog",     &t_inputrec::nstlog},
            {"nstcalcenergy",     &t_inputrec::nstcalcenergy},
            {"nstenergy",     &t_inputrec::nstenergy},
            {"nstxout-compressed",     &t_inputrec::nstxout_compressed},
            {"nstlist",     &t_inputrec::nstlist},
//            ...
    };
}

const std::map<std::string, float t_inputrec::*> float32Params()
{
    return {
            {"bd-fric",     &t_inputrec::bd_fric},
            {"emtol",     &t_inputrec::em_tol},
            {"emstep",     &t_inputrec::em_stepsize},
            {"fcstep",     &t_inputrec::fc_stepsize},
            {"rtpi",     &t_inputrec::rtpi},
            {"compressed-x-precision",     &t_inputrec::x_compression_precision},
//            ...

    };
}
const std::map<std::string, double t_inputrec::*> float64Params()
{
    return {
            {"dt", &t_inputrec::delta_t},
            {"tinit", &t_inputrec::init_t},
//            ...

    };
}
const std::map<std::string, int64_t t_inputrec::*> int64Params()
{
    return {
            {"nsteps",     &t_inputrec::nsteps},
            {"init-step",     &t_inputrec::init_step},
            {"ld-seed",     &t_inputrec::ld_seed},
//            ...

    };
}

/*!
 * \brief Static mapping of parameter names to gmxapi types for GROMACS 2019.
 *
 * \param name MDP entry name.
 * \return enumeration value for known parameters.
 *
 * \throws gmxapi_compat::ValueError for parameters with no mapping.
 */
GmxapiType mdParamToType(const std::string& name)
{
    const auto staticMap = simulationParameterTypeMap();
    auto entry = staticMap.find(name);
    if(entry == staticMap.end())
    {
        throw ValueError("Named parameter has unknown type mapping.");
    }
    return entry->second;
};

/*!
 * \brief Handle / manager for GROMACS MM computation input parameters.
 *
 * Interface should be consistent with MDP file entries, but data maps to TPR
 * file interface. For type safety and simplicity, we don't have generic operator
 * accessors. Instead, we have templated accessors that throw exceptions when
 * there is trouble.
 *
 * When MDP input is entirely stored in a key-value tree, this class can be a
 * simple adapter or wrapper. Until then, we need a manually maintained mapping
 * of MDP entries to TPR data.
 *
 * Alternatively, we could update the infrastructure used by list_tpx to provide
 * more generic output, but our efforts may be better spent in updating the
 * infrastructure for the key-value tree input system.
 */
class GmxMdParams final
{
public:
    /*!
     * \brief Create an initialized but empty parameters structure.
     *
     * Parameter keys are set at construction, but all values are empty. This
     * allows the caller to check for valid parameter names or their types,
     * while allowing the consuming code to know which parameters were explicitly
     * set by the caller.
     *
     * To load values from a TPR file, see getMdParams().
     */
    GmxMdParams();

    /*!
     * \brief Get the current list of keys.
     *
     * \return
     */
    std::vector<std::string> keys() const
    {
        std::vector<std::string> keyList;
        for (auto&& entry : int64Params_)
        {
            keyList.emplace_back(entry.first);
        }
        for (auto&& entry : floatParams_)
        {
            keyList.emplace_back(entry.first);
        }
        return keyList;
    };

    template<typename T> T extract(const std::string& key) const
    {
        auto value = T();
        // should be an APIError
        throw TypeError("unhandled type");
    }

private:
    // TODO: update to gmxapi named types?
    std::map<std::string, int64_t t_inputrec::*> int64Params_;
    std::map<std::string, int t_inputrec::*> intParams_;
    std::map<std::string, float t_inputrec::*> floatParams_;
    std::map<std::string, double t_inputrec::*> float64Params_;
    // t_inputrec requires libgromacs to construct or destroy.
    t_inputrec inputRecord_;
};

template<>
int GmxMdParams::extract<int>(const std::string& key) const {
    const auto& params = intParams_;
    const auto& entry = params.find(key);
    if (entry == params.cend())
    {
        throw KeyError("Parameter of the requested name and type not available.");
    }
    else
    {
        const auto& dataMemberPointer = entry->second;
        return inputRecord_.*dataMemberPointer;
    }
}
template<>
int64_t GmxMdParams::extract<int64_t>(const std::string& key) const {
    const auto& params = int64Params_;
    const auto& entry = params.find(key);
    if (entry == params.cend())
    {
        throw KeyError("Parameter of the requested name and type not available.");
    }
    else
    {
        const auto& dataMemberPointer = entry->second;
        return inputRecord_.*dataMemberPointer;
    }
}
template<>
float GmxMdParams::extract<float>(const std::string& key) const {
    const auto& params = floatParams_;
    const auto& entry = params.find(key);
    if (entry == params.cend())
    {
        throw KeyError("Parameter of the requested name and type not available.");
    }
    else
    {
        const auto& dataMemberPointer = entry->second;
        return inputRecord_.*dataMemberPointer;
    }
}
template<>
double GmxMdParams::extract<double>(const std::string& key) const {
    const auto& params = float64Params_;
    const auto& entry = params.find(key);
    if (entry == params.cend())
    {
        throw KeyError("Parameter of the requested name and type not available.");
    }
    else
    {
        const auto& dataMemberPointer = entry->second;
        return inputRecord_.*dataMemberPointer;
    }
}

/*!
 * \brief
 *
 */
GmxMdParams::GmxMdParams()
{
    // Set up the static mapping of (typed) parameter names.
    intParams_ = int32Params();
    int64Params_ = int64Params();
    floatParams_ = float32Params();
    float64Params_ = float64Params();
}

} // end namespace gmxapi_compat

#endif //GROMACS2019

#endif //GMXAPICOMPAT_GROMACS2019_H

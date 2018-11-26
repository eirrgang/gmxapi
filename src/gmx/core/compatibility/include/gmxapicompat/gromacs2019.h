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

namespace gmxapicompat
{

/*!
 * \brief Static map of GROMACS 2019 mdp file entries to normalized "type".
 *
 * \return
 */
const std::map<std::string, GmxapiType> simulationParameterTypeMap();

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

const std::map<std::string, bool t_inputrec::*> boolParams();
const std::map<std::string, int t_inputrec::*> int32Params();
const std::map<std::string, float t_inputrec::*> float32Params();
const std::map<std::string, double t_inputrec::*> float64Params();
const std::map<std::string, int64_t t_inputrec::*> int64Params();

/*!
 * \brief Static mapping of parameter names to gmxapi types for GROMACS 2019.
 *
 * \param name MDP entry name.
 * \return enumeration value for known parameters.
 *
 * \throws gmxapi_compat::ValueError for parameters with no mapping.
 */
GmxapiType mdParamToType(const std::string& name);

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
class GmxMdParams;

} // end namespace gmxapicompat

#endif //GROMACS2019

#endif //GMXAPICOMPAT_GROMACS2019_H

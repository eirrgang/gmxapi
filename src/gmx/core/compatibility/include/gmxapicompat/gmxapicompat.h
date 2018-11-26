#ifndef GMXPY_GMXAPICOMPAT_H
#define GMXPY_GMXAPICOMPAT_H

/*! \file
 * \brief Compatibility header for functionality differences in gmxapi releases.
 *
 * Also handle the transitioning installed headers from GROMACS 2019 moving forward.
 *
 * \todo Configure for gmxapi 0.0.7, 0.0.8, GROMACS 2019, GROMACS master...
 *
 * \defgroup gmxapi_compat
 * \ingroup gmxapi_compat
 */

#include "gromacs2019.h"

/*!
 * \brief Compatibility code for features that may not be in gmxapi yet.
 */
namespace gmxapicompat
{

/*!
 * \brief A set of overloaded functions to fetch parameters of the indicated type, if possible.
 *
 * \param params Handle to a parameters structure from which to extract.
 * \param name Parameter name
 * \param
 *
 * Could be used for dispatch and/or some sort of templating in the future, but
 * invoked directly for now.
 */
int extractParam(const gmxapicompat::GmxMdParams& params, const std::string& name, int);
int64_t extractParam(const gmxapicompat::GmxMdParams& params, const std::string& name, int64_t);
float extractParam(const gmxapicompat::GmxMdParams& params, const std::string& name, float);
double extractParam(const gmxapicompat::GmxMdParams& params, const std::string& name, double);

} // end namespace gmxapicompat

#endif //GMXPY_GMXAPICOMPAT_H

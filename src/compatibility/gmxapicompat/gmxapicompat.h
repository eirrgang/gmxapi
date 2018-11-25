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
namespace gmxapi_compat
{

} // end namespace gmxapi_compat

#endif //GMXPY_GMXAPICOMPAT_H

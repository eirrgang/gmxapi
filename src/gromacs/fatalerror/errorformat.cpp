/*
 *
 *                This source code is part of
 *
 *                 G   R   O   M   A   C   S
 *
 *          GROningen MAchine for Chemical Simulations
 *
 * Written by David van der Spoel, Erik Lindahl, Berk Hess, and others.
 * Copyright (c) 1991-2000, University of Groningen, The Netherlands.
 * Copyright (c) 2001-2009, The GROMACS development team,
 * check out http://www.gromacs.org for more information.
 *
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
 */
/*! \internal \file
 * \brief
 * Implements functions declared in errorformat.h.
 *
 * \author Teemu Murtola <teemu.murtola@cbr.su.se>
 * \ingroup module_fatalerror
 */
#include "gromacs/fatalerror/errorformat.h"

#include <string>

#include "gromacs/legacyheaders/copyrite.h"
#include "gromacs/legacyheaders/statutil.h"

#include "gromacs/utility/format.h"

namespace gmx
{

/*! \cond internal */
namespace internal
{

std::string formatFatalError(const char *title, const char *details,
                             const char *func, const char *file, int line)
{
    std::string result;
    result.append("\n-------------------------------------------------------\n");
    // TODO: Make the program name work also for unit tests
    result.append(formatString("Program %s, %s\n", "TEST", GromacsVersion()));
    if (func != NULL)
    {
        result.append(formatString("In function %s\n", func));
    }
    // TODO: Strip away absolute paths from file names (CMake seems to generate those)
    if (file != NULL)
    {
        result.append(formatString("Source file %s, line %d\n\n", file, line));
    }
    else
    {
        result.append("\n");
    }
    result.append(formatString("%s:\n%s\n", title, details));
    result.append("For more information and tips for troubleshooting, please check the GROMACS\n"
                  "website at http://www.gromacs.org/Documentation/Errors");
    result.append("\n-------------------------------------------------------\n");
    return result;
}

} // namespace internal
//! \endcond

} // namespace gmx

/*
 * This file is part of the GROMACS molecular simulation package.
 *
 * Copyright (c) 2020, by the GROMACS development team, led by
 * Mark Abraham, David van der Spoel, Berk Hess, and Erik Lindahl,
 * and including many others, as listed in the AUTHORS file in the
 * top-level source directory and at http://www.gromacs.org.
 *
 * GROMACS is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1
 * of the License, or (at your option) any later version.
 *
 * GROMACS is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with GROMACS; if not, see
 * http://www.gnu.org/licenses, or write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA.
 *
 * If you want to redistribute modifications to GROMACS, please
 * consider that scientific software is very special. Version
 * control is crucial - bugs must be traceable. We will be happy to
 * consider code for inclusion in the official distribution, but
 * derived work must not be called official GROMACS. Details are found
 * in the README & COPYING files - if they are missing, get the
 * official version at http://www.gromacs.org.
 *
 * To help us fund GROMACS development, we humbly ask that you cite
 * the research papers on the package. Check out http://www.gromacs.org.
 */
/*! \file
 * \brief
 *
 * \author M. Eric Irrgang <ericirrgang@gmail.com>
 * \ingroup module_mdrun
 */

#ifndef GMX_MDRUN_SIMULATIONINPUTUTILITY_H
#define GMX_MDRUN_SIMULATIONINPUTUTILITY_H

#include <memory>

#include "simulationinput.h"

namespace gmx
{

// Forward declarations for types from other modules that are opaque to the public API.
// TODO: Document the sources of these symbols or import a (self-documenting) fwd header.
struct t_commrec;
struct ivec;
class t_inputrec;
class t_state;
struct gmx_mtop_t;
struct t_fileio;
struct ObservablesHistory;
struct gmx_bool;

/*! \brief Get the global simulation input.
 *
 * Acquire global simulation data structures from the SimulationInput handle.
 * Note that global data is returned in the calling thread. In parallel
 * computing contexts, the client is responsible for calling only where needed.
 *
 * Example:
 *    if (SIMMASTER(cr))
 *    {
 *        // Only the master rank has the global state
 *        globalState = globalSimulationState(simulationInput);
 *
 *        // Read (nearly) all data required for the simulation
 *        applyGlobalInputRecord(simulationInput, inputrec);
 *        applyGlobalTopology(simulationInput, &mtop);
 *     }
 */
std::unique_ptr<t_state> globalSimulationState(const SimulationInput&);
void                     applyGlobalInputRecord(const SimulationInput&, t_inputrec*);
void                     applyGlobalTopology(const SimulationInput&, gmx_mtop_t*);

/*! \brief Initialize local stateful simulation data.
 *
 * Establish an invariant for the simulator at a trajectory point.
 * Call on all ranks (after domain decomposition and task assignments).
 *
 * After this call, the simulator has all of the information it will
 * receive in order to advance a trajectory from the current step.
 * Checkpoint information has been applied, if applicable, and stateful
 * data has been (re)initialized.
 *
 * \warning It is the caller’s responsibility to make sure that
 * preconditions are satisfied for the parameter objects.
 *
 * \seealso globalSimulationState()
 * \seealso applyGlobalInputRecord()
 * \seealso applyGlobalTopology()
 *
 * Example:
 *    applyLocalState(simulationInput,
 *               logFileHandle,
 *               cr, domdecOptions.numCells,
 *               inputrec, globalState.get(),
 *               &observablesHistory,
 *               mdrunOptions.reproducible);
 */
void applyLocalState(const SimulationInput&,
                     t_fileio*           logfio,
                     const t_commrec*    cr,
                     const ivec          dd_nc,
                     t_inputrec*         ir,
                     t_state*            state,
                     ObservablesHistory* observablesHistory,
                     gmx_bool            reproducibilityRequested);

} // end namespace gmx

#endif // GMX_MDRUN_SIMULATIONINPUTUTILITY_H

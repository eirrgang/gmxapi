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

#include <gromacs/mdtypes/state.h>
#include <gromacs/fileio/gmxfio.h>
#include <gromacs/mdtypes/commrec.h>
#include <gromacs/mdtypes/observableshistory.h>
#include <gromacs/topology/topology.h>
#include "simulationinput.h"
#include "simulationinpututility.h"

namespace gmx
{

class SimulationInput
{
};

class detail::SimulationInputHolderImpl
{
public:
    SimulationInput* simulationInput() const { return simulationInput_.get(); }

private:
    std::shared_ptr<SimulationInput> simulationInput_;
};

detail::SimulationInputHolderImplDeleter::SimulationInputHolderImplDeleter() = default;

detail::SimulationInputHolderImplDeleter::SimulationInputHolderImplDeleter(
        const SimulationInputHolderImplDeleter&) noexcept = default;

detail::SimulationInputHolderImplDeleter::SimulationInputHolderImplDeleter(
        SimulationInputHolderImplDeleter&&) noexcept = default;

detail::SimulationInputHolderImplDeleter& detail::SimulationInputHolderImplDeleter::
                                          operator=(const SimulationInputHolderImplDeleter&) noexcept = default;

detail::SimulationInputHolderImplDeleter& detail::SimulationInputHolderImplDeleter::
                                          operator=(SimulationInputHolderImplDeleter&&) noexcept = default;

void detail::SimulationInputHolderImplDeleter::operator()(SimulationInputHolderImpl* impl) const
{
    delete impl;
}

SimulationInputHolder::~SimulationInputHolder() = default;

SimulationInputHolder::SimulationInputHolder(std::unique_ptr<detail::SimulationInputHolderImpl>&& impl)
{
    impl_.reset(impl.release());
}

SimulationInput* SimulationInputHolder::get() const noexcept
{
    return impl_->simulationInput();
}

std::unique_ptr<t_state> globalSimulationState(const SimulationInput&);

void applyGlobalInputRecord(const SimulationInput&, t_inputrec*);

void applyGlobalTopology(const SimulationInput&, gmx_mtop_t*);

void applyLocalState(const SimulationInput&,
                     t_fileio*           logfio,
                     const t_commrec*    cr,
                     const ivec          dd_nc,
                     t_inputrec*         ir,
                     t_state*            state,
                     ObservablesHistory* observablesHistory,
                     gmx_bool            reproducibilityRequested);


} // end namespace gmx

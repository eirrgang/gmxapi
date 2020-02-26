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
 * \brief Public interface for SimulationInput facilities.
 *
 * \author M. Eric Irrgang <ericirrgang@gmail.com>
 * \ingroup module_mdrun
 */

#ifndef GMX_MDRUN_SIMULATIONINPUT_H
#define GMX_MDRUN_SIMULATIONINPUT_H

#include <memory>

namespace gmx
{

// Forward declarations for types from other modules that are opaque to the public API.
class LegacyMdrunOptions;

/*!
 * \brief Prescription for molecular simulation.
 *
 * Represent the complete and unique information needed to generate a simulation
 * trajectory segment. SimulationInput objects are opaque to the public API.
 * Ownership can be managed with SimulationInputHolder objects. Clients can
 * acquire owning references to SimulationInput objects (as SimulationInputHolder)
 * through makeSimulationInput() or from other SimulationInputHolders.
 *
 * A SimulationInput object represents an immutable source of data, and is safe
 * to share. A SimulationInput object may have internal state to support
 * performance optimizations when shared by multiple SimulationInputHolders.
 * The SimulationInput is guaranteed to live at least as long as any associated
 * SimulationInputHolders. The API does not specify whether it may persist
 * longer internally or be reused for later equivalent requests.
 *
 * \seealso SimulationInputHolder
 * \seealso makeSimulationInput()
 *
 * See also https://redmine.gromacs.org/issues/3379 for design and development road map.
 */
class SimulationInput;

/*! \cond internal
 */
namespace detail
{
/*! \internal
 * \brief Private implementation class;
 */
class SimulationInputHolderImpl;

/*! \internal
 * \brief Explicit deleter details for SimulationInputHolderImpl.
 *
 * SimulationInputHolderImpl objects are created by the GROMACS library, but
 * are destroyed when the SimulationInputHolder goes out of scope in the client
 * code, which may be linked to a different allocator.
 * We want to make sure that objects are allocated and deallocated with the same
 * allocator, so we avoid the default deleter in unique_ptrs and compile allocation
 * and deallocation code in the same translation unit.
 *
 * Note that this does not solve potential ABI incompatibilities between the
 * unique_ptr implementations themselves, but we need to consider ABI
 * compatibility goals and challenges as well as supported use cases and
 * ownership semantics.
 */
struct SimulationInputHolderImplDeleter
{
    SimulationInputHolderImplDeleter();
    SimulationInputHolderImplDeleter(const SimulationInputHolderImplDeleter&) noexcept;
    SimulationInputHolderImplDeleter(SimulationInputHolderImplDeleter&&) noexcept;
    SimulationInputHolderImplDeleter& operator=(const SimulationInputHolderImplDeleter&) noexcept;
    SimulationInputHolderImplDeleter& operator=(SimulationInputHolderImplDeleter&&) noexcept;
    void                              operator()(SimulationInputHolderImpl*) const;
};
} // end namespace detail
/*! \endcond internal */

/*!
 * \brief Owning handle to a SimulationInput object.
 *
 * SimulationInput objects are logically immutable, so ownership may be shared
 * by multiple SimulationInputHolders.
 *
 * Acquire a SimulationInputHolder with makeSimulationInput()
 *
 * \seealso https://redmine.gromacs.org/issues/3379
 */
class SimulationInputHolder
{
public:
    SimulationInputHolder() = delete;
    ~SimulationInputHolder();

    /*! \cond internal
     * \brief Take ownership of private implementation object to produce a new public holder.
     */
    explicit SimulationInputHolder(std::unique_ptr<detail::SimulationInputHolderImpl>&&);
    /*! \endcond */

    /*!
     * \brief Access opaque SimulationInput pointer.
     *
     * \return Borrowed access to the SimulationInput.
     */
    SimulationInput* get() const noexcept;

private:
    std::unique_ptr<detail::SimulationInputHolderImpl, detail::SimulationInputHolderImplDeleter> impl_;
};

/*! \brief Direct the construction of a SimulationInput.
 *
 * Example:
 *     // After preparing a LegacyMdrunOptions and calling handleRestart()...
 *     SimulationInputBuilder builder;
 *     auto simulationInputHandle = makeSimulationInput(options, &builder);
 *
 *     // In addition to MdrunnerBuilder::addFiles(),
 *     mdrunnerBuilder.addInput(simulationInputHandle.get());
 *
 */
SimulationInputHolder makeSimulationInput(const LegacyMdrunOptions&);
// TODO: SimulationInputHolder makeSimulationInput(const LegacyMdrunOptions&, SimulationInputBuilder*);

} // end namespace gmx

#endif // GMX_MDRUN_SIMULATIONINPUT_H

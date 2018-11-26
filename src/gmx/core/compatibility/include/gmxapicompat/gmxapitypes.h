#ifndef GMXPY_GMXAPITYPES_H
#define GMXPY_GMXAPITYPES_H

/*! \file
 * \brief Type helpers for gmxapi data compatibility
 *
 * \author M. Eric Irrgang <ericirrgang@gmail.com>
 */

#include <type_traits>

namespace gmxapicompat {

/*!
 * \brief Label the types recognized by gmxapi.
 *
 * Provide an enumeration to aid in translating data between languages, APIs,
 * and storage formats.
 *
 * \todo The spec should explicitly map these to types in APIs already used.
 * e.g. MPI, Python, numpy, GROMACS, JSON, etc.
 * \todo Actually check the size of the types.
 */
enum class GmxapiType {
    gmxNull, //! Reserved
    gmxMap, //! Mapping of key name (string) to a value of some MdParamType
    gmxBool, //! Boolean logical type
    gmxInt32, //! 32-bit integer type, initially unused
    gmxInt64, //! 64-bit integer type
    gmxFloat32, //! 32-bit float type, initially unused
    gmxFloat64, //! 64-bit float type
    gmxString, //! string with metadata
    gmxMDArray, //! multi-dimensional array with metadata
// Might be appropriate to have convenience types for small non-scalars that
// shouldn't need metadata.
//    gmxFloat32Vector3, //! 3 contiguous 32-bit floating point values.
//    gmxFloat32SquareMatrix3, //! 9 contiguous 32-bit FP values in row-major order.
};

namespace traits {

// These can be more than traits. We might as well make them named types.
struct gmxNull {
    static const GmxapiType value = GmxapiType::gmxNull;
};
struct gmxMap {
    static const GmxapiType value = GmxapiType::gmxMap;
};
struct gmxInt32 {
    static const GmxapiType value = GmxapiType::gmxInt32;
};
struct gmxInt64 {
    static const GmxapiType value = GmxapiType::gmxInt64;
};
struct gmxFloat32 {
    static const GmxapiType value = GmxapiType::gmxFloat32;
};
struct gmxFloat64 {
    static const GmxapiType value = GmxapiType::gmxFloat64;
};
struct gmxBool {
    static const GmxapiType value = GmxapiType::gmxBool;
};
struct gmxString {
    static const GmxapiType value = GmxapiType::gmxString;
};
struct gmxMDArray {
    static const GmxapiType value = GmxapiType::gmxMDArray;
};
//struct gmxFloat32Vector3 {
//    static const GmxapiType value = GmxapiType::gmxFloat32Vector3;
//};
//struct gmxFloat32SquareMatrix3 {
//    static const GmxapiType value = GmxapiType::gmxFloat32SquareMatrix3;
//};

} // end namespace traits


// Partial specialization of functions is not allowed, which makes the following tedious.
// To-do: switch to type-based logic, struct templates, etc.
template<typename T, size_t s> GmxapiType mapCppType()
{
    return GmxapiType::gmxNull;
}

template<typename T> GmxapiType mapCppType()
{
    return mapCppType<T, sizeof(T)>();
};

template<> GmxapiType mapCppType<bool>()
{
    return GmxapiType::gmxBool;
}

template<> GmxapiType mapCppType<int, 4>()
{
    return GmxapiType::gmxInt32;
}

template<> GmxapiType mapCppType<int, 8>()
{
    return GmxapiType::gmxInt64;
};


template<> GmxapiType mapCppType<float, 4>()
{
    return GmxapiType::gmxFloat32;
}

template<> GmxapiType mapCppType<double, 8>()
{
    return GmxapiType::gmxFloat64;
};


} // end namespace gmxapicompat

#endif //GMXPY_GMXAPITYPES_H

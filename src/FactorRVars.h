#ifndef HALIDE_FACTOR_RVARS_H
#define HALIDE_FACTOR_RVARS_H

/** \file
 *
 * Defines pass to replace calls to wrapped Functions with their wrappers.
 */

#include <map>

#include "IR.h"

namespace Halide {
namespace Internal {

/** Replace every call to wrapped Functions in the Functions' definitions with
  * call to their wrapper functions. */
std::map<std::string, Function> factor_rvars(const std::map<std::string, Function> &env);

}
}

#endif

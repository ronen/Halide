#include "LiftCalls.h"

#include <set>

namespace Halide{
namespace Internal {

using std::map;
using std::pair;
using std::set;
using std::string;
using std::vector;

typedef map<Function, Function, Function::Compare> SubstitutionMap;

map<string, Function> lift_func_calls(const map<string, Function> &env) {
    map<string, Function> wrapped_env(env);

    //TODO(psuriana): implement this
    return wrapped_env;
}

}
}
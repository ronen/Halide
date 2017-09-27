#include "DeviceInterface.h"
#include "JITModule.h"
#include "IROperator.h"
#include "Target.h"

using namespace Halide;
using namespace Halide::Internal;

namespace Halide {

namespace {

// Defines the precedence for which DeviceAPI will be used when
// DeviceAPI::Default_GPU is requested.
const std::vector<DeviceAPI> default_device_api_order = {
    DeviceAPI::Metal,
    DeviceAPI::OpenCL,
    DeviceAPI::CUDA,
    DeviceAPI::OpenGLCompute,
    DeviceAPI::GLSL
};

template <typename fn_type>
bool lookup_runtime_routine(const std::string &name,
                            const Target &target,
                            fn_type &result) {
    std::vector<JITModule> runtime(
        JITSharedRuntime::get(nullptr, target.with_feature(Target::JIT)));

    for (size_t i = 0; i < runtime.size(); i++) {
        std::map<std::string, JITModule::Symbol>::const_iterator f =
          runtime[i].exports().find(name);
        if (f != runtime[i].exports().end()) {
            result = reinterpret_bits<fn_type>(f->second.address);
            return true;
        }
    }
    return false;
}

}

const halide_device_interface_t *get_default_device_interface_for_target(const Target &t) {
    return get_device_interface_for_device_api(DeviceAPI::Default_GPU, t);
}

const halide_device_interface_t *get_device_interface_for_device_api(const DeviceAPI &d, const Target &t) {
    if (d == DeviceAPI::Default_GPU) {
        return get_device_interface_for_device_api(get_default_device_api_for_target(t), t);
    }

    const struct halide_device_interface_t *(*fn)();
    std::string name;
    if (d == DeviceAPI::Metal) {
        name = "metal";
    } else if (d == DeviceAPI::OpenCL) {
        name = "opencl";
    } else if (d == DeviceAPI::CUDA) {
        name = "cuda";
    } else if (d == DeviceAPI::OpenGLCompute) {
        name = "openglcompute";
    } else if (d == DeviceAPI::GLSL) {
        name = "opengl";
    } else {
        return nullptr;
    }

    if (lookup_runtime_routine("halide_" + name + "_device_interface", t, fn)) {
        return (*fn)();
    } else {
        return nullptr;
    }
}

bool device_api_enabled_in_target(const DeviceAPI &d, const Target &target)
{
    switch (d) {
        case DeviceAPI::Metal:         return target.has_feature(Target::Metal);
        case DeviceAPI::OpenCL:        return target.has_feature(Target::OpenCL);
        case DeviceAPI::CUDA:          return target.has_feature(Target::CUDA);
        case DeviceAPI::OpenGLCompute: return target.has_feature(Target::OpenGLCompute);
        case DeviceAPI::GLSL:          return target.has_feature(Target::OpenGL);
        default:                       return false;
    }
}

DeviceAPI get_default_device_api_for_target(const Target &target) {
    for (DeviceAPI d : default_device_api_order) {
        if (device_api_enabled_in_target(d, target)) {
            return d;
        }
    }
    return DeviceAPI::Host;
}


namespace Internal {
Expr make_device_interface_call(DeviceAPI device_api) {
    if (device_api == DeviceAPI::Host) {
        return make_zero(type_of<const halide_device_interface_t *>());
    }

    std::string interface_name;
    switch (device_api) {
    case DeviceAPI::CUDA:
        interface_name = "halide_cuda_device_interface";
        break;
    case DeviceAPI::OpenCL:
        interface_name = "halide_opencl_device_interface";
        break;
    case DeviceAPI::Metal:
        interface_name = "halide_metal_device_interface";
        break;
    case DeviceAPI::GLSL:
        interface_name = "halide_opengl_device_interface";
        break;
    case DeviceAPI::OpenGLCompute:
        interface_name = "halide_openglcompute_device_interface";
        break;
    case DeviceAPI::Hexagon:
        interface_name = "halide_hexagon_device_interface";
        break;
    case DeviceAPI::Default_GPU:
        // Will be resolved later
        interface_name = "halide_default_device_interface";
        break;
    default:
        internal_error << "Bad DeviceAPI " << static_cast<int>(device_api) << "\n";
        break;
    }
    return Call::make(type_of<const halide_device_interface_t *>(), interface_name, {}, Call::Extern);
}
}

}

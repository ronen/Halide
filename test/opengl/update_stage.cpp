// test case provided by Lee Yuguang


#include "Halide.h"
#include <stdio.h>
#include <chrono>

using namespace Halide;

int main() {
    // This test must be run with an OpenGL target.
    const Target target = get_jit_target_from_environment().with_feature(Target::OpenGL);

    // Define the input
    const int width = 10, height = 12, channels = 2, res_channels = 2;
    Buffer<float> input(width, height, channels);
    for (int c = 0; c < input.channels(); c++) {
        for (int y = 0; y < input.height(); y++) {
            for (int x = 0; x < input.width(); x++) {
                input(x, y, c) = float(x + y);
            }
        }
    }

    // Define the algorithm.
    Var x, y, c;
    RDom r(3, 5, "r");
    RDom r2(2, 4, "r");
    Func f, g;

    Expr coordx = clamp(x + r, 0, input.width() - 1);
    f(x, y, c) = cast<uint8_t>(0);
    f(r.x, y, c) = cast<uint8_t>(11);

    g(x, y, c) = cast<uint8_t>(1);
    g(r2.x, y, c) = cast<uint8_t>(9);


    // Schedule f to compute pure stage on CPU and update on GPU
    f.bound(c, 0, 2);
    f.update(0).glsl(r.x, y, c);

    // Schedule g to compute pure and update stages on GPU
    g.bound(c, 0, 2);
    g.glsl(x, y, c).update(0).glsl(r2.x, y, c);

    // Generate the result.
    Buffer<uint8_t> result = f.realize(width, height, res_channels, target);

    result.copy_to_host();

    //Check the result.
    for (int c = 0; c < result.channels(); c++) {
        for (int y = 0; y < result.height(); y++) {
            for (int x = 0; x < result.width(); x++) {
                uint8_t correct = (x>=3 && x<8) ? 11 : 0;
                if (result(x, y, c) != correct) {
                    fprintf(stderr, "result(%d, %d, %d) = %d, should be %d\n",
                            x, y, c, result(x, y, c), correct);
                    return 1;
                }
            }
        }
    }

    Buffer<uint8_t> result2 = g.realize(width, height, res_channels, target);

    result2.copy_to_host();

    //Check the result.
    for (int c = 0; c < result2.channels(); c++) {
        for (int y = 0; y < result2.height(); y++) {
            for (int x = 0; x < result2.width(); x++) {
                uint8_t correct = (x>=2 && x<6) ? 9 : 1;
                if (result2(x, y, c) != correct) {
                    fprintf(stderr, "result2(%d, %d, %d) = %d, should be %d\n",
                            x, y, c, result2(x, y, c), correct);
                    return 1;
                }
            }
        }
    }



    printf("Success!\n");
    return 0;
}

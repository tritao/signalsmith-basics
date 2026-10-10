#include "signalsmith-basics/reverb.h"
#include <array>
#include <cassert>
#include <cmath>

// Standalone regression test; pass Basics/include and signalsmith-dsp/include
// to a C++17 compiler. No plugin host or audio device is required.
int main() {
    for (const int rate : {32000, 48000, 96000}) {
        for (const int block : {1, 64, 257}) {
            signalsmith::basics::ReverbFloat effect;
            effect.dry = 0;
            effect.wet = 1;
            assert(effect.configure(rate, 257, 2));
            std::array<float, 257> left{}, right{}, out_left{}, out_right{};
            float *input[] = {left.data(), right.data()};
            float *output[] = {out_left.data(), out_right.data()};
            left[0] = right[0] = 1;
            effect.process(input, output, block);
            left[0] = right[0] = 0;
            float tail_energy = 0;
            for (int rendered = 0; rendered < rate / 5; rendered += block) {
                effect.process(input, output, block);
                for (int i = 0; i < block; ++i) {
                    assert(std::isfinite(out_left[i]) && std::isfinite(out_right[i]));
                    tail_energy += out_left[i]*out_left[i] + out_right[i]*out_right[i];
                }
            }
            assert(tail_energy > 0);
            effect.reset();
            // Check beyond the longest final diffusion delay. A missing
            // delay4.reset() brings part of the old tail back during this interval.
            for (int rendered = 0; rendered < rate / 5; rendered += block) {
                effect.process(input, output, block);
                for (int i = 0; i < block; ++i)
                    assert(out_left[i] == 0 && out_right[i] == 0);
            }
        }
    }
}

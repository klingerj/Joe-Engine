#pragma once

#include <random>

// TODO: make pcg compile, not sure why it won't
//#include "pcg_random.hpp"

namespace JoeEngine {
    namespace RNG {
        template <typename T>
        class JERandomNumberGen {
        private:
            //pcg32 rng;
            std::default_random_engine rng;
            std::uniform_real_distribution<> uniform_dist;
            
        public:
            JERandomNumberGen(T min, T max) {
                //pcg_extras::seed_seq_from<std::random_device> seed_source;
                //rng = pcg32(seed_source);
                std::random_device rd;
                std::default_random_engine rng(rd());
                uniform_dist = std::uniform_real_distribution<>(min, max);
            }
            ~JERandomNumberGen() {}
            
            T GetNextRandomNum() {
                return uniform_dist(rng);
            }
        };

        template <typename T>
        class JEIntRandomNumberGen {
        private:
            //pcg32 rng;
            std::default_random_engine rng;
            std::uniform_int_distribution<> uniform_dist;

        public:
            JEIntRandomNumberGen(T min, T max) {
                //pcg_extras::seed_seq_from<std::random_device> seed_source;
                //rng = pcg32(seed_source);
                std::random_device rd;
                std::default_random_engine rng(rd());
                uniform_dist = std::uniform_int_distribution<>(min, max);
            }
            ~JEIntRandomNumberGen() {}

            T GetNextRandomNum() {
                return uniform_dist(rng);
            }
        };
    }
}

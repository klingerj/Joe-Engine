#pragma once

#include <random>

// TODO: make pcg compile, not sure why it won't
//#include "pcg_random.hpp"

namespace JoeEngine {
    namespace RNG {
        //! The RNG class
        /*!
          Wrapper class around random number generation classes for a cleaner API.
          This is the float/double variant (complies with std::uniform_real_distribution)
        */
        template <typename T>
        class JERandomNumberGen {
        private:
            //pcg32 rng;
            //! Random number engine
            std::default_random_engine m_rng;

            //! Uniform distribution
            std::uniform_real_distribution<> m_uniform_dist;
            
        public:
            //! Default constructor.
            /*! Deleted. */
            JERandomNumberGen() = delete;

            //! Constructor.
            /*! Constructs the class given a desired min and max range for the values. */
            JERandomNumberGen(T min, T max) {
                //pcg_extras::seed_seq_from<std::random_device> seed_source;
                //rng = pcg32(seed_source);
                std::random_device rd;
                std::default_random_engine rng(rd());
                m_uniform_dist = std::uniform_real_distribution<>(min, max);
            }

            //! Destructor (default).
            ~JERandomNumberGen() = default;
            
            //! Get a random number from the distribution.
            /*! Returns a new random number.*/
            T GetNextRandomNum() {
                return m_uniform_dist(m_rng);
            }
        };

        //! The RNG class
        /*!
          Wrapper class around random number generation classes for a cleaner API.
          This is the integer variant (complies with std::uniform_int_distribution)
        */
        template <typename T>
        class JEIntRandomNumberGen {
        private:
            //pcg32 rng;
            //! Random number engine
            std::default_random_engine m_rng;

            //! Uniform distribution
            std::uniform_int_distribution<> m_uniform_dist;

        public:
            //! Default constructor.
            /*! Deleted. */
            JEIntRandomNumberGen() = delete;

            //! Constructor.
            /*! Constructs the class given a desired min and max range for the values. */
            JEIntRandomNumberGen(T min, T max) {
                //pcg_extras::seed_seq_from<std::random_device> seed_source;
                //rng = pcg32(seed_source);
                std::random_device rd;
                std::default_random_engine rng(rd());
                m_uniform_dist = std::uniform_int_distribution<>(min, max);
            }

            //! Destructor (default).
            ~JEIntRandomNumberGen() = delete;

            //! Get a random number from the distribution.
            /*! Returns a new random number.*/
            T GetNextRandomNum() {
                return m_uniform_dist(m_rng);
            }
        };
    }
}

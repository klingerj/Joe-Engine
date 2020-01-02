#pragma once

#include <chrono>

#include "ParticleSystem.h"

namespace JoeEngine {
    //! The Physics Manager class.
    /*!
      Class dedicated to making physics calculations at a framerate that is decoupled from the rendering framerate.
      Currently only used for particle system calculations.s
    */
    class JEPhysicsManager {
    private:
        //! Simple std::chrono typedef for convenience
        using JE_TIME = std::chrono::time_point<std::chrono::steady_clock>;

        //! Start time.
        /*!
          The start time marker for a particular frame. Physics calculations only occur if the amount of time elapsed
          since this time marker is greater than the specified time increment, 'm_updateRateMillis'. Then this marker
          is updated to contain the current time.
        */
        JE_TIME m_startTime;

        //! Update rate in milliseconds.
        /*! How many milliseconds must pass before a physics update is permitted to occur. */
        const float m_updateRateMillis; // time increment

        //! Update rate for integration
        /*! Timestep for physics integration calculations. */
        const float m_updateDt; // dt for physics integration

    public:
        //! Constructor.
        /*! Initializes update rate member variables. */
        JEPhysicsManager() : m_startTime(), m_updateRateMillis(16.6666667f), m_updateDt(1.0f / 60.0f) {}

        //! Destructor (default).
        ~JEPhysicsManager() = default;

        //! Initialize the class.
        void Initialize();

        //! Update particle systems.
        /*!
          Performs an update on each provided particle system, if at least 'm_updateRateMillis' milliseconds have elapsed
          since the last update.
          \param particleSystems the list of particle systems to update.
        */
        void UpdateParticleSystems(std::vector<JEParticleSystem>& particleSystems);
    };
}

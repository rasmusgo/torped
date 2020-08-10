Based on:

    Rozmanov, Dmitri & Kusalik, Peter. (2010). Robust rotational-velocity-Verlet integration methods. Physical review. E, Statistical, nonlinear, and soft matter physics. 81. 056706. 10.1103/PhysRevE.81.056706.

1) Convert angular momentum and torque to local frame for t = 0

    angular_momentum_in_local(t=0) = R_world_from_local(t=0).conjugate() * angular_momentum_in_world(t=0) * R_world_from_local(t=0)

    torque_in_local(t=0) = R_world_from_local(t=0).conjugate() * torque_in_world(t=0) * R_world_from_local(t=0)

2) Estimate angular_momentum_in_local at t = Δt / 2

    angular_velocity_in_local(t=0) = inertia_in_local.inverse() * angular_momentum_in_local(t=0)

    d/dt angular_momentum_in_local(t=0) = torque_in_local(t=0) - angular_velocity_in_local(t=0) cross angular_momentum_in_local(t=0)

    angular_momentum_in_local(t=Δt/2) = angular_momentum_in_local(t=0) + Δt/2 * d/dt angular_momentum_in_local(t=0)

3) Zeroth approximation of time derivative of R_world_from_local at t = Δt/2

    d/dt R_world_from_local[0](t=Δt/2)
        = 1/2 * R_world_from_local(t=0) * angular_velocity_in_local(t=Δt/2)
        = 1/2 * R_world_from_local(t=0) * inertia_in_local.inverse() * angular_momentum_in_local(t=Δt/2)

4) Zeroth approximation of R_world_from_local at t = Δt/2

    R_world_from_local[0](t=Δt/2) = R_world_from_local(t=0) + Δt/2 * d/dt R_world_from_local[0](t=Δt/2)

5) Propagate angular_momentum_in_world

    angular_momentum_in_world(t=Δt/2) = angular_momentum_in_world(t=0) + Δt/2 * torque_in_world(t=0)

6) Iterate over k until |R_world_from_local[k+1](t=Δt/2) - R_world_from_local[k](t=Δt/2)| < epsilon, eg. epsilon = 10^-9

    angular_momentum_in_local[k+1](t=Δt/2) = R_world_from_local[k](t=Δt/2).conjugate() * angular_momentum_in_world(t=Δt/2) * R_world_from_local[k](t=Δt/2)

    angular_velocity_in_local[k+1](t=Δt/2) = inertia_in_local.inverse() * angular_momentum_in_local[k+1](t=Δt/2)

    d/dt R_world_from_local[k+1](t=Δt/2) = 1/2 * R_world_from_local[k](t=Δt/2) * angular_velocity_in_local[k+1](t=Δt/2)

    R_world_from_local[k+1](t=Δt/2) = R_world_from_local(t=0) + Δt/2 * d/dt R_world_from_local[k+1](t=Δt/2)

7) When d/dt R_world_from_local(t=Δt/2) is found, we can compute R_world_from_local(t=Δt) and torque at t = Δt

    R_world_from_local(t=Δt) = R_world_from_local(t=0) + Δt * d/dt R_world_from_local(t=Δt/2)

    torque_in_world(t=Δt) = compute_torque_in_world(R_world_from_local(t=Δt))

8) angular_momentum_in_world at t = Δt

    angular_momentum_in_world(t=Δt) = angular_momentum_in_world(t=Δt/2) + Δt/2 * torque_in_world(t=Δt)

#include <complex>
#include <ostream>
#include <sstream>

#include <Eigen/Eigen>
#include <unsupported/Eigen/AutoDiff>

#include "physics.hpp"
#include "logging.hpp"

using Jet12 = Eigen::AutoDiffScalar<Eigen::Vector<double, 12>>; // x0, y0, z0, x0', y0', z0', x1, y1, z1, x1', y1', z1'

Eigen::Vector<Jet12, 3> SpringForce(
    const Eigen::Vector<Jet12, 3>& A_pos,
    const Eigen::Vector<Jet12, 3>& A_vel,
    const Eigen::Vector<Jet12, 3>& B_pos,
    const Eigen::Vector<Jet12, 3>& B_vel,
    double relaxed_length,
    double k, // Spring constant
    double d) // Damping constant
{
    const Eigen::Vector<Jet12, 3> AB = B_pos - A_pos;
    const Eigen::Vector<Jet12, 3> velAB = B_vel - A_vel;
    const Jet12 spring_length = AB.norm();
    const Eigen::Vector<Jet12, 3> spring_dir = AB / spring_length;
    return -spring_dir * (k * (spring_length - relaxed_length) + d * (spring_dir.dot(velAB)));
}

void computeVibrations()
{
    // Input: Two point masses connected by a single spring.
    // F = k*u + d*u'  spring with damping, u is length displacement of spring, k is spring constand, d is damping.
    // F = m*a
    // Linearize spring equation wrt. x,y,z and their derivatives.
    // X' = AX + g(t)  where X = [x0, y0, z0, x0', y0', z0', x1, y1, z1, x1', y1', z1', ...]
    // A is a sparse square matrix containing the coefficients of the linear equations.
    // Compute the complex eigenvalues λ and eigenvectors η of A to get the general solution.
    // (Some extra work is needed to handle repeated eigenvalues)
    // X(t) = c1 * exp(λ1*t) * η1 + c2 * exp(λ2*t) * η2 + ...
    // Where c1, c2, etc are real valued constants, λ1, λ2 are complex eigenvalues and
    // η1, η2 are their respective eigenvectors.
    // Apply the initial conditions to solve for the constants and get the actual solution.

    // Output: Coefficients for a function of positions over time on the form:
    //   f(t) = c0 + c1 * t + c2 * t^2 + exp(c3 * t) * (c5 * cos(c4 * t) + c6 * sin(c4 * t))



    using namespace Eigen;
    using std::complex;

    const size_t num_points = 2;
    const size_t num_springs = 1;
    // SparseMatrix<double> A(6 * num_points, 6 * num_points);
    MatrixXd A = MatrixXd::Zero(6 * num_points, 6 * num_points);

    // Equations for variable substitutions
    for (int i = 0; i < num_points; ++i)
    {
        const int point_start_index = i * 6;
        for (int j = 0; j < 3; ++j)
        {
            // "velocity is velocity"
            A(point_start_index + j, point_start_index + 3 + j) = 1.0;
        }
    }

    // Equations for springs
    Eigen::Vector<Jet12, 3> A_pos{
        {{0, 12, 0}},
        {{0, 12, 1}},
        {{0, 12, 2}},
    };
    Eigen::Vector<Jet12, 3> A_vel{
        {{0, 12, 3}},
        {{0, 12, 4}},
        {{0, 12, 5}},
    };
    Eigen::Vector<Jet12, 3> B_pos{
        {{0, 12, 6}},
        {{0, 12, 7}},
        {{0, 12, 8}},
    };
    Eigen::Vector<Jet12, 3> B_vel{
        {{0, 12, 9}},
        {{0, 12, 10}},
        {{0, 12, 11}},
    };
    for (int i = 0; i < num_springs; ++i)
    {
        const int point_a_start_index = i * 6;
        const int point_b_start_index = (i + 1) * 6;

        A_pos(0).value() = 0.5;
        B_pos(0).value() = 1.5;
        const double relaxed_length = 0.5;
        const double k = 100;
        const double d = 10;
        const double point_a_mass = 1.0;
        const double point_b_mass = 0.2;

        const Vector<Jet12, 3> force = SpringForce(A_pos, A_vel, B_pos, B_vel, relaxed_length, k, d);

        for (int j = 0; j < 3; ++j)
        {
            // Acceleration is force divided by mass. Force is defined by the spring equation.
            A.block<1, 6>(point_a_start_index + 3 + j, point_a_start_index) += force(j).derivatives().topRows<6>().transpose() / point_a_mass;
            A.block<1, 6>(point_a_start_index + 3 + j, point_b_start_index) += force(j).derivatives().bottomRows<6>().transpose() / point_a_mass;
            A.block<1, 6>(point_b_start_index + 3 + j, point_a_start_index) -= force(j).derivatives().topRows<6>().transpose() / point_b_mass;
            A.block<1, 6>(point_b_start_index + 3 + j, point_b_start_index) -= force(j).derivatives().bottomRows<6>().transpose() / point_b_mass;
        }
    }

    /*
    // 6 variable substitions per point + 2 ends per spring affecting 2 points.
    A.reserve(num_points * 6 + num_springs * 2 * 6);

    for (int i = 0; i < 6 * num_points; ++i)
    {
        // Variable substitutions.
        A.insert(i, i + 3) = 1.0;
    }
    A.makeCompressed();
    */

    //MatrixXd A = MatrixXd::Random(6,6);
    LOG_S(INFO) << "Here is the matrix A:\n" << A;

    EigenSolver<MatrixXd> es(A);
    LOG_S(INFO) << "The eigenvalues of A are:\n" << es.eigenvalues();
    LOG_S(INFO) << "The matrix of eigenvectors, V, is:\n" << es.eigenvectors();
    complex<double> lambda = es.eigenvalues()[0];
    LOG_S(INFO) << "Consider the first eigenvalue, lambda = " << lambda;
    VectorXcd v = es.eigenvectors().col(0);
    LOG_S(INFO) << "If v is the corresponding eigenvector, then lambda * v = \n" << lambda * v;
    LOG_S(INFO) << "... and A * v = \n" << A.cast<complex<double> >() * v;

    MatrixXcd D = es.eigenvalues().asDiagonal();
    MatrixXcd V = es.eigenvectors();
    LOG_S(INFO) << "Finally, V * D * V^(-1) = \n" << V * D * V.inverse();
}

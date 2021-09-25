#include <complex>
#include <ostream>
#include <sstream>

#include <Eigen/Eigen>
#include <unsupported/Eigen/AutoDiff>
#include <unsupported/Eigen/MatrixFunctions>

#include "alstruct.hpp"
#include "physics.hpp"
#include "logging.hpp"

static const double TAU = 3.14159265358979323846264338327950288 * 2.0;

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

void computeVibrations(AlStruct& al)
{
    // Input: Two point masses connected by a single spring.
    // F = k*u + d*u'  spring with damping, u is length displacement of spring, k is spring constant, d is damping.
    // F = m*a
    // Linearize spring equation wrt. x,y,z and their derivatives.
    // X' = AX  where X = [x0, y0, z0, x0', y0', z0', x1, y1, z1, x1', y1', z1', ..., 1]
    // A is a sparse square matrix containing the coefficients of the linear equations.
    // The solution to X' = AX is X(t) = exp(At) * X(0)
    // exp(At) can be computed by eigen decomposition: A = V*D*V^-1 => exp(At) = exp(V*D*V^-1*t) = V*exp(D*t)*V^-1


    // Compute the complex eigenvalues λ and eigenvectors η of A to get the general solution.
    // (Some extra work is needed to handle repeated eigenvalues)
    // X(t) = c1 * exp(λ1*t) * η1 + c2 * exp(λ2*t) * η2 + ...
    // Where c1, c2, etc are real valued constants, λ1, λ2 are complex eigenvalues and
    // η1, η2 are their respective eigenvectors.
    // Apply the initial conditions to solve for the constants and get the actual solution.
    // X(0) = c1 * exp(0) * η1 + c2 * exp(0) * η2 + ...
    // X(0) = c1 * η1 + c2 * η2 + ...
    // X(0) = [η1, η2, ...] * c
    // [η1, η2, ...]^-1 * X(0) = c

    // Output: Coefficients for a function of positions over time on the form:
    //   f(t) = c0 + c1 * t + c2 * t^2 + exp(c3 * t) * (c5 * cos(c4 * t) + c6 * sin(c4 * t))



    using namespace Eigen;
    using std::complex;

    const size_t num_points = 3;
    const size_t num_springs = 2;
    const double point_masses[num_points] = {10.0e-3, 2.0e-3, 3.0e-3};
    const size_t N = 6 * num_points + 1;
    // SparseMatrix<double> A(6 * num_points, 6 * num_points);
    MatrixXd A = MatrixXd::Zero(N, N);
    VectorXd X0 = VectorXd::Zero(N);
    X0(0) = 0.5;
    X0(6) = 1.5;
    X0(12) = 2.0;
    X0(N - 1) = 1.0;

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

        for (int j = 0; j < 3; ++j)
        {
            A_pos(j).value() = X0(point_a_start_index + j);
            A_vel(j).value() = X0(point_a_start_index + j + 3);
            B_pos(j).value() = X0(point_b_start_index + j);
            B_vel(j).value() = X0(point_b_start_index + j + 3);
        }
        const double relaxed_length = 0.5;
        const double k = 10000;
        const double d = 0.03;
        const double point_a_mass = point_masses[i];
        const double point_b_mass = point_masses[i + 1];

        const Vector<Jet12, 3> force = SpringForce(A_pos, A_vel, B_pos, B_vel, relaxed_length, k, d);

        for (int j = 0; j < 3; ++j)
        {
            // Acceleration is force divided by mass. Force is defined by the spring equation.
            A.block<1, 6>(point_a_start_index + 3 + j, point_a_start_index) -= force(j).derivatives().topRows<6>().transpose() / point_a_mass;
            A.block<1, 6>(point_a_start_index + 3 + j, point_b_start_index) -= force(j).derivatives().bottomRows<6>().transpose() / point_a_mass;
            A.block<1, 6>(point_b_start_index + 3 + j, point_a_start_index) += force(j).derivatives().topRows<6>().transpose() / point_b_mass;
            A.block<1, 6>(point_b_start_index + 3 + j, point_b_start_index) += force(j).derivatives().bottomRows<6>().transpose() / point_b_mass;
            A.coeffRef(point_a_start_index + 3 + j, 6 * num_points) -= force(j).value() / point_a_mass;
            A.coeffRef(point_b_start_index + 3 + j, 6 * num_points) += force(j).value() / point_b_mass;
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

    // MatrixXd D = es.pseudoEigenvalueMatrix();
    // MatrixXd V = es.pseudoEigenvectors();
    MatrixXcd D = es.eigenvalues().asDiagonal();
    MatrixXcd V = es.eigenvectors();
    LOG_S(INFO) << "The eigenvalues of A are:\n" << D;
    LOG_S(INFO) << "The matrix of eigenvectors, V, is:\n" << V;
    LOG_S(INFO) << "V \\ X0 is:\n" << V.inverse() * X0;
    for (int i = 0; i < es.eigenvalues().rows(); ++i)
    {
        if (es.eigenvalues()(i).imag() != 0)
        {
            double frequency = std::abs(es.eigenvalues()(i).imag()) / TAU;
            LOG_S(INFO) << "Found vibrational mode with frequency: " << frequency << " hz:\n" << V.col(i);
        }
    }

    // complex<double> lambda = es.eigenvalues()[0];
    // LOG_S(INFO) << "Consider the first eigenvalue, lambda = " << lambda;
    // VectorXcd v = es.eigenvectors().col(0);
    // LOG_S(INFO) << "If v is the corresponding eigenvector, then lambda * v = \n" << lambda * v;
    // LOG_S(INFO) << "... and A * v = \n" << A.cast<complex<double> >() * v;

    // LOG_S(INFO) << "V * V^(-1) = \n" << round((V * V.inverse() * 1.0e5).array()) * 1.0e-5;
    // LOG_S(INFO) << "V * D * V^(-1) = \n" << round((V * D * V.inverse() * 1.0e5).array()) * 1.0e-5;
    // LOG_S(INFO) << "V^(-1) * A * V = \n" << round((V.inverse() * A * V * 1.0e5).array()) * 1.0e-5;
    LOG_S(INFO) << "V * V^(-1) = \n" << V * V.inverse();
    LOG_S(INFO) << "V * D * V^(-1) = \n" << V * D * V.inverse();
    LOG_S(INFO) << "V^(-1) * A * V = \n" << V.inverse() * A * V;

    MatrixXd expA = A.exp();
    LOG_S(INFO) << "exp(A) = \n" << expA;
    LOG_S(INFO) << "X0 = \n" << X0;
    LOG_S(INFO) << "exp(A)*X0 = \n" << expA * X0;
    const ALsizei sampling_frequency = 44100;
    const size_t number_of_samples = sampling_frequency * 5;
    const MatrixXd exp_tiny_A = (A / double(sampling_frequency)).exp();
    LOG_S(INFO) << "exp(A/num_samples) = \n" << exp_tiny_A;

    VectorXd samples(number_of_samples);
    VectorXd X = X0;
    for (int i = 0; i < number_of_samples; ++i)
    {
        VectorXd X_next = exp_tiny_A * X;
        samples(i) = X_next(3) - X(3);
        X = X_next;
    }
    LOG_S(INFO) << "X = \n" << X;
    // LOG_S(INFO) << "samples = \n" << samples.transpose();
    const double max_elem_size = samples.cwiseAbs().maxCoeff();
    const double scale = std::numeric_limits<int16_t>::max() * 0.5 / max_elem_size;
    LOG_S(INFO) << "max_elem_size = " << max_elem_size;
    LOG_S(INFO) << "scale = " << scale;
    std::vector<int16_t> buffer_data(number_of_samples);
    for (int i = 0; i < number_of_samples; ++i)
    {
        buffer_data[i] = static_cast<int16_t>(samples(i) * scale);
    }
    for (int i = 0; i + 1 < number_of_samples; ++i)
    {
        if (buffer_data[i] == 0 && buffer_data[i + 1] == 0)
        {
            LOG_F(INFO, "buffer ends after %d of %d samples", i, number_of_samples);
            break;
        }
    }

    if (al.LoadSoundFromBuffer("generated_sound", buffer_data.data(), buffer_data.size(), sampling_frequency))
    {
        ALuint source = al.AddSound("generated_sound", Vec3r(0, 0, 0));
        alSourcePlay(source);
    }
}

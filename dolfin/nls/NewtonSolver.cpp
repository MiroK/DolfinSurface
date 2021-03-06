// Copyright (C) 2005-2008 Garth N. Wells
//
// This file is part of DOLFIN.
//
// DOLFIN is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// DOLFIN is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with DOLFIN. If not, see <http://www.gnu.org/licenses/>.
//
// Modified by Anders Logg, 2005-2009.
// Modified by Martin Alnes, 2008.
// Modified by Johan Hake, 2010.
//
// First added:  2005-10-23
// Last changed: 2011-03-29

#include <iostream>
#include <dolfin/common/constants.h>
#include <dolfin/common/NoDeleter.h>
#include <dolfin/la/GenericLinearSolver.h>
#include <dolfin/la/LinearSolver.h>
#include <dolfin/la/Matrix.h>
#include <dolfin/la/Vector.h>
#include <dolfin/log/log.h>
#include <dolfin/log/dolfin_log.h>
#include <dolfin/common/MPI.h>
#include "NonlinearProblem.h"
#include "NewtonSolver.h"

using namespace dolfin;

//-----------------------------------------------------------------------------
Parameters NewtonSolver::default_parameters()
{
  Parameters p("newton_solver");

  p.add("maximum_iterations",      10);
  p.add("relative_tolerance",      1e-9);
  p.add("absolute_tolerance",      1e-10);
  p.add("convergence_criterion",   "residual");
  p.add("method",                  "full");
  p.add("relaxation_parameter",    1.0);
  p.add("report",                  true);
  p.add("error_on_nonconvergence", true);

  //p.add("reuse_preconditioner", false);

  return p;
}
//-----------------------------------------------------------------------------
NewtonSolver::NewtonSolver(std::string solver_type, std::string pc_type)
  : Variable("Newton solver", "unamed"),
    newton_iteration(0), _residual(0.0), residual0(0.0),
    _solver(new LinearSolver(solver_type, pc_type)),
    _A(new Matrix), _dx(new Vector), _b(new Vector)
{
  // Set default parameters
  parameters = default_parameters();
}
//-----------------------------------------------------------------------------
NewtonSolver::NewtonSolver(boost::shared_ptr<GenericLinearSolver> solver,
                           GenericLinearAlgebraFactory& factory)
  : Variable("Newton solver", "unamed"),
    newton_iteration(0), _residual(0.0), residual0(0.0),
    _solver(solver),
    _A(factory.create_matrix()),
    _dx(factory.create_vector()),
    _b(factory.create_vector())
{
  // Set default parameters
  parameters = default_parameters();
}
//-----------------------------------------------------------------------------
NewtonSolver::~NewtonSolver()
{
  // Do nothing
}
//-----------------------------------------------------------------------------
std::pair<std::size_t, bool> NewtonSolver::solve(NonlinearProblem& nonlinear_problem,
                                                  GenericVector& x)
{
  dolfin_assert(_A);
  dolfin_assert(_b);
  dolfin_assert(_dx);
  dolfin_assert(_solver);

  const std::string convergence_criterion = parameters["convergence_criterion"];
  const std::size_t maxiter = parameters["maximum_iterations"];

  std::size_t krylov_iterations = 0;
  newton_iteration = 0;

  // Compute F(u)
  nonlinear_problem.F(*_b, x);

  // Check convergence
  bool newton_converged = false;
  if (convergence_criterion == "residual")
    newton_converged = converged(*_b, nonlinear_problem);
  else if (convergence_criterion == "incremental")
  {
    // We need to do at least one Newton step
    // with the ||dx||-stopping criterion.
    newton_converged = false;
  }
  else
  {
    dolfin_error("NewtonSolver.cpp",
                 "check for convergence",
                 "The convergence criterion %s is unknown, known criteria are 'residual' or 'incremental'", convergence_criterion.c_str());
  }

  nonlinear_problem.form(*_A, *_b, x);

  // Start iterations
  while (!newton_converged && newton_iteration < maxiter)
  {
    // Compute Jacobian
    nonlinear_problem.J(*_A, x);

    // FIXME: This reset is a hack to handle a deficiency in the Trilinos wrappers
    _solver->set_operator(_A);

    // Perform linear solve and update total number of Krylov iterations
    if (!_dx->empty())
      _dx->zero();
    krylov_iterations += _solver->solve(*_dx, *_b);

    // Update solution
    const double relaxation = parameters["relaxation_parameter"];
    if (std::abs(1.0 - relaxation) < DOLFIN_EPS)
      x -= (*_dx);
    else
      x.axpy(-relaxation, *_dx);

    // FIXME: this step is not needed if residual is based on dx and
    //        this has converged.
    // Compute F
    nonlinear_problem.form(*_A, *_b, x);
    nonlinear_problem.F(*_b, x);

    // Test for convergence
    if (convergence_criterion == "residual")
    {
      ++newton_iteration;
      newton_converged = converged(*_b, nonlinear_problem);
    }
    else if (convergence_criterion == "incremental")
    {
      // Increment the number of iterations *after* converged(). This
      // makes sure that the initial residual0 is properly set.
      newton_converged = converged(*_dx, nonlinear_problem);
      ++newton_iteration;
    }
    else
      dolfin_error("NewtonSolver.cpp",
                   "check for convergence",
                   "The convergence criterion %s is unknown, known criteria are 'residual' or 'incremental'", convergence_criterion.c_str());
  }

  if (newton_converged)
  {
    if (dolfin::MPI::process_number() == 0)
    {
     info("Newton solver finished in %d iterations and %d linear solver iterations.",
            newton_iteration, krylov_iterations);
    }
  }
  else
  {
    const bool error_on_nonconvergence = parameters["error_on_nonconvergence"];
    if (error_on_nonconvergence)
    {
      dolfin_error("NewtonSolver.cpp",
                   "solve nonlinear system with NewtonSolver",
                   "Newton solver did not converge. Bummer");
    }
    else
      warning("Newton solver did not converge.");
  }

  return std::make_pair(newton_iteration, newton_converged);
}
//-----------------------------------------------------------------------------
std::size_t NewtonSolver::iteration() const
{
  return newton_iteration;
}
//-----------------------------------------------------------------------------
double NewtonSolver::residual() const
{
  return _residual;
}
//-----------------------------------------------------------------------------
double NewtonSolver::relative_residual() const
{
  return _residual/residual0;
}
//-----------------------------------------------------------------------------
GenericLinearSolver& NewtonSolver::linear_solver() const
{
  dolfin_assert(_solver);
  return *_solver;
}
//-----------------------------------------------------------------------------
bool NewtonSolver::converged(const GenericVector& r,
                             const NonlinearProblem& nonlinear_problem)
{
  const double rtol = parameters["relative_tolerance"];
  const double atol = parameters["absolute_tolerance"];
  const bool report = parameters["report"];

  _residual = r.norm("l2");

  // If this is the first iteration step, set initial residual
  if (newton_iteration == 0)
    residual0 = _residual;

  // Relative residual
  const double relative_residual = _residual / residual0;

  // Output iteration number and residual
  if (report && dolfin::MPI::process_number() == 0)
  {
    info("Newton iteration %d: r (abs) = %.3e (tol = %.3e) r (rel) = %.3e (tol = %.3e)",
         newton_iteration, _residual, atol, relative_residual, rtol);
  }

  // Return true of convergence criterion is met
  if (relative_residual < rtol || _residual < atol)
    return true;

  // Otherwise return false
  return false;
}
//-----------------------------------------------------------------------------

#include <cmath>
#include <iostream>
#include <iomanip>
#include "Meshes.h"
#include "Advection.h"
#include "Jacobian.h"
#include "TimeIntegration.h"
#include "Preconditioners.h"

struct ExactGaussian : FnFunctor
{
    double theta;
    
    double operator()(double x, double y) const
    {
        double x0, y0;
        
        x0 = 0.5 - 0.15*cos(2*theta);
        y0 = 0.5 + 0.3*cos(theta)*sin(theta);
        
        return exp(-150*((x-x0)*(x-x0) + (y-y0)*(y-y0)));
    }
    
    ExactGaussian(double a_theta) : theta(a_theta) {};
};

double planarWave(double x, double y)
{
    int nx = 10;
    int ny = 10;
    return sin(nx*x*2*M_PI + ny*y*2*M_PI);
}

int main(int argc, char ** argv)
{
    using namespace std;
    
    int deg = 0;
    double h = 0.05;
    
    cout << "Create mesh with h = " << h << endl;
    
    //PolyMesh msh = quadUnitSquare(h*0.5*sqrt(sqrt(3)));
    //PolyMesh msh = triUnitSquare(h*sqrt(sqrt(3)/2));
    //PolyMesh msh = honeycombUnitSquare(h);
    PolyMesh msh = hexUnitSquare(h/sqrt(6.0));
    
    cout << "Total degrees of freedom: " << msh.np*0.5*(deg+1)*(deg+2) << endl;
    
    msh.gnuplot();
    
    MassMatrix M(msh, deg);
    M.spy("plt/M.gnu");
    Advection eqn(msh);
    
    //FnCallbackFunctor exact(planarWave);
    ExactGaussian exact(0);
    
    MeshFn f = MeshFn(msh, deg, 1);
    f.interp(exact);
    
    cout << "Computing Jacobian matrix" << endl;
    Jacobian B = eqn.jacobian(f, 0);
    B.spy("plt/J.gnu");
    
    MeshFn unp1 = f;
    
    int K = 10;
    int i;
    //double dt = h*2;
    double dt = h;
    
    cout << "Using dt = " << dt << endl;
    cout << "Computing total of " << K << " timesteps." << endl;
    
    B *= -dt;
    B += M;
    
    B.spy("plt/B.gnu");
    
    //BlockILU0 pc(B);
    BlockJacobi pc(B);
    
    //BackwardEuler ti(M, eqn);
    
    for (i = 0; i < K; i++)
    {
        if (i%10 == 0)
            cout << "Beginning timestep " << i << ", t=" << i*dt << endl;
        
        unp1.gnuplot("plt/u" + to_string(i) + ".gnu");
        unp1 = B.solve(M.dot(unp1), pc, kJacobiSolver);
        //unp1 = ti.advance(unp1, dt, i*dt);
    }
    unp1.gnuplot("plt/u" + to_string(i) + ".gnu");
    
    cout << setprecision(20) << "Computed until final time t=" << i*dt << endl;
    
    double l2err;
    
    exact.theta = i*dt;
    l2err = unp1.L2Error(exact);
    cout << "L^2 error = " << l2err << endl;
    
    return 0;
}

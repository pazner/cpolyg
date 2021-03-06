#include "BoundaryConditions.h"
#include "Legendre.h"

using namespace std;
using namespace arma;

mat BoundaryConditions::boundaryValue(double x, double y, mat U, int b)
{
    switch(bcTypes.at(b))
    {
        case kDirichletCondition:
            dirichlet.at(b)->t = t;
            return (*dirichlet.at(b))(x, y);
        case kPeriodicCondition:
        {
            double xLocal, yLocal;
            int nc = U.n_cols;
            int m = 0.5*(sqrt(4*U.n_rows +1) - 1);
            mat result(nc, 1);
            
            msh.getPeriodicCoordinates(b, x, y, xLocal, yLocal);
            
            for (size_t c = 0; c < U.n_cols; c++)
            {
                result(c) = Leg2D(xLocal, yLocal, m, U.col(c));
            }
            
            return result;
        }
        default:
        {
            // should never get here
            abort();
            return mat();
        }
    }
}

BoundaryConditions BoundaryConditions::dirichletConditions(PolyMesh &a_msh, 
    VecFunctor *fn)
{
    BoundaryConditions bc(a_msh);
    
    bc.bcTypes.emplace(-1, kDirichletCondition);
    bc.dirichlet.emplace(-1, fn);
    
    return bc;
}

BoundaryConditions BoundaryConditions::periodicConditions(PolyMesh &a_msh)
{
    BoundaryConditions bc(a_msh);
    
    for (size_t i = 0; i < a_msh.bc.size(); i++)
    {
        bc.bcTypes.emplace(-1 - i, kPeriodicCondition);
    }
    
    return bc;
}


#ifndef VSAR_SOLVEOPERATION_H_INCLUDED
#define VSAR_SOLVEOPERATION_H_INCLUDED

#include <string>
//#include <boost/scoped_ptr.hpp>

namespace Vsar
{
    class SolveOperation
    {
    public:
        SolveOperation();
        ~SolveOperation();

        void Execute();
    protected:
        void CleanResult();
        void LoadResult();
    };
}

#endif //VSAR_SOLVEOPERATION_H_INCLUDED

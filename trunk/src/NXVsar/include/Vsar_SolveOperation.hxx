
#ifndef VSAR_SOLVEOPERATION_H_INCLUDED
#define VSAR_SOLVEOPERATION_H_INCLUDED

#include <string>
//#include <boost/scoped_ptr.hpp>

namespace Vsar
{
    class BaseSolveOperation
    {
    public:
        BaseSolveOperation();
        virtual ~BaseSolveOperation() = 0;

        void Execute();
    protected:
        void CreateWorkDir();

        void CleanResult();
        void LoadResult();

    protected:
        std::string m_workDir;
        std::string m_scratchDir;
        std::string m_solDir;
    };

    class SolveResponseOperation : public BaseSolveOperation
    {
    public:
        SolveResponseOperation();
        ~SolveResponseOperation();

    };
}

#endif //VSAR_SOLVEOPERATION_H_INCLUDED


#ifndef VSAR_SOLVEOPERATION_H_INCLUDED
#define VSAR_SOLVEOPERATION_H_INCLUDED

//#include <string>
//#include <boost/scoped_ptr.hpp>
#include <boost/filesystem.hpp>

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

        virtual void PreExecute() = 0;

        virtual void PrepareInputFiles() const = 0;

    protected:
        boost::filesystem::path   m_workDir;
        boost::filesystem::path   m_solDir;
    };

    class SolveResponseOperation : public BaseSolveOperation
    {
    public:
        SolveResponseOperation();
        ~SolveResponseOperation();

    protected:
        void PreExecute();

        void PrepareInputFiles() const;
    };
}

#endif //VSAR_SOLVEOPERATION_H_INCLUDED

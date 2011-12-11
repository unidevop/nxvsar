
#ifndef VSAR_SOLVEOPERATION_H_INCLUDED
#define VSAR_SOLVEOPERATION_H_INCLUDED

#include <string>
#include <vector>
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

    class ExcitationInput
    {
    public:
        struct InputItem
        {
            std::string  m_partName;
            std::string  m_expName;
            //  conversion
            std::string  m_targetUnitName;  // to SI Unit
        };

        typedef std::vector<InputItem>  StlInputItemVector;

    public:
        ExcitationInput(const boost::filesystem::path &targetDir);

        ~ExcitationInput();

        void Generate() const;

    protected:

        void CopyIrrData() const;
        void WriteVehicleData() const;
        void WriteRailData() const;
        void WriteSlabData() const;
        void WriteBeamData() const;
        void WriteCalculationData() const;

        void WriteInputData(const StlInputItemVector &vInputItems, const std::string &fileName) const;
    private:
        boost::filesystem::path   m_targetDir;
    };
}

#endif //VSAR_SOLVEOPERATION_H_INCLUDED

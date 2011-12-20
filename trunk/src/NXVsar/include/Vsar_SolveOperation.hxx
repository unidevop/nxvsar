
#ifndef VSAR_SOLVEOPERATION_H_INCLUDED
#define VSAR_SOLVEOPERATION_H_INCLUDED

#include <string>
#include <vector>
#include <boost/filesystem.hpp>

namespace NXOpen
{
    class TaggedObject;

    namespace CAE
    {
        class FEModelOccurrence;
        class FENode;
    }
}

namespace Vsar
{
    class BaseSolveOperation
    {
    public:
        BaseSolveOperation();
        virtual ~BaseSolveOperation() = 0;

        void Execute();

        boost::filesystem::path GetWorkDir() const
        {
            return m_workDir;
        }

        boost::filesystem::path GetSolutionDir() const
        {
            return m_solDir;
        }

        virtual void LoadResult() = 0;

        virtual void CleanResult() = 0;

    protected:
        void CreateWorkDir();

        void CleanAfuFile(const std::string &resultName);

        virtual void PreExecute() = 0;

        virtual void Solve() = 0;

        //virtual void PrepareInputFiles() const = 0;

    protected:
        boost::filesystem::path   m_workDir;
        boost::filesystem::path   m_solDir;
    };

    class BaseTask
    {
    public:
        BaseTask(const BaseSolveOperation *solOper);
        virtual ~BaseTask() = 0;

        virtual void Run();

        virtual std::vector<std::string> GetOutputResults() const = 0;

    protected:

        virtual void CleanResults() const;

        virtual void PrepareInput() = 0;

        virtual void CallExecutable() const;

        virtual void PostSolveCheck() const;

        virtual void MoveOutputs() const;

        virtual std::string GetExecutableName() const = 0;

        virtual std::string GetSuccessLog() const = 0;

        virtual std::string GetFailLog() const = 0;

    protected:
        const BaseSolveOperation  *m_solOper;
    };

    class ComputeExcitationTask : public BaseTask
    {
    public:
        ComputeExcitationTask(const BaseSolveOperation *solOper);
        ~ComputeExcitationTask();

        virtual std::vector<std::string> GetOutputResults() const;

    protected:

        virtual void PrepareInput();

        virtual std::string GetExecutableName() const;

        virtual std::string GetSuccessLog() const;

        virtual std::string GetFailLog() const;
    };

    class ConvertExcitationTask : public BaseTask
    {
    public:
        ConvertExcitationTask(const BaseSolveOperation *solOper);
        ~ConvertExcitationTask();

        virtual std::vector<std::string> GetOutputResults() const;

    protected:

        virtual void PrepareInput();

        virtual std::string GetExecutableName() const;

        virtual std::string GetSuccessLog() const;

        virtual std::string GetFailLog() const;

        NXOpen::CAE::FEModelOccurrence* GetRailFEModelOcc() const;

        std::vector<tag_t> GetRailNodes();

        void WriteInputData(std::vector<tag_t> &railNodes) const;

    private:
        int     m_nodeOffset;
    };

    class SolveResponseOperation : public BaseSolveOperation
    {
    public:
        SolveResponseOperation();
        virtual ~SolveResponseOperation();

        virtual void LoadResult();

        virtual void CleanResult();

    protected:
        virtual void PreExecute();

        virtual void Solve();

    private:
        ComputeExcitationTask  m_computeExcitation;
        ConvertExcitationTask  m_convertExcitation;
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

    class SolveSettings
    {
    public:
        SolveSettings(bool bOutputElems, const std::vector<NXOpen::TaggedObject*> &outputElems,
            bool bOutputNodes, const std::vector<NXOpen::TaggedObject*> &outputNodes,
            bool bOutputNodesForNoise);

        ~SolveSettings()
        {
        }

        void Apply();
    protected:
        void SetEntityGroup(const std::string &groupName,
            const std::vector<NXOpen::TaggedObject*> &outputEntities);

        void SetNoiseOutput();

        void SetTimeStep();

    private:
        bool m_bOutputElems;
        std::vector<NXOpen::TaggedObject*>  m_outputElems;

        bool m_bOutputNodes;
        std::vector<NXOpen::TaggedObject*>  m_outputNodes;

        bool m_bOutputNodesForNoise;
    };
}

#endif //VSAR_SOLVEOPERATION_H_INCLUDED

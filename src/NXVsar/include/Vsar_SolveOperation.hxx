
#ifndef VSAR_SOLVEOPERATION_H_INCLUDED
#define VSAR_SOLVEOPERATION_H_INCLUDED

#include <string>
#include <vector>
#include <boost/filesystem.hpp>

namespace NXOpen
{
    class TaggedObject;
    class Point;

    namespace CAE
    {
        class FEModelOccurrence;
        class FENode;
    }
}

namespace Vsar
{
    //////////////////////////////////////////////////////////////////////////
    // Solve Operations
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

        void CleanResultFile(const std::string &resultName);

        virtual void PreExecute() = 0;

        virtual void Solve() = 0;

        virtual bool CanAutoLoadResult() const = 0;

        //virtual void PrepareInputFiles() const = 0;

    protected:
        boost::filesystem::path   m_workDir;
        boost::filesystem::path   m_solDir;
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

        virtual bool CanAutoLoadResult() const;
    };

    class SolveNoiseOperation : public BaseSolveOperation
    {
    public:
        SolveNoiseOperation(const std::vector<NXOpen::Point*> &pts);
        virtual ~SolveNoiseOperation();

        virtual void LoadResult();

        virtual void CleanResult();

    protected:
        virtual void PreExecute();

        virtual void Solve();

        virtual bool CanAutoLoadResult() const;

    private:
        std::vector<NXOpen::Point*>    m_outputPoints;
    };


    //////////////////////////////////////////////////////////////////////////
    // Compute Tasks
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


    //////////////////////////////////////////////////////////////////////////
    // excitation input

    class BaseExeInput
    {
    public:
        BaseExeInput(const boost::filesystem::path &targetDir) : m_targetDir(targetDir)
        {
        }

        virtual ~BaseExeInput()
        {
        }

        virtual void Generate() const = 0;

    protected:
        boost::filesystem::path   m_targetDir;
    };

    class ExcitationInput : public BaseExeInput
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
        ExcitationInput(const boost::filesystem::path &targetDir) : BaseExeInput(targetDir)
        {
        }

        virtual ~ExcitationInput()
        {
        }

        virtual void Generate() const;

    protected:

        void CopyIrrData() const;
        void WriteVehicleData() const;
        void WriteRailData() const;
        void WriteSlabData() const;
        void WriteBeamData() const;
        void WriteCalculationData() const;

        void WriteInputData(const StlInputItemVector &vInputItems, const std::string &fileName) const;
    };

    //////////////////////////////////////////////////////////////////////////
    // Noise input
    class NoiseInput : public BaseExeInput
    {
    public:
        NoiseInput(const boost::filesystem::path &targetDir,
                  const std::vector<NXOpen::Point*> &pts) : BaseExeInput(targetDir), m_outputPoints(pts), m_refNodeSeq()
        {
            ConstructRefNodeSequence();
        }

        virtual ~NoiseInput()
        {
        }

        virtual void Generate() const;

    protected:
        void ConstructRefNodeSequence();

        std::string GetIntermediateResult() const;

        std::string GetTargetInputName(const std::string &recordName) const;

        void WriteFrequenceData() const;

        void WriteRecord(const std::string &recordName, const std::vector<double> &freqVals,
                         const std::vector<double> &yReals, const std::vector<double> &yImags) const;

        void WriteOutputPoints() const;

        NXOpen::Point* GetSlabCenter() const;

    private:
        const std::vector<NXOpen::Point*> &m_outputPoints;

        std::vector<NXOpen::TaggedObject*>  m_refNodeSeq;
    };

    //////////////////////////////////////////////////////////////////////////
    // Solver settings
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


#ifndef VSAR_RESULT_H_INCLUDED
#define VSAR_RESULT_H_INCLUDED

#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/filesystem.hpp>

#include <NXOpen\CAE_XYFunctionDataTypes.hxx>

namespace NXOpen
{
    class Point;
    //namespace CAE
    //{
    //    class AfuData;
    //}
}

namespace Vsar
{
    class ResultBlock;

    class BaseResult
    {
    public:
        BaseResult()
        {
        }

        virtual ~BaseResult() = 0;

        bool IsResultExist() const;

        bool Load() const;

        virtual std::string GetResultPathName() const = 0;
    };

    class ResponseOp2Result : public BaseResult
    {
    public:
        ResponseOp2Result() : BaseResult()
        {
        }

        virtual ~ResponseOp2Result()
        {
        }

        virtual std::string GetResultPathName() const;
    };

    class BaseAfuResult : public BaseResult
    {
    public:
        BaseAfuResult()
        {
        }

        virtual ~BaseAfuResult()
        {
        }

        void Create();

    protected:

        void CreateResultFile();

        virtual void CreateRecords() = 0;
    };

    struct ResponseRecordItem;

    class ResponseAfuResult : public BaseAfuResult
    {
    public:
        ResponseAfuResult(const std::string &inputPath) : BaseAfuResult(), m_inputPath(inputPath)
        {
        }

        virtual ~ResponseAfuResult()
        {
        }

        virtual std::string GetResultPathName() const;

    protected:
        virtual void CreateRecords();

        void CreateRecord(const ResponseRecordItem &recordItem);

        void ReadDataFromDat(const ResponseRecordItem &recordItem,
            std::vector<double> &xValues, std::vector<double> &yValues) const;

        //virtual StlResultBlockVector ExtractContent(std::ifstream &iResult);

    private:
        std::string   m_inputPath;
    };

    typedef std::vector<boost::shared_ptr<ResultBlock>> StlResultBlockVector;

    class NastranResult : public BaseAfuResult
    {
    public:
        NastranResult() : BaseAfuResult()
        {
        }

        virtual ~NastranResult()
        {
        }

    protected:
        std::string GetNastranResultPathName() const;

        virtual void CreateRecords();

        virtual StlResultBlockVector ExtractContent(std::ifstream &iResult) = 0;

        template<typename BlockType>
        StlResultBlockVector ReadDataBlock(std::ifstream &ifStream);
    };

#if 0
    class ResponseResult : public NastranResult
    {
    public:
        ResponseResult() : NastranResult()
        {
        }

        virtual ~ResponseResult()
        {
        }

        virtual std::string GetResultPathName() const;

    protected:

        virtual StlResultBlockVector ExtractContent(std::ifstream &iResult);
    };
#endif

    class NoiseIntermResult : public NastranResult
    {
    public:
        NoiseIntermResult() : NastranResult()
        {
        }

        virtual ~NoiseIntermResult()
        {
        }

        virtual std::string GetResultPathName() const;

    protected:

        virtual StlResultBlockVector ExtractContent(std::ifstream &iResult);
    };

    class NoiseResult : public BaseAfuResult
    {
    public:
        NoiseResult(const boost::filesystem::path &srcDir,
                    const std::vector<NXOpen::Point*> &pts) : BaseAfuResult(),
                    m_srcDir(srcDir), m_outputPoints(pts)
        {
        }

        virtual ~NoiseResult()
        {
        }

        virtual std::string GetResultPathName() const;

    protected:

        virtual void CreateRecords();

        void WriteRecord(const std::string &recordName, NXOpen::CAE::XyFunctionDataType funcType,
                         NXOpen::CAE::XyFunctionUnit xUnit, NXOpen::CAE::XyFunctionUnit yUnit,
                         const std::vector<double> &xValues, const std::vector<double> &yValues);

#if 0
        void WriteRecord(const std::string &recordName, NXOpen::CAE::XyFunctionDataType funcType,
                         NXOpen::CAE::XyFunctionUnit xUnit, NXOpen::CAE::XyFunctionUnit yUnit,
                         const std::vector<double> &xValues,
                         const std::vector<double> &yRealValues, const std::vector<double> &yImagValues);
#endif

        void ReadDataFromDat(const std::string &noiseOutputName,
            std::vector<double> &xValues, std::vector<double> &yValues) const;

        void OutputTimeRecord(int idxRecord);

        void OutputFreqRecord(int idxRecord);
    private:

        boost::filesystem::path   m_srcDir;
        const std::vector<NXOpen::Point*> &m_outputPoints;
    };

    class ResultsLoader
    {
    public:
        ResultsLoader()
        {
        }

        ~ResultsLoader()
        {
        }

        void operator () ();
    };
}

#endif //VSAR_RESULT_H_INCLUDED

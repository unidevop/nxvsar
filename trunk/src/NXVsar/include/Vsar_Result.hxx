
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

        void Create();

        bool IsResultExist() const;

        virtual std::string GetResultPathName() const = 0;

        std::string GetNastranResultPathName() const;

    protected:

        void CreateResultFile();

        virtual void CreateRecords() = 0;
    };

    struct ResponseRecordItem;

    typedef std::vector<boost::shared_ptr<ResultBlock>> StlResultBlockVector;

    class NastranResult : public BaseResult
    {
    public:
        NastranResult() : BaseResult()
        {
        }

        virtual ~NastranResult()
        {
        }

    protected:
        virtual void CreateRecords();

        virtual StlResultBlockVector ExtractContent(std::ifstream &iResult) = 0;

        template<typename BlockType>
        StlResultBlockVector ReadDataBlock(std::ifstream &ifStream);

    private:

        //void CreateRecord(const ResponseRecordItem &recordItem);

        //void ReadDataFromDat(const ResponseRecordItem &recordItem,
        //                     std::vector<double> &xValues, std::vector<double> &yValues) const;
    };

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

    class NoiseResult : public BaseResult
    {
    public:
        NoiseResult(const boost::filesystem::path &srcDir,
                    const std::vector<NXOpen::Point*> &pts) : BaseResult(),
                    m_srcDir(srcDir), m_outputPoints(pts)
        {
        }

        virtual ~NoiseResult()
        {
        }

        virtual std::string GetResultPathName() const;

    protected:

        virtual void CreateRecords();

        void WriteRecord(const std::string &noiseOutputName, const std::string &recordName,
                         NXOpen::CAE::XyFunctionDataType funcType, NXOpen::CAE::XyFunctionUnit xUnit, NXOpen::CAE::XyFunctionUnit yUnit);

        void ReadDataFromDat(const std::string &noiseOutputName,
            std::vector<double> &xValues, std::vector<double> &yValues) const;

    private:

        boost::filesystem::path   m_srcDir;
        const std::vector<NXOpen::Point*> &m_outputPoints;
    };
}

#endif //VSAR_RESULT_H_INCLUDED


#ifndef VSAR_RESULT_H_INCLUDED
#define VSAR_RESULT_H_INCLUDED

#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>

//namespace NXOpen
//{
//    namespace CAE
//    {
//        class AfuData;
//    }
//}

namespace Vsar
{
    class ResultBlock;

    class BaseResult
    {
    public:
        BaseResult();
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
        NastranResult();
        virtual ~NastranResult();

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
        ResponseResult();
        virtual ~ResponseResult();

        virtual std::string GetResultPathName() const;

    protected:

        virtual StlResultBlockVector ExtractContent(std::ifstream &iResult);
    };

    class NoiseIntermResult : public NastranResult
    {
    public:
        NoiseIntermResult();
        virtual ~NoiseIntermResult();

        virtual std::string GetResultPathName() const;

    protected:

        virtual StlResultBlockVector ExtractContent(std::ifstream &iResult);
    };
}

#endif //VSAR_RESULT_H_INCLUDED

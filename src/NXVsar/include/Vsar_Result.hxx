
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
    protected:

        void CreateResultFile();

        virtual void CreateRecords() = 0;
    };

    struct ResponseRecordItem;

    typedef std::vector<boost::shared_ptr<ResultBlock>> StlResultBlockVector;

    class ResponseResult : public BaseResult
    {
    public:
        ResponseResult();
        virtual ~ResponseResult();

        virtual std::string GetResultPathName() const;

    protected:
        void CreateRecords();

        template<typename BlockType>
        StlResultBlockVector ReadDataBlock(std::ifstream &ifStream);

    private:

        void CreateRecord(const ResponseRecordItem &recordItem);

        void ReadDataFromDat(const ResponseRecordItem &recordItem,
                             std::vector<double> &xValues, std::vector<double> &yValues) const;
    };
}

#endif //VSAR_RESULT_H_INCLUDED

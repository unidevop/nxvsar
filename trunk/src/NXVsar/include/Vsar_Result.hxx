
#ifndef VSAR_RESULT_H_INCLUDED
#define VSAR_RESULT_H_INCLUDED

#include <string>
#include <vector>
//#include <boost/scoped_ptr.hpp>

//namespace NXOpen
//{
//    namespace CAE
//    {
//        class AfuData;
//    }
//}

namespace Vsar
{
    class BaseResult
    {
    public:
        BaseResult();
        virtual ~BaseResult() = 0;

        void Load();

        bool IsResultExist() const;

        std::string GetResultPathName() const
        {
            return m_resultFullName;
        }
    protected:

        void CreateResultFile();

        virtual void CreateRecords() = 0;
    private:
        std::string     m_resultFullName;
    };

    struct ResponseRecordItem;

    class ResponseResult : public BaseResult
    {
    public:
        ResponseResult();
        virtual ~ResponseResult();

    protected:
        void CreateRecords();

    private:

        void CreateRecord(const ResponseRecordItem &recordItem);

        void ReadDataFromDat(const ResponseRecordItem &recordItem,
                             std::vector<double> &xValues, std::vector<double> &yValues) const;
    };
}

#endif //VSAR_RESULT_H_INCLUDED

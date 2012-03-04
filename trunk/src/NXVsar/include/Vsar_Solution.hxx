
#ifndef VSAR_SOLUTION_H_INCLUDED
#define VSAR_SOLUTION_H_INCLUDED

#include <Vsar_Component.hxx>

namespace Vsar
{
    class BaseSolution : public BaseComponent
    {
    public:
        virtual ~BaseSolution() = 0;

        virtual void OnInit();

    protected:
        inline BaseSolution() : BaseComponent(NULL, 0)
        {
        }

        inline BaseSolution(const CompAttrInfo *pCompAttrs, int compAttrCnt) : BaseComponent(pCompAttrs, compAttrCnt)
        {
        }

        virtual bool HasGeometryDependency() const;

        virtual bool CanUpdateRailSlabFEModel() const;

        virtual bool CanUpdateBraseFEModel() const;

        virtual bool CanUpdateRailSlabConnection() const;

        virtual bool CanUpdateBraseConnection() const;

    private:
    };

    class ResponseSolution : public BaseSolution
    {
    public:
        ResponseSolution();
        virtual ~ResponseSolution();
    };

    class NoiseSolution : public BaseSolution
    {
    public:
        NoiseSolution();
        virtual ~NoiseSolution();
    };
}

#endif //VSAR_SOLUTION_H_INCLUDED

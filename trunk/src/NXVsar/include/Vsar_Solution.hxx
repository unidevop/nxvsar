
#ifndef VSAR_SOLUTION_H_INCLUDED
#define VSAR_SOLUTION_H_INCLUDED

#include <Vsar_Component.hxx>

namespace Vsar
{
    class Solution : public BaseComponent
    {
    public:
        Solution();
        ~Solution();

        virtual void OnInit();

    protected:
        virtual bool HasGeometryDependency() const;

        virtual bool CanUpdateRailSlabFEModel() const;

        virtual bool CanUpdateBraseFEModel() const;

        virtual bool CanUpdateRailSlabConnection() const;

        virtual bool CanUpdateBraseConnection() const;

    private:
    };
}

#endif //VSAR_SOLUTION_H_INCLUDED

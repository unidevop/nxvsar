
#ifndef VSAR_SELMIINFINITEBASE_H_INCLUDED
#define VSAR_SELMIINFINITEBASE_H_INCLUDED

#include <Vsar_Component.hxx>

namespace Vsar
{
    class SelmiInfiniteBase : public BaseComponent
    {
    public:
        SelmiInfiniteBase();
        ~SelmiInfiniteBase();

        virtual void OnInit();

    protected:

        virtual bool CanUpdateRailSlabFEModel() const;

        virtual bool CanUpdateBraseFEModel() const;

        virtual bool CanUpdateRailSlabConnection() const;

        virtual bool CanUpdateBraseConnection() const;
    private:

        double GetHeight() const;

    private:
        double m_oldHeight;
    };
}

#endif //VSAR_SELMIINFINITEBASE_H_INCLUDED

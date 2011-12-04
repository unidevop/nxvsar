
#ifndef VSAR_BRIDGE_H_INCLUDED
#define VSAR_BRIDGE_H_INCLUDED

#include <Vsar_Component.hxx>

namespace Vsar
{
    class Bridge : public BaseComponent
    {
    public:
        Bridge();
        ~Bridge();

        virtual void OnInit();

    protected:

        virtual bool CanUpdateRailSlabFEModel() const;

        virtual bool CanUpdateBraseFEModel() const;

        virtual bool CanUpdateRailSlabConnection() const;

        virtual bool CanUpdateBraseConnection() const;
    private:

        double GetHeight() const;
        int GetSpanCount() const;

    private:
        double m_oldHeight;
        int  m_oldSpanCount;
    };
}

#endif //VSAR_BRIDGE_H_INCLUDED

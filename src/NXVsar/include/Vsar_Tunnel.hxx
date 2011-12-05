
#ifndef VSAR_TUNNEL_H_INCLUDED
#define VSAR_TUNNEL_H_INCLUDED

#include <Vsar_Component.hxx>

namespace Vsar
{
    class Tunnel : public BaseComponent
    {
    public:
        Tunnel();
        ~Tunnel();

        virtual void OnInit();

    protected:

        virtual bool CanUpdateRailSlabFEModel() const;

        virtual bool CanUpdateBraseFEModel() const;

        virtual bool CanUpdateRailSlabConnection() const;

        virtual bool CanUpdateBraseConnection() const;

    private:
    };
}

#endif //VSAR_BRIDGE_H_INCLUDED


#ifndef VSAR_SLAB_H_INCLUDED
#define VSAR_SLAB_H_INCLUDED

#include <Vsar_Component.hxx>

namespace Vsar
{
    class Slab : public BaseComponent
    {
    public:
        Slab();
        ~Slab();

        virtual void OnInit();

    protected:

        virtual bool CanUpdateRailSlabFEModel() const;

        virtual bool CanUpdateBraseFEModel() const;

        virtual bool CanUpdateRailSlabConnection() const;

        virtual bool CanUpdateBraseConnection() const;
    private:

        int GetSupportCount() const;

    private:
        int  m_oldSupportCount;
    };
}

#endif //VSAR_SLAB_H_INCLUDED

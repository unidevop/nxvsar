
#ifndef VSAR_TRAIN_H_INCLUDED
#define VSAR_TRAIN_H_INCLUDED

#include <Vsar_Component.hxx>

namespace NXOpen
{
    class Body;

    namespace CAE
    {
        class FemPart;
    }
}

namespace Vsar
{
    class Train : public BaseComponent
    {
    public:
        Train();
        ~Train();

        virtual void OnInit();

    protected:
        virtual void UpdateRailSlabModel();

        virtual void UpdateBraseModel();

        std::vector<NXOpen::Body*> GetGeoModelOccs(NXOpen::CAE::FemPart *pFemPart,
                                                const std::string &bodyPrtName, const std::string &bodyName);

        void UpdateRailSlabConnection(NXOpen::CAE::FemPart *pFemPart);

        void UpdateBaseSlabConnection(NXOpen::CAE::FemPart *pFemPart);
    private:

        int GetCarriageCount() const;

        void SetFeGeometryData( NXOpen::CAE::FemPart * pFemPart, const std::vector<NXOpen::Body*> &bodyOccs, bool syncLines );

    private:
        int  m_oldCarriageCount;
    };
}

#endif //VSAR_TRAIN_H_INCLUDED

// NXDotNetWrap.h

#ifndef VSDANE_NXVSDANEDOTNET_HXX_INCLUDED
#define VSDANE_NXVSDANEDOTNET_HXX_INCLUDED

#pragma once
#pragma managed

//#using <NXOpen.dll>

namespace Vsdane
{
    public ref class NXVsdane
    {
    public:
        void CreateSweptMeshDotNet(NXOpen::CAE::IFEModel ^pFeModel,
            System::String^ meshColName, System::String ^meshName,
            NXOpen::CAE::CAEFace ^pSrcFace, NXOpen::CAE::CAEFace ^pTargetFace, NXOpen::Expression ^pEleSize);
    };
}

#endif //VSDANE_NXVSDANEDOTNET_HXX_INCLUDED

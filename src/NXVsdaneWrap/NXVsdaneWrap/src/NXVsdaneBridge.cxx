// This is the main DLL file.

#include "NXVsdaneBridge.hxx"

#include "NXVsdaneDotNet.hxx"

using namespace System;
using namespace System::Runtime::InteropServices;
using namespace NXOpen;
using namespace NXOpen::Utilities;
using namespace NXOpen::CAE;

namespace Vsdane
{
    NXVsdaneBridge::NXVsdaneBridge()
    {
        m_impl = gcnew NXVsdane();
    }

    //void NXVsdaneBridge::CreateSweptMesh(void *pFeModel,
    //    const std::string &meshColName, const std::string &meshName,
    //    void *pSrcFace, void *pTargetFace, void *pEleSize)
    //{
        //NXVsdane::CreateSweptMeshDotNet(UnmanagedPtr2ManagedIFEModel(pFeModel),
        //    UnmanagedString2ManagedString(meshColName.c_str()),
        //    UnmanagedString2ManagedString(meshName.c_str()),
        //    UnmanagedPtr2ManagedCAEFace(pSrcFace),
        //    UnmanagedPtr2ManagedCAEFace(pTargetFace),
        //    UnmanagedPtr2ManagedExpression(pEleSize)
        //    );
    //}

    void NXVsdaneBridge::CreateSweptMesh(tag_t tFeModel,
        const std::string &meshColName, const std::string &meshName,
        tag_t tSrcFace, tag_t tTargetFace, tag_t tEleSize)
    {
        //IFEModel^ pFeModel = (IFEModel^)NXObjectManager::Get((Tag)tFeModel);
        m_impl->CreateSweptMeshDotNet((IFEModel^)NXObjectManager::Get((Tag)tFeModel),
            UnmanagedString2ManagedString(meshColName.c_str()),
            UnmanagedString2ManagedString(meshName.c_str()),
            (CAEFace^)NXObjectManager::Get((Tag)tSrcFace),
            (CAEFace^)NXObjectManager::Get((Tag)tTargetFace),
            (Expression^)NXObjectManager::Get((Tag)tEleSize)
            );
    }

    String^ NXVsdaneBridge::UnmanagedString2ManagedString(const char* pIn)
    {
        return Marshal::PtrToStringAnsi(static_cast<IntPtr>(const_cast<char*>(pIn)));
    }

    //generic<typename T>
    //T^ UnmanagedPtr2ManagedType(void *nativePtr)
    //{
    //    IntPtr ptrHandle = IntPtr(nativePtr);

    //    GCHandle handle = GCHandle::FromIntPtr(ptrHandle);

    //    T^ result=static_cast<T^>(handle.Target);

    //    handle.Free();

    //    return result;
    //}
    IFEModel^ NXVsdaneBridge::UnmanagedPtr2ManagedIFEModel(void *nativePtr)
    {
        IntPtr ptrHandle = IntPtr(nativePtr);

        GCHandle handle = GCHandle::FromIntPtr(ptrHandle);

        IFEModel^ result=static_cast<IFEModel^>(handle.Target);

        handle.Free();

        return result;
    }

    CAEFace^ NXVsdaneBridge::UnmanagedPtr2ManagedCAEFace(void *nativePtr)
    {
        IntPtr ptrHandle = IntPtr(nativePtr);

        GCHandle handle = GCHandle::FromIntPtr(ptrHandle);

        CAEFace^ result=static_cast<CAEFace^>(handle.Target);

        handle.Free();

        return result;
    }

    Expression^ NXVsdaneBridge::UnmanagedPtr2ManagedExpression(void *nativePtr)
    {
        IntPtr ptrHandle = IntPtr(nativePtr);

        GCHandle handle = GCHandle::FromIntPtr(ptrHandle);

        Expression^ result=static_cast<Expression^>(handle.Target);

        handle.Free();

        return result;
    }
}
//==============================================================================
//  WARNING!!  This file is overwritten by the Block Styler while generating
//  the automation code. Any modifications to this file will be lost after
//  generating the code again.
//
//       Filename:  E:\Project\CAE\Response\VSAR-DEV\NXProject\AppRoot\application\Tunnel.hpp
//
//        This file was generated by the NX Block Styler
//        Created by: Joseph
//              Version: NX 7.5
//              Date: 05-08-2011  (Format: mm-dd-yyyy)
//              Time: 14:04
//
//==============================================================================

#ifndef VSARUI_TUNNELSETTINGS_H_INCLUDED
#define VSARUI_TUNNELSETTINGS_H_INCLUDED

//------------------------------------------------------------------------------
//These includes are needed for the following template code
//------------------------------------------------------------------------------
#include <VsarUI_BaseCompDialog.hxx>

//------------------------------------------------------------------------------
// Forward declaration for Class
//------------------------------------------------------------------------------

namespace VsarUI
{
    class TunnelSettings : public BaseCompDialog
    {
        // class members
    public:
        TunnelSettings();
        ~TunnelSettings();

        static void ShowDialog();

        //----------------------- BlockStyler Callback Prototypes ---------------------
        // The following member function prototypes define the callbacks 
        // specified in your BlockStyler dialog.  The empty implementaiton
        // of these prototypes is provided in the Tunnel.cpp file. 
        // You are REQUIRED to write the implementation for these funtions.
        //------------------------------------------------------------------------------
        virtual void InitializeCb();
        virtual void DialogShownCb();
        //int ApplyCb();
        virtual int UpdateCb(NXOpen::BlockStyler::UIBlock* block);

    private:

        //NXOpen::BlockStyler::UIBlock* grpGeometry;// Block type: Group
        NXOpen::BlockStyler::UIBlock* m_image;// Block type: Label
        NXOpen::BlockStyler::UIBlock* m_diameter;// Block type: Expression
        NXOpen::BlockStyler::UIBlock* m_width;// Block type: Expression
        NXOpen::BlockStyler::UIBlock* m_h1;// Block type: Expression
        NXOpen::BlockStyler::UIBlock* m_h2;// Block type: Expression
        NXOpen::BlockStyler::UIBlock* m_h3;// Block type: Expression
    };
}
#endif //VSARUI_TUNNELSETTINGS_H_INCLUDED

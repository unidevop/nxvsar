//==============================================================================
//  WARNING!!  This file is overwritten by the Block UI Styler while generating
//  the automation code. Any modifications to this file will be lost after
//  generating the code again.
//
//       Filename:  E:\Project\CAE\Response\VSAR-DEV\NXProject\AppRoot\application\Tunnel.cpp
//
//        This file was generated by the NX Block UI Styler
//        Created by: Joseph
//              Version: NX 7.5
//              Date: 05-08-2011  (Format: mm-dd-yyyy)
//              Time: 14:04 (Format: hh-mm)
//
//==============================================================================

//==============================================================================
//  Purpose:  This TEMPLATE file contains C++ source to guide you in the
//  construction of your Block application dialog. The generation of your
//  dialog file (.dlx extension) is the first step towards dialog construction
//  within NX.  You must now create a NX Open application that
//  utilizes this file (.dlx).
//
//  The information in this file provides you with the following:
//
//  1.  Help on how to load and display your Block UI Styler dialog in NX
//      using APIs provided in NXOpen.BlockStyler namespace
//  2.  The empty callback methods (stubs) associated with your dialog items
//      have also been placed in this file. These empty methods have been
//      created simply to start you along with your coding requirements.
//      The method name, argument list and possible return values have already
//      been provided for you.
//==============================================================================

//------------------------------------------------------------------------------
//These includes are needed for the following template code
//------------------------------------------------------------------------------
#include <uf_defs.h>
#include <VsarUI_TunnelSettings.hxx>

#include <NXOpen/UI.hxx>
#include <NXOpen/NXMessageBox.hxx>

#include <Vsar_Component.hxx>
#include <Vsar_Init_Utils.hxx>

#include <Vsar_Names.hxx>
#include <Vsar_Tunnel.hxx>

using namespace NXOpen;
using namespace NXOpen::BlockStyler;
using namespace Vsar;

namespace VsarUI
{
    //------------------------------------------------------------------------------
    // Constructor for NX Styler class
    //------------------------------------------------------------------------------
    TunnelSettings::TunnelSettings() : BaseCompDialog("Tunnel.dlx",
                                      new Tunnel())
    {
    }

    //------------------------------------------------------------------------------
    // Destructor for NX Styler class
    //------------------------------------------------------------------------------
    TunnelSettings::~TunnelSettings()
    {
    }

    void TunnelSettings::ShowDialog()
    {
        boost::scoped_ptr<TunnelSettings> pRailSettingDlg(new TunnelSettings());

        try
        {
            // The following method shows the dialog immediately
            pRailSettingDlg->Show(BlockDialog::DialogModeEdit);
        }
        catch(std::exception& ex)
        {
            //---- Enter your exception handling code here -----
            s_theUI->NXMessageBox()->Show("Block Styler", NXMessageBox::DialogTypeError, ex.what());
        }
    }

    //------------------------------------------------------------------------------
    //---------------------Block UI Styler Callback Functions--------------------------
    //------------------------------------------------------------------------------

    //------------------------------------------------------------------------------
    //Callback Name: initialize_cb
    //------------------------------------------------------------------------------
    void TunnelSettings::InitializeCb()
    {
        BaseCompDialog::InitializeCb();

        try
        {
            CompositeBlock *pTopBlock = m_theDialog->TopBlock();

            //grpGeometry = pTopBlock->FindBlock("grpGeometry");
            m_image     = pTopBlock->FindBlock("image");
            m_diameter  = pTopBlock->FindBlock(DIAMETER_ID_NAME);
            m_width     = pTopBlock->FindBlock(WIDTH_ID_NAME);
            m_h1        = pTopBlock->FindBlock(TUNNEL_H1_ID_NAME);
            m_h2        = pTopBlock->FindBlock(TUNNEL_H2_ID_NAME);
            m_h3        = pTopBlock->FindBlock(TUNNEL_H3_ID_NAME);
        }
        catch(std::exception& ex)
        {
            //---- Enter your exception handling code here -----
            s_theUI->NXMessageBox()->Show("Block Styler", NXMessageBox::DialogTypeError, ex.what());
        }
    }

    //------------------------------------------------------------------------------
    //Callback Name: dialogShown_cb
    //This callback is executed just before the dialog launch. Thus any value set 
    //here will take precedence and dialog will be launched showing that value. 
    //------------------------------------------------------------------------------
    void TunnelSettings::DialogShownCb()
    {
        try
        {
            //---- Enter your callback code here -----
        }
        catch(std::exception& ex)
        {
            //---- Enter your exception handling code here -----
            s_theUI->NXMessageBox()->Show("Block Styler", NXMessageBox::DialogTypeError, ex.what());
        }
    }

    //------------------------------------------------------------------------------
    //Callback Name: apply_cb
    //------------------------------------------------------------------------------
    //int Tunnel::ApplyCb()
    //{
    //    int errorCode = 0;
    //    try
    //    {
    //        //---- Enter your callback code here -----
    //    }
    //    catch(std::exception& ex)
    //    {
    //        //---- Enter your exception handling code here -----
    //        errorCode = 1;
    //        theUI->NXMessageBox()->Show("Block Styler", NXMessageBox::DialogTypeError, ex.what());
    //    }
    //    return errorCode;
    //}

    //------------------------------------------------------------------------------
    //Callback Name: update_cb
    //------------------------------------------------------------------------------
    int TunnelSettings::UpdateCb(UIBlock* block)
    {
        try
        {
            if(block == m_image)
            {
                //---------Enter your code here-----------
            }
            else if(block == m_diameter)
            {
                //---------Enter your code here-----------
            }
            else if(block == m_width)
            {
                //---------Enter your code here-----------
            }
            else if(block == m_h1)
            {
                //---------Enter your code here-----------
            }
            else if(block == m_h2)
            {
                //---------Enter your code here-----------
            }
            else if(block == m_h3)
            {
                //---------Enter your code here-----------
            }
        }
        catch(std::exception& ex)
        {
            //---- Enter your exception handling code here -----
            s_theUI->NXMessageBox()->Show("Block Styler", NXMessageBox::DialogTypeError, ex.what());
        }
        return 0;
    }
}

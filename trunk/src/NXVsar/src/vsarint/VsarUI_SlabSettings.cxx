//==============================================================================
//  WARNING!!  This file is overwritten by the Block UI Styler while generating
//  the automation code. Any modifications to this file will be lost after
//  generating the code again.
//
//       Filename:  E:\Project\CAE\Response\VSAR-DEV\NXProject\AppRoot\application\SlabSettings.cpp
//
//        This file was generated by the NX Block UI Styler
//        Created by: Joseph
//              Version: NX 7.5
//              Date: 05-08-2011  (Format: mm-dd-yyyy)
//              Time: 14:00 (Format: hh-mm)
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
#include <VsarUI_SlabSettings.hxx>

#include <NXOpen/UI.hxx>
#include <NXOpen/NXMessageBox.hxx>

#include <Vsar_Component.hxx>
#include <Vsar_Init_Utils.hxx>


using namespace NXOpen;
using namespace NXOpen::BlockStyler;
using namespace Vsar;

namespace VsarUI
{
    static CompAttrInfo attrExpInfo[] =
    {
        {"supportCnt",                  "slab",       "Support_Count"},

        {"fastenerStiffness",           "RailSlab_f", "Slab_Fastener_Stiffness"},
        {"fastenerDamping",             "RailSlab_f", "Slab_Fastener_Damping"},
        {"massRatio",                   "RailSlab_f", "Slab_Mass_Ratio"},
        {"stiffnessRatio",              "RailSlab_f", "Slab_Stiffness_Ratio"},
        {"dampingRatio",                "RailSlab_f", "Slab_Damping_Ratio"},
        {"elasticModulus",              "RailSlab_f", "Slab_Elastic_Modulus"},
        {"crossSectMomentInertia",      "RailSlab_f", "Slab_CrossSect_MomentInertia"},
    };

    //------------------------------------------------------------------------------
    // Constructor for NX Styler class
    //------------------------------------------------------------------------------
    SlabSettings::SlabSettings() : BaseCompDialog("SlabSettings.dlx",
                                                  new BaseComponent(attrExpInfo, N_ELEMENTS(attrExpInfo)))
    {
    }

    //------------------------------------------------------------------------------
    // Destructor for NX Styler class
    //------------------------------------------------------------------------------
    SlabSettings::~SlabSettings()
    {
    }

    void SlabSettings::ShowDialog()
    {
        boost::scoped_ptr<SlabSettings> pRailSettingDlg(new SlabSettings());

        try
        {
            // The following method shows the dialog immediately
            pRailSettingDlg->Show(BlockDialog::DialogModeEdit);
        }
        catch(std::exception& ex)
        {
            //---- Enter your exception handling code here -----
            theUI->NXMessageBox()->Show("Block Styler", NXMessageBox::DialogTypeError, ex.what());
        }
    }

    //------------------------------------------------------------------------------
    //---------------------Block UI Styler Callback Functions--------------------------
    //------------------------------------------------------------------------------

    //------------------------------------------------------------------------------
    //Callback Name: initialize_cb
    //------------------------------------------------------------------------------
    void SlabSettings::InitializeCb()
    {
        BaseCompDialog::InitializeCb();

        try
        {
            CompositeBlock *pTopBlock = m_theDialog->TopBlock();

            //grpGeometry = pTopBlock->FindBlock("grpGeometry");
            m_length = pTopBlock->FindBlock("length");
            //m_eleSize = pTopBlock->FindBlock("eleSize");
            //counts = pTopBlock->FindBlock("counts");
            m_supportCnt = pTopBlock->FindBlock("supportCnt");

            //grpMaterial = pTopBlock->FindBlock("grpMaterial");
            m_fastenerStiffness = pTopBlock->FindBlock("fastenerStiffness");
            m_fastenerDamping = pTopBlock->FindBlock("fastenerDamping");
            m_massRatio = pTopBlock->FindBlock("massRatio");
            m_stiffnessRatio = pTopBlock->FindBlock("stiffnessRatio");
            m_dampingRatio = pTopBlock->FindBlock("dampingRatio");
            m_elasticModulus = pTopBlock->FindBlock("elasticModulus");
            m_crossSectMomentInertia = pTopBlock->FindBlock("crossSectMomentInertia");
        }
        catch(std::exception& ex)
        {
            //---- Enter your exception handling code here -----
            theUI->NXMessageBox()->Show("Block Styler", NXMessageBox::DialogTypeError, ex.what());
        }
    }

    //------------------------------------------------------------------------------
    //Callback Name: dialogShown_cb
    //This callback is executed just before the dialog launch. Thus any value set 
    //here will take precedence and dialog will be launched showing that value. 
    //------------------------------------------------------------------------------
    void SlabSettings::DialogShownCb()
    {
        try
        {
            //---- Enter your callback code here -----
        }
        catch(std::exception& ex)
        {
            //---- Enter your exception handling code here -----
            theUI->NXMessageBox()->Show("Block Styler", NXMessageBox::DialogTypeError, ex.what());
        }
    }

    //------------------------------------------------------------------------------
    //Callback Name: apply_cb
    //------------------------------------------------------------------------------
    //int SlabSettings::ApplyCb()
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
    int SlabSettings::UpdateCb(UIBlock* block)
    {
        try
        {
            if(block == m_length)
            {
                //---------Enter your code here-----------
            }
            //else if(block == m_eleSize)
            //{
            //    //---------Enter your code here-----------
            //}
            //else if(block == counts)
            //{
            ////---------Enter your code here-----------
            //}
            else if(block == m_supportCnt)
            {
                //---------Enter your code here-----------
            }
            else if(block == m_fastenerStiffness)
            {
                //---------Enter your code here-----------
            }
            else if(block == m_fastenerDamping)
            {
                //---------Enter your code here-----------
            }
            else if(block == m_massRatio)
            {
                //---------Enter your code here-----------
            }
            else if(block == m_stiffnessRatio)
            {
                //---------Enter your code here-----------
            }
            else if(block == m_dampingRatio)
            {
                //---------Enter your code here-----------
            }
            else if(block == m_elasticModulus)
            {
                //---------Enter your code here-----------
            }
            else if(block == m_crossSectMomentInertia)
            {
                //---------Enter your code here-----------
            }
        }
        catch(std::exception& ex)
        {
            //---- Enter your exception handling code here -----
            theUI->NXMessageBox()->Show("Block Styler", NXMessageBox::DialogTypeError, ex.what());
        }
        return 0;
    }
}

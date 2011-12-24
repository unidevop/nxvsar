//==============================================================================
//  WARNING!!  This file is overwritten by the Block UI Styler while generating
//  the automation code. Any modifications to this file will be lost after
//  generating the code again.
//
//       Filename:  E:\Project\Response\VSAR\NXProject\AppRoot\application\Vsar_NewProject.cpp
//
//        This file was generated by the NX Block UI Styler
//        Created by: wujif
//              Version: NX 7.5
//              Date: 02-19-2011  (Format: mm-dd-yyyy)
//              Time: 17:32 (Format: hh-mm)
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

#include <uf.h>
#include <NXOpen/Session.hxx>
#include <NXOpen/UI.hxx>
#include <NXOpen/NXMessageBox.hxx>
#include <NXOpen/NXException.hxx>

#include <algorithm>
#include <boost/function.hpp>
#include <boost/bind.hpp>
//#include <boost/lambda/lambda.hpp>

#include <VsarUI_NewProject.hxx>
#include <VsarUI_TrainSettings.hxx>
#include <VsarUI_RailSettings.hxx>
#include <VsarUI_SlabSettings.hxx>
#include <VsarUI_BridgeSettings.hxx>
#include <VsarUI_SelmiInfiniteSettings.hxx>
#include <VsarUI_TunnelSettings.hxx>
#include <VsarUI_SolveResponse.hxx>
#include <VsarUI_SolveNoise.hxx>

#include <Vsar_Init_Utils.hxx>
#include <Vsar_Init_Names.hxx>

using namespace NXOpen;
using namespace NXOpen::BlockStyler;
using namespace Vsar;
using namespace VsarUI;


//------------------------------------------------------------------------------
// Declaration of global variables
//------------------------------------------------------------------------------
//namespace VsarUI
//{
//    NewProject *theVsar_NewProject;
//}

//------------------------------- DIALOG LAUNCHING ---------------------------------
//
//    Before invoking this application one needs to open any part/empty part in NX
//    because of the behavior of the blocks.
//
//    Make sure the dlx file is in one of the following locations:
//        1.) From where NX session is launched
//        2.) $UGII_USER_DIR/application
//        3.) For released applications, using UGII_CUSTOM_DIRECTORY_FILE is highly
//            recommended. This variable is set to a full directory path to a file 
//            containing a list of root directories for all custom applications.
//            e.g., UGII_CUSTOM_DIRECTORY_FILE=$UGII_ROOT_DIR\menus\custom_dirs.dat
//
//    You can create the dialog using one of the following way:
//
//    1. USER EXIT
//
//        1) Remove the following conditional definitions:
//                a) #if USER_EXIT_OR_MENU
//                    #endif//USER_EXIT_OR_MENU
//
//                b) #if USER_EXIT
//                    #endif//USER_EXIT
//        2) Create the Shared Library -- Refer "Block UI Styler programmer's guide"
//        3) Invoke the Shared Library through File->Execute->NX Open menu.
//
//    2. THROUGH CALLBACK OF ANOTHER DIALOG
//
//        1) Remove the following conditional definition:
//             #if CALLBACK
//             #endif//CALLBACK
//        2) Call the following line of code from where ever you want to lauch this dialog.
//             Vsar_NewProject->Show_Vsar_NewProject();
//        3) Integrate this file with your main application file.
//
//    3. MENU BAR
//    
//        1) Remove the following conditional definition:
//                a) #if USER_EXIT_OR_MENU
//                   #endif//USER_EXIT_OR_MENU
//        2) Add the following lines to your MenuScript file in order to
//           associate a menu bar button with your dialog.  In this
//           example, a cascade menu will be created and will be
//           located just before the Help button on the main menubar.
//           The button, SAMPLEVB_BTN is set up to launch your dialog and
//           will be positioned as the first button on your pulldown menu.
//           If you wish to add the button to an existing cascade, simply
//           add the 3 lines between MENU LAUNCH_CASCADE and END_OF_MENU
//           to your menuscript file.
//           The MenuScript file requires an extension of ".men".
//           Move the contents between the dashed lines to your Menuscript file.
//        
//           !-----------------------------------------------------------------------------
//           VERSION 120
//        
//           EDIT UG_GATEWAY_MAIN_MENUBAR
//        
//           BEFORE UG_HELP
//           CASCADE_BUTTON BLOCKSTYLER_DLX_CASCADE_BTN
//           LABEL Dialog Launcher
//           END_OF_BEFORE
//        
//           MENU BLOCKSTYLER_DLX_CASCADE_BTN
//           BUTTON SAMPLEVB_BTN
//           LABEL Display SampleVB dialog
//           ACTIONS <path of Shared library> !For example: D:\temp\SampleVB.dll
//           END_OF_MENU
//           !-----------------------------------------------------------------------------
//        
//        3) Make sure the .men file is in one of the following locations:
//        
//           - $UGII_USER_DIR/startup   
//           - For released applications, using UGII_CUSTOM_DIRECTORY_FILE is highly
//             recommended. This variable is set to a full directory path to a file 
//             containing a list of root directories for all custom applications.
//             e.g., UGII_CUSTOM_DIRECTORY_FILE=$UGII_ROOT_DIR\menus\custom_dirs.dat
//    
//------------------------------------------------------------------------------
//#if USER_EXIT_OR_MENU

struct MenuItemCallbackInfo
{
    std::string                  m_menuItemName;
    boost::function<void ()>     m_callback;
};

    extern "C" DllExport void  ufusr(char *param, int *retcod, int param_len)
    {
        if (!UF_is_initialized())
            UF_initialize();

        MenuItemCallbackInfo  menuItemCbs[] = {
            {MENU_ITEM_NAME_NEW_PROJECT,        NewProject::ShowDialog},
            {MENU_ITEM_NAME_SET_TRAIN,          TrainSettings::ShowDialog},
            {MENU_ITEM_NAME_SET_RAIL,           RailSettings::ShowDialog},
            {MENU_ITEM_NAME_SET_SLAB,           SlabSettings::ShowDialog},
            {MENU_ITEM_NAME_SET_BRIDGE,         BridgeSettings::ShowDialog},
            {MENU_ITEM_NAME_SET_BASE,           SelmiInfiniteSettings::ShowDialog},
            {MENU_ITEM_NAME_SET_TUNNEL,         TunnelSettings::ShowDialog},
            {MENU_ITEM_NAME_SOLVE_RESPONSE,     SolveResponse::ShowDialog},
            {MENU_ITEM_NAME_SOLVE_NOISE,        SolveNoise::ShowDialog},
            //{MENU_ITEM_NAME_LOAD_RESULT,        TrainSettings::ShowDialog}
        };

        std::string  menuItemName(param ? param : "");

        MenuItemCallbackInfo *pMenuItem = std::find_if(menuItemCbs, menuItemCbs + N_ELEMENTS(menuItemCbs),
            boost::bind(&MenuItemCallbackInfo::m_menuItemName, _1) == menuItemName);

        if (pMenuItem != menuItemCbs + N_ELEMENTS(menuItemCbs))
            pMenuItem->m_callback();

        UF_terminate();
    }
//#endif//USER_EXIT_OR_MENU
//#if USER_EXIT
    
    //------------------------------------------------------------------------------
    // This method specifies how a shared image is unloaded from memory
    // within NX. This method gives you the capability to unload an
    // internal NX Open application or user  exit from NX. Specify any
    // one of the three constants as a return value to determine the type
    // of unload to perform:
    //
    //
    //    Immediately : unload the library as soon as the automation program has completed
    //    Explicitly  : unload the library from the "Unload Shared Image" dialog
    //    AtTermination : unload the library when the NX session terminates
    //
    //
    // NOTE:  A program which associates NX Open applications with the menubar
    // MUST NOT use this option since it will UNLOAD your NX Open application image
    // from the menubar.
    //------------------------------------------------------------------------------
    extern "C" DllExport int ufusr_ask_unload()
    {
#if defined (DEBUG) || (_DEBUG)
        return (int)Session::LibraryUnloadOptionImmediately;
#else
        return (int)Session::LibraryUnloadOptionExplicitly;
#endif
        //return (int)Session::LibraryUnloadOptionAtTermination;
    }
    
    //------------------------------------------------------------------------------
    // Following method cleanup any housekeeping chores that may be needed.
    // This method is automatically called by NX.
    //------------------------------------------------------------------------------
    extern "C" DllExport void ufusr_cleanup(void)
    {
        try
        {
            //---- Enter your callback code here -----
        }
        catch(std::exception& ex)
        {
            //---- Enter your exception handling code here -----
            NewProject::theUI->NXMessageBox()->Show("Block Styler", NXMessageBox::DialogTypeError, ex.what());
        }
    }
//#endif//USER_EXIT

/*****************************************************************************
**
** VsarInit.cpp
**
** Description:
**     Contains Unigraphics entry points for the application.
**
*****************************************************************************/

/* Include files */
#include <uf_defs.h>

#include <sstream>
#include <iostream>

#include <NXOpen/Session.hxx>
#include <NXOpen/NXException.hxx>
#include <NXOpen/Callback.hxx>
#include <NXOpen/PartCollection.hxx>
#include <NXOpen/UI.hxx>
#include <NXOpen/MenuBar_MenuBarManager.hxx>
#include <NXOpen/ListingWindow.hxx>

#include <Vsar_Project.hxx>

using namespace NXOpen;
using namespace Vsar;

static void processException( const NXException &exc );

namespace Vsar
{
    MenuBar::MenuBarManager::CallbackStatus NewProjectCB( MenuBar::MenuButtonEvent* buttonEvent )
    {
        ListingWindow *pLstWnd = Session::GetSession()->ListingWindow();

        /* Open the UgInfoWindow */
        if (!pLstWnd->IsOpen())
            pLstWnd->Open();

        /* Write the message to the UgInfoWindow.  The str method */
        /* freezes the buffer, so it must be unfrozen afterwards. */
        pLstWnd->WriteLine( "New Project Dialog" );

        return MenuBar::MenuBarManager::CallbackStatusContinue;
    }
}

/*****************************************************************************
**  Activation Methods
*****************************************************************************/
/*  Unigraphics Startup
**      This entry point activates the application at Unigraphics startup */
extern "C" DllExport void ufsta( char *param, int *returnCode, int rlen )
{
    try
    {
        // Initialize the NX Open C++ API environment
        Session *theSession = Session::GetSession();

        //MenuBar::MenuBarManager *pMBMgr = UI::GetUI()->MenuBarManager();

        //pMBMgr->AddMenuAction("Vsar_NewProject", make_callback(&Vsar::NewProjectCB));

        theSession->Parts()->AddPartOpenedHandler(make_callback(&Project::Open));

        theSession->Parts()->AddPartClosedHandler(make_callback(&Project::Close));
    }

    /* Handle errors */
    catch ( const NXException &exc )
    {
        processException( exc );
    }
}

/*  Open Part
**      This user exit is invoked after the following menu item is activated:
**      "File->Open" */
extern "C" DllExport void ufget( char *param, int *returnCode, int rlen )
{
    try
    {
        // Initialize the NX Open C++ API environment
        Session *theSession = Session::GetSession();

        /* TODO: Add your application code here */
    }

    /* Handle errors */
    catch ( const NXException &exc )
    {
        processException( exc );
    }
}

/*****************************************************************************
**  Utilities
*****************************************************************************/

/* Unload Handler
**     This function specifies when to unload your application from Unigraphics.
**     If your application registers a callback (from a MenuScript item or a
**     User Defined Object for example), this function MUST return
**     "UF_UNLOAD_UG_TERMINATE". */
extern "C" DllExport int ufusr_ask_unload()
{
    //return (int)Session::LibraryUnloadOptionExplicitly;
    //return (int)Session::LibraryUnloadOptionImmediately;
    return (int)Session::LibraryUnloadOptionAtTermination;
}

/* processException
       Prints error messages to standard error and a Unigraphics
       information window. */
static void processException( const NXException &exc )
{
    /* Construct a buffer to hold the text. */
    std::ostringstream error_message;

    /* Initialize the buffer with the required text. */
    error_message << std::endl
                  << "Error:" << std::endl
                  << exc.Message()
                  << std::endl << std::endl << std::ends;

    ListingWindow *pLstWnd = Session::GetSession()->ListingWindow();

    /* Open the UgInfoWindow */
    if (!pLstWnd->IsOpen())
        pLstWnd->Open();

    /* Write the message to the UgInfoWindow.  The str method */
    /* freezes the buffer, so it must be unfrozen afterwards. */
    pLstWnd->WriteLine( error_message.str() );

    /* Write the message to standard error */
    std::cerr << error_message.str();
    //error_message.rdbuf()->freeze( 0 );
}

//======= Maestra Fenix, 2017 ==================================================//
//
// Purpose: Map load background panel
//
//==============================================================================//

#include "cbase.h"
#include "mapload_background.h"
#include <vgui/IScheme.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CMapLoadBG::CMapLoadBG( char const *panelName ) : EditablePanel( NULL, panelName )
{
	VPANEL toolParent = enginevgui->GetPanel( PANEL_GAMEUIDLL );
	SetParent( toolParent );

	// Fenix: We load a RES file rather than create the element here for taking advantage of the "F" parameter for wide and tall
	// Is the sole thing that makes fill the background to the entire screen regardless of the texture size
	// Congratulations to Valve for once again give options to only one side and not both
	SetProportional( true );
	LoadControlSettings( "resource/loadingdialogbackground.res" );

	m_pBackground = FindControl<ImagePanel>( "LoadingImage", true );
	m_pMapIcon = FindControl<ImagePanel>( "MapIcon", true );
	m_pServerName = FindControl<Label>( "ServerName", true );
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CMapLoadBG::~CMapLoadBG()
{
	// None
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CMapLoadBG::ApplySchemeSettings( IScheme *pScheme )
{
    BaseClass::ApplySchemeSettings( pScheme );

    int iWide, iTall;
    surface()->GetScreenSize(iWide, iTall);
    SetSize(iWide, iTall);

    int iconWidth  = iWide / 10;   // 10% of screen width
    int iconHeight = iTall / 5;    // 20% of screen height
    int iconX = iWide * 0.01;      // 1% from left
    int iconY = iTall * 0.02;      // 2% from top

    m_pMapIcon->SetSize(iconWidth, iconHeight);
    m_pMapIcon->SetPos(iconX, iconY);

    // Server name font
    vgui::HFont hFont = pScheme->GetFont("Default", true);
    hFont = vgui::surface()->CreateFont();
    vgui::surface()->SetFontGlyphSet(hFont, "Arial", 24, 800, 0, 0, NULL);
    m_pServerName->SetFont(hFont);
	m_pServerName->SizeToContents();

    // Position server name **next to the map icon**
    int labelX = iconX + iconWidth + (iWide * 0.01);
    int labelY = iconY + 5;
    m_pServerName->SetPos(labelX, labelY);

    m_pServerName->SetWide(iWide - labelX - (iWide * 0.01));
}

//-----------------------------------------------------------------------------
// Purpose: Sets a new server name on demand
//-----------------------------------------------------------------------------
void CMapLoadBG::setServerName( const char *serverName )
{
	m_pServerName->SetText( serverName );
}
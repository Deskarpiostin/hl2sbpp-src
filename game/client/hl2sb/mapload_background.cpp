//======= Maestra Fenix, 2017 ==================================================//
//
// Purpose: Map load background panel
//
//==============================================================================//

#include "cbase.h"
#include "mapload_background.h"
#include <vgui/IScheme.h>
#include <string>
#include "filesystem.h"
#include <vector>

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

	m_pBackground = FindControl<ImagePanel> ( "LoadingImage", true );
	m_pServerName = FindControl<Label>		( "ServerName", true );
	m_pGradient = 	FindControl<ImagePanel> ( "Gradient", true );
	m_pMapName = 	FindControl<Label>		( "MapName", true );
	m_pGameMode = 	FindControl<Label>		( "GameMode", true );
	m_pMapIcon = 	new PngImagePanel( this, "PNGPanel", "maps/thumb/placeholder.png" );
	m_pGameLogo = 	FindControl<ImagePanel> ( "Logo", true );

	SetZPos(99999);
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CMapLoadBG::~CMapLoadBG()
{
	// None
}

//-----------------------------------------------------------------------------
// Purpose: not much
//-----------------------------------------------------------------------------
void CMapLoadBG::OnThink()
{
    BaseClass::OnThink();

    if (m_pGameLogo)
    {
        float flTime = gpGlobals->curtime;
        float flWave = sin(flTime * 2.0f);
        float flScale = (flWave + 1.0f) * 0.5f;

        float flMinScale = 1.0f;
        float flMaxScale = 1.2f;
        float flCurrentScale = flMinScale + (flMaxScale - flMinScale) * flScale;

        int newWide = (int)(m_iLogoBaseWide * flCurrentScale);
        int newTall = (int)(m_iLogoBaseTall * flCurrentScale);

        int centerX = m_iLogoBaseX + m_iLogoBaseWide / 2;
        int centerY = m_iLogoBaseY + m_iLogoBaseTall / 2;

        m_pGameLogo->SetSize(newWide, newTall);
        m_pGameLogo->SetPos(centerX - newWide / 2, centerY - newTall / 2);
    }
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

    int iconWidth  = iWide / 12.75;// 12.75% of screen width
    int iconX = iWide * 0.01;      // 1% from left
    int iconY = iTall * 0.02;      // 2% from top

	m_pGradient->SetBorder( nullptr );
	m_pBackground->SetBorder( nullptr );
	m_pGameLogo->SetBorder( nullptr );

	m_pGradient->SetAlpha(128);

    vgui::HFont hFont = pScheme->GetFont("Default", true);
    hFont = vgui::surface()->CreateFont();
    vgui::surface()->SetFontGlyphSet(hFont, "Arial", 24, 800, 0, 0, NULL);

    m_pServerName->SetFont(hFont);
	m_pServerName->SizeToContents();
	m_pMapName->SetFont(hFont);
	m_pMapName->SizeToContents();
	m_pGameMode->SetFont(hFont);
	m_pGameMode->SizeToContents();

	m_pMapIcon->SetBounds(
		10, 10,
		110, 110
	);

    int labelX = iconX + iconWidth + (iWide * 0.01);
    int labelY = iconY + 5;
    m_pServerName->SetPos(labelX, labelY);
    m_pServerName->SetWide(iWide - labelX - (iWide * 0.01));
	m_pServerName->SetFgColor(COLOR_BLACK);

	m_pMapName->SetPos(labelX, labelY + m_pServerName->GetTall() + 5);
    m_pMapName->SetWide(iWide - labelX - (iWide * 0.01));
	m_pMapName->SetFgColor(COLOR_BLACK);

	m_pGameMode->SetPos(labelX, m_pMapName->GetYPos() + m_pMapName->GetTall() + 5);
    m_pGameMode->SetWide(iWide - labelX - (iWide * 0.01));
	m_pGameMode->SetFgColor(COLOR_BLACK);

	if (m_pGameLogo)
    {
        m_iLogoBaseWide = m_pGameLogo->GetWide();
        m_iLogoBaseTall = m_pGameLogo->GetTall();
        m_pGameLogo->GetPos(m_iLogoBaseX, m_iLogoBaseY);
    }
}

std::string CapitalizeFirstLetter(const char* str)
{
    if (!str || str[0] == '\0')
        return "";

    std::string result(str);
    if (result[0] >= 'a' && result[0] <= 'z')
    {
        result[0] = result[0] - 'a' + 'A';
    }
    return result;
}

//-----------------------------------------------------------------------------
// Purpose: Find the disconnect command, and rename it...
//-----------------------------------------------------------------------------
CON_COMMAND(__map, "Start playing on specified map.")
{
	extern CMapLoadBG *pPanelBg;

	if ( args.ArgC() < 2 )
	{
		Msg( "Usage: map <mapname>\n" );
		return;
	}

	const char *mapName = args[1];

	std::string capServer = CapitalizeFirstLetter(cvar->FindVar("hostname")->GetString());
	pPanelBg->setServerName(capServer.c_str());

	std::string capGamemode = CapitalizeFirstLetter(cvar->FindVar("gamemode")->GetString());
	pPanelBg->setGameModeName(capGamemode.c_str());

	std::string capMap = CapitalizeFirstLetter(mapName);
	pPanelBg->setMapName(capMap.c_str());

	char cmd[MAX_PATH];
	Q_snprintf( cmd, sizeof(cmd), "progress_enable; __real_map %s", mapName );
	engine->ClientCmd_Unrestricted( cmd );
}

void SwapMapCommand()
{
	ConCommand* _realMapCommand = dynamic_cast<ConCommand*>(g_pCVar->FindCommand("map"));
	ConCommand* _MapCommand = dynamic_cast<ConCommand*>(g_pCVar->FindCommand("__map"));

	if (!_realMapCommand)
		return;

	if (!_MapCommand)
		return;

	_realMapCommand->Shutdown();
	_realMapCommand->CreateBase( "__real_map", "" );
	_realMapCommand->Init();

	_MapCommand->Shutdown();
    _MapCommand->CreateBase("map", "Start playing on specified map.");
	_MapCommand->Init();
}
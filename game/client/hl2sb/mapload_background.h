//======= Maestra Fenix, 2017 ==================================================//
//
// Purpose: Map load background panel
//
//==============================================================================//

#ifndef MAPLOAD_BACKGROUND_H
#define MAPLOAD_BACKGROUND_H

#ifdef _WIN32
#pragma once
#endif

#include "vgui/ISurface.h"
#include <vgui_controls/EditablePanel.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/Label.h>
#include "ienginevgui.h"
#include "filesystem.h"

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CMapLoadBG : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CMapLoadBG, vgui::EditablePanel);

public:
	CMapLoadBG( char const *panelName );
	~CMapLoadBG();

	void setServerName( const char *serverName )
	{
		m_pServerName->SetText( serverName );
	}

	void setGameModeName( const char *gameModeName )
	{
		m_pGameMode->SetText( gameModeName );
	}

	void setMapName( const char *mapName )
	{
		m_pMapName->SetText( mapName);

		char imageName[ MAX_PATH ];
		Q_snprintf( imageName, sizeof( imageName ), "thumb/%s", mapName );

		char imageNameB[ MAX_PATH ];
		Q_snprintf( imageNameB, sizeof( imageNameB ), "materials/vgui/thumb/%s.vmt", mapName );

		if ( g_pFullFileSystem->FileExists( imageNameB, "MOD" ) )
			m_pMapIcon->SetImage( imageName );
		else
			m_pMapIcon->SetImage( "img/noicon" );
	}

	virtual void OnThink();

protected:
	void ApplySchemeSettings( vgui::IScheme *pScheme );

private:
	vgui::ImagePanel *m_pBackground;
	vgui::ImagePanel *m_pMapIcon;
	vgui::ImagePanel *m_pGradient;
	vgui::ImagePanel *m_pGameLogo;
    vgui::Label		 *m_pMapName;
    vgui::Label		 *m_pGameMode;
	vgui::Label 	 *m_pServerName;

    int m_iLogoBaseWide;
    int m_iLogoBaseTall;
    int m_iLogoBaseX;
    int m_iLogoBaseY;
};

#endif	// !MAPLOAD_BACKGROUND_H
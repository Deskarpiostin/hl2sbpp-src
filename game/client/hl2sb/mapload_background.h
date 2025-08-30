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

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CMapLoadBG : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE( CMapLoadBG, vgui::EditablePanel);

public:
	// Construction
	CMapLoadBG( char const *panelName );
	~CMapLoadBG();

	void setServerName( const char *serverName );

protected:
	void ApplySchemeSettings( vgui::IScheme *pScheme );

private:
	vgui::ImagePanel *m_pBackground;
	vgui::ImagePanel *m_pMapIcon;
	vgui::Label 	 *m_pServerName;
};

#endif	// !MAPLOAD_BACKGROUND_H
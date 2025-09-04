#ifndef CREATEMPDIALOG_H
#define CREATEMPDIALOG_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/PropertyPage.h>
#include <vgui_controls/PropertyDialog.h>
#include <vgui_controls/PanelListPanel.h>
#include <vgui_controls/TextEntry.h>
#include <vgui_controls/Label.h>
#include "vgui_imagebutton.h"

using namespace vgui;

class MapListPanel : public vgui::PanelListPanel
{
	typedef vgui::PanelListPanel BaseClass;
public:
	MapListPanel( vgui::Panel *parent, const char *pName );
	virtual void OnTick( void );
	virtual void PerformLayout();
	virtual void AddButton( MapListPanel *panel, const char *image, const char *command, const char *mapName );
	virtual void LoadMaps( MapListPanel *panel );
	virtual void OnCommand( const char *command );
	void CreateVMTIfMissing(const char* vtfPath);

private:
	CUtlVector< vgui::Panel * >		layoutItems;

	ImageButton *m_pSelectedButton;
	bool m_bMapsLoaded;
};

class ServerSettingsPanel : public vgui::PanelListPanel
{
	typedef vgui::PanelListPanel BaseClass;
public:
	ServerSettingsPanel( vgui::Panel *parent, const char *pName );
	virtual void OnTick( void );
	virtual void PerformLayout();

	virtual void OnCommand( const char *command );

	vgui::TextEntry* m_pMaxPlayers;
	vgui::TextEntry* m_pHostname;
	vgui::TextEntry* m_pPassword;
};

class MapList : public vgui::PropertyDialog
{
	typedef vgui::PropertyDialog BaseClass;
public:

	MapList( vgui::VPANEL *parent, const char *pName );
	void OnTick();

	virtual bool OnOK(bool applyOnly);
	virtual void OnClose();
	virtual void OnCancel();

};

class CMapList
{
public:
	virtual void		Create( vgui::VPANEL parent) = 0;
	virtual void		Destroy( void ) = 0;
	virtual void		Activate( void ) = 0;
};

extern CMapList* maplist;

#endif
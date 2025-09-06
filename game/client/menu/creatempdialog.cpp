#include "cbase.h"
#include "creatempdialog.h"
#include <vgui_controls/PropertyPage.h>
#include <vgui_controls/PropertyDialog.h>
#include <vgui_controls/PropertySheet.h>
#include <vgui_controls/PanelListPanel.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/Tooltip.h>
#include <vgui/IScheme.h>
#include <vgui/IVGui.h>
#include "hl2sb/mapload_background.h"
#include "filesystem.h"

using namespace vgui;

ConVar selmap("selmap", "", FCVAR_DEVELOPMENTONLY);
ConVar mpdialog("mpdialog", "0");

ServerSettingsPanel::ServerSettingsPanel(vgui::Panel *parent, const char *pName) : BaseClass(parent, pName)
{
    SetBounds(0, 0, 800, 640);

    const int labelWidth = 180;
    const int inputWidth = 200;
    const int rowHeight = 30;
    const int xLabel = 10;
    const int xInput = xLabel + labelWidth + 10;
    int y = 10;

    vgui::Label* lblMaxPlayers = new vgui::Label(this, "MaxPlayersLabel", "Max Players:");
    lblMaxPlayers->SetPos(xLabel, y);
    lblMaxPlayers->SetSize(labelWidth, rowHeight);
    lblMaxPlayers->SetContentAlignment(vgui::Label::a_west);

    m_pMaxPlayers = new vgui::TextEntry(this, "MaxPlayersEntry");
    m_pMaxPlayers->SetText("16"); // default / placeholder
    m_pMaxPlayers->SetSize(inputWidth, rowHeight);
    m_pMaxPlayers->SetPos(xInput, y);
    y += rowHeight + 15;

    vgui::Label* lblHostname = new vgui::Label(this, "HostnameLabel", "Hostname:");
    lblHostname->SetPos(xLabel, y);
    lblHostname->SetSize(labelWidth, rowHeight);
    lblHostname->SetContentAlignment(vgui::Label::a_west);

    m_pHostname = new vgui::TextEntry(this, "HostnameEntry");
    m_pHostname->SetText("My Server"); // default / placeholder
    m_pHostname->SetSize(inputWidth, rowHeight);
    m_pHostname->SetPos(xInput, y);
    y += rowHeight + 15;

    vgui::Label* lblPassword = new vgui::Label(this, "PasswordLabel", "Password:");
    lblPassword->SetPos(xLabel, y);
    lblPassword->SetSize(labelWidth, rowHeight);
    lblPassword->SetContentAlignment(vgui::Label::a_west);

    m_pPassword = new vgui::TextEntry(this, "PasswordEntry");
    m_pPassword->SetText(""); // placeholder
    m_pPassword->SetSize(inputWidth, rowHeight);
    m_pPassword->SetPos(xInput, y);
	m_pPassword->SetTextHidden(true);

    y += rowHeight + 15;

    vgui::Label* lblGamemode = new vgui::Label(this, "GamemodeLabel", "Gamemode:");
    lblGamemode->SetPos(xLabel, y);
    lblGamemode->SetSize(labelWidth, rowHeight);
    lblGamemode->SetContentAlignment(vgui::Label::a_west);

    m_pGamemodeCombo = new vgui::ComboBox(this, "GamemodeCombo", 6, false);
    m_pGamemodeCombo->SetSize(inputWidth, rowHeight);
    m_pGamemodeCombo->SetPos(xInput, y);
    y += rowHeight + 15;

    // populate the combobox with gamemodes found on disk
    LoadGamemodes();
}

void ServerSettingsPanel::LoadGamemodes()
{
    if (!m_pGamemodeCombo)
        return;

    int defaultIndex = m_pGamemodeCombo->AddItem("Default", NULL);
	m_pGamemodeCombo->ActivateItem( defaultIndex );

    FileFindHandle_t findhandle;
    for (const char* p = filesystem->FindFirstEx("gamemodes/*", "MOD", &findhandle); p && *p; p = filesystem->FindNext(findhandle))
    {
        if (strchr(p, '.'))
            continue;

        m_pGamemodeCombo->AddItem(p, NULL);
    }
    filesystem->FindClose(findhandle);
}

void ServerSettingsPanel::OnTick( void )
{
	BaseClass::OnTick();

	if ( !IsVisible() )
		return;
}

void ServerSettingsPanel::PerformLayout()
{
	BaseClass::PerformLayout();
}

void ServerSettingsPanel::OnCommand( const char *command )
{
	BaseClass::OnCommand( command );
}

MapListPanel::MapListPanel( vgui::Panel *parent, const char *pName ) : BaseClass( parent, pName )
{
	SetBounds( 0, 0, 800, 640 );
    m_pSelectedButton = nullptr;
	m_bMapsLoaded = false;
}

void MapListPanel::OnTick( void )
{
	BaseClass::OnTick();

	if ( !IsVisible() )
		return;

	int c = layoutItems.Count();
	for ( int i = 0; i < c; i++ )
	{
		vgui::Panel *p = layoutItems[ i ];
		p->OnTick();
	}
}

//Just perform like a SMenu
void MapListPanel::PerformLayout()
{
	BaseClass::PerformLayout();

	int w = 127;
	int h = 127;
	int x = 5;
	int y = 5;
	int gap = 2;

	int c = layoutItems.Count();
	int wide = GetWide();

	for ( int i = 0; i < c; i++ )
	{
		vgui::Panel *p = layoutItems[ i ];
		p->SetBounds( x, y, w, h );

		x += ( w + gap );
		if ( x >= wide - w )
		{
			y += ( h + gap );
			x = 5;
		}	
	}	
}

void MapListPanel::AddButton( MapListPanel *panel, const char *image, const char *command, const char *mapName )
{
	ImageButton *img = new ImageButton( panel, image, image, NULL, NULL, command );
	layoutItems.AddToTail( img );
	panel->AddItem( NULL, img );
	img->SetFgColor( Color( 180, 180, 180, 255 ) ); // unselected
	BaseTooltip *pTooltip = img->GetTooltip();
	pTooltip->SetText( mapName );
}

void MapListPanel::CreateVMTIfMissing(const char* vtfFullPath)
{
    char vmtPath[260];
    Q_strncpy(vmtPath, vtfFullPath, sizeof(vmtPath));
    Q_StripExtension(vmtPath, vmtPath, sizeof(vmtPath));
    Q_strncat(vmtPath, ".vmt", sizeof(vmtPath));

    if (filesystem->FileExists(vmtPath))
        return;

    const char* relativePath = vtfFullPath;
    if (Q_strnicmp(vtfFullPath, "materials/", 10) == 0)
        relativePath += 10;
    Q_StripExtension(relativePath, const_cast<char*>(relativePath), 260);

    char vmtContent[1024];
    Q_snprintf(vmtContent, sizeof(vmtContent),
        "\"UnlitGeneric\"\n"
        "{\n"
        "\t\"$basetexture\" \"%s\"\n"
        "\t\"$translucent\" \"1\"\n"
        "\t\"$ignorez\" \"1\"\n"
        "\t\"$vertexcolor\" \"1\"\n"
        "\t\"$vertexalpha\" \"1\"\n"
        "}\n",
        relativePath
    );
	
    FileHandle_t f = filesystem->Open(vmtPath, "w");
    if (f)
    {
        filesystem->Write(vmtContent, Q_strlen(vmtContent), f);
        filesystem->Close(f);
    }
}

void MapList::OnCancel()
{
    mpdialog.SetValue(0);
}

void MapList::OnClose()
{
    mpdialog.SetValue(0);
}

void MapListPanel::LoadMaps(MapListPanel* panel)
{
    if (m_bMapsLoaded) return;
    m_bMapsLoaded = true;

	layoutItems.RemoveAll();
	
	int mapCount = 0;
    FileFindHandle_t findhandle;
    for (const char* pMap = filesystem->FindFirstEx("maps/*.bsp", "MOD", &findhandle); pMap && *pMap; pMap = filesystem->FindNext(findhandle))
    {
		mapCount++;
        char file[ MAX_PATH ];
        Q_FileBase(pMap, file, sizeof(file));

        char vtfFull[ MAX_PATH ];
        Q_snprintf(vtfFull, sizeof(vtfFull), "materials/vgui/thumb/%s.vtf", file);
        char vtfMaterial[ MAX_PATH ];
        Q_snprintf(vtfMaterial, sizeof(vtfMaterial), "vgui/thumb/%s", file);
		char vtf_without_ex[ MAX_PATH ];
		Q_snprintf( vtf_without_ex, sizeof( vtf_without_ex ), "thumb/%s", file );

        if (filesystem->FileExists(vtfFull))
        {
            CreateVMTIfMissing(vtfFull);

            char imageCommand[ MAX_PATH ];
            Q_snprintf(imageCommand, sizeof(imageCommand), "select %s", pMap);

            AddButton(panel, vtf_without_ex, imageCommand, file);
        }
		else
		{
			Q_snprintf(vtf_without_ex, sizeof(vtf_without_ex), "thumb/placeholder");

			char imageCommand[ MAX_PATH ];
            Q_snprintf(imageCommand, sizeof(imageCommand), "select %s", pMap);

            AddButton(panel, vtf_without_ex, imageCommand, file);
		}
    }
    filesystem->FindClose(findhandle);
}

void MapListPanel::OnCommand( const char *command )
{
    if (Q_strnicmp(command, "select ", 7) == 0)
    {
        const char* mapName = command + 7;
		char mapNameNoExt[ MAX_PATH ];
		Q_strncpy( mapNameNoExt, mapName, sizeof( mapNameNoExt ) );
		Q_StripExtension( mapNameNoExt, mapNameNoExt, sizeof( mapNameNoExt ) );
		selmap.SetValue( mapNameNoExt );

		if ( m_pSelectedButton )
			m_pSelectedButton->SetFgColor( Color( 180, 180, 180, 255 ) );

		Msg( "Selected %s\n", mapNameNoExt );

        for ( int i = 0; i < layoutItems.Count(); i++ )
        {
            ImageButton* btn = dynamic_cast< ImageButton* >( layoutItems[ i ] );
            if ( !btn ) continue;

            if ( Q_strcmp( btn->GetCommand(), command ) == 0 )
            {
                btn->SetFgColor( Color( 255, 255, 255, 255 ) );

                m_pSelectedButton = btn;
                break;
            }
        }
    }
}

MapList::MapList( vgui::VPANEL *parent, const char *pName ) : BaseClass( NULL, "MapList" )
{
	SetSize( 800, 640 );
	SetTitle("New Game", true);
	
	MapListPanel *maplist = new MapListPanel( this, NULL );
	ServerSettingsPanel *info = new ServerSettingsPanel( this, NULL );
	maplist->LoadMaps( maplist );
	AddPage( maplist, "Maps" );
	AddPage( info, "Server Settings");

	vgui::ivgui()->AddTickSignal(GetVPanel(), 100);
	
	GetPropertySheet()->SetTabWidth(72);
	SetMoveable( true );
	SetVisible( true );
	SetSizeable( true );
	SetProportional( true );
	SetApplyButtonVisible( false );
}

bool MapList::OnOK( bool applyOnly )
{
    const char* mapName = selmap.GetString();
    if (mapName && mapName[0] != '\0')
	{
		int maxPlayers = 16;
		const char* hostname = "My Server";
		const char* password = "";

		ServerSettingsPanel* infoPanel = dynamic_cast<ServerSettingsPanel*>(GetPropertySheet()->GetPage(1));

		char maxPlayersBuffer[2048];
		infoPanel->m_pMaxPlayers->GetText(maxPlayersBuffer, sizeof(maxPlayersBuffer));
		maxPlayers = atoi(maxPlayersBuffer);

		char hostnameBuffer[2048];
		infoPanel->m_pHostname->GetText(hostnameBuffer, sizeof(hostnameBuffer));

		char passwordBuffer[2048];
		infoPanel->m_pPassword->GetText(passwordBuffer, sizeof(passwordBuffer));

		char gamemodeBuffer[2048] = "";
		if (infoPanel->m_pGamemodeCombo)
			infoPanel->m_pGamemodeCombo->GetText(gamemodeBuffer, sizeof(gamemodeBuffer));
		
		// EDIT: PLEASE MOVE THIS I DON'T WANT THIS TO BE HERE
		// GOD SAVE ME
		extern CMapLoadBG *pPanelBg;
		pPanelBg->setServerName( hostnameBuffer );

        char szMapCommand[2048];
        if (gamemodeBuffer[0] != '\0' && Q_strcmp(gamemodeBuffer, "Default") != 0)
        {
            Q_snprintf(szMapCommand, sizeof(szMapCommand),
                "disconnect\nwait\nwait\nsv_lan 1\nmaxplayers %i\nsv_password \"%s\"\nhostname \"%s\"\nprogress_enable\ngamemode \"%s\"\nmap %s\n",
                maxPlayers,
                password,
                hostnameBuffer,
                gamemodeBuffer,
                selmap.GetString()
            );
        }
        else
        {
            Q_snprintf(szMapCommand, sizeof(szMapCommand),
                "disconnect\nwait\nwait\nsv_lan 1\nmaxplayers %i\nsv_password \"%s\"\nhostname \"%s\"\nprogress_enable\nmap %s\ngamemode sandbox\n",
                maxPlayers,
                password,
                hostnameBuffer,
                selmap.GetString()
            );
        }

		engine->ClientCmd_Unrestricted(szMapCommand);
	}

	mpdialog.SetValue( "0" ); // cannibalism

	return true;
}

void MapList::OnTick()
{
	BaseClass::OnTick();
	
	SetVisible(mpdialog.GetBool());
}

class MapListInterface : public CMapList
{
private:
	MapList *CMapList;
public:
	MapListInterface()
	{
		CMapList = NULL;
	}
	void Create(vgui::VPANEL parent)
	{
		CMapList = new MapList( &parent, NULL );
	}
	void Destroy()
	{
		if (CMapList)
		{
			CMapList->SetParent((vgui::Panel *)NULL);
			delete CMapList;
		}
	}
	void Activate(void)
	{
		if (CMapList)
		{
			CMapList->Activate();
		}
	}
};
static MapListInterface g_MapList;
CMapList* maplist = (CMapList*)&g_MapList;
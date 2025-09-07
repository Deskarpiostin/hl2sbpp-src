#ifndef SYS_GETMODES_H
#define SYS_GETMODES_H
#ifdef _WIN32
#pragma once
#endif

#include "ivideomode.h"
typedef uint32 ScreenshotHandle;
#if defined( _WIN32 ) && !defined( _X360 )

#ifdef _WIN32
// 
// Prevent tons of unused windows definitions
//
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#define NOWINRES
#define NOSERVICE
#define NOMCX
#define NOIME
#if !defined( _X360 )
#pragma warning(push, 1)
#pragma warning(disable: 4005)
#include <windows.h>
#pragma warning(pop)
#endif
#undef PostMessage

#pragma warning( disable: 4800 )	// forcing value to bool 'true' or 'false' (performance warning)

#endif // WIN32

#elif defined(POSIX)
typedef void *HDC;
#endif

//-----------------------------------------------------------------------------
// Purpose: Functionality shared by all video modes
//-----------------------------------------------------------------------------
class CVideoMode_Common : public IVideoMode
{
public:
                        CVideoMode_Common( void );
    virtual             ~CVideoMode_Common( void );

    // Methods of IVideoMode
    virtual bool        Init( );
    virtual void        Shutdown( void );
    virtual vmode_t     *GetMode( int num );
    virtual int         GetModeCount( void );
    virtual bool        IsWindowedMode( void ) const;
    virtual void        UpdateWindowPosition( void );
    virtual void        RestoreVideo( void );
    virtual void        ReleaseVideo( void );
    virtual void        DrawNullBackground( void *hdc, int w, int h );
    virtual void        InvalidateWindow();
    virtual void        DrawStartupGraphic();
    virtual bool        CreateGameWindow( int nWidth, int nHeight, bool bWindowed );
    virtual int         GetModeWidth( void ) const;
    virtual int         GetModeHeight( void ) const;
	virtual int			GetModeStereoWidth() const;
	virtual int			GetModeStereoHeight() const;
	virtual int			GetModeUIWidth() const  OVERRIDE;
	virtual int			GetModeUIHeight() const OVERRIDE;
    virtual const vrect_t &GetClientViewRect( ) const;
    virtual void        SetClientViewRect( const vrect_t &viewRect );
    virtual void        MarkClientViewRectDirty();
    virtual void        TakeSnapshotTGA( const char *pFileName );
    virtual void        TakeSnapshotTGARect( const char *pFilename, int x, int y, int w, int h, int resampleWidth, int resampleHeight, bool bPFM, CubeMapFaceIndex_t faceIndex );
    virtual void        WriteMovieFrame( const MovieInfo_t& info );
    virtual void        TakeSnapshotJPEG( const char *pFileName, int quality );
    virtual bool        TakeSnapshotJPEGToBuffer( CUtlBuffer& buf, int quality );
protected:
    bool                GetInitialized( ) const;
    void                SetInitialized( bool init );
    void                AdjustWindow( int nWidth, int nHeight, int nBPP, bool bWindowed );
    void                ResetCurrentModeForNewResolution( int width, int height, bool bWindowed );
    int                 GetModeBPP( ) const { return 32; }
    void                DrawStartupVideo();
    void                ComputeStartupGraphicName( char *pBuf, int nBufLen );
	void				WriteScreenshotToSteam( uint8 *pImage, int cubImage, int width, int height );
	void				AddScreenshotToSteam( const char *pchFilenameJpeg, int width, int height );
#if !defined(NO_STEAM)
	void				ApplySteamScreenshotTags( ScreenshotHandle hScreenshot );
#endif

    // Finds the video mode in the list of video modes 
    int                 FindVideoMode( int nDesiredWidth, int nDesiredHeight, bool bWindowed );

    // Purpose: Returns the optimal refresh rate for the specified mode
    int                 GetRefreshRateForMode( const vmode_t *pMode );

    // Inline accessors
    vmode_t&            DefaultVideoMode();
    vmode_t&            RequestedWindowVideoMode();

private:
    // Purpose: Loads the startup graphic
    void                SetupStartupGraphic();
    void                CenterEngineWindow(void *hWndCenter, int width, int height);
	void                DrawStartupGraphic( HWND window );
    void                BlitGraphicToHDC(HDC hdc, byte *rgba, int imageWidth, int imageHeight, int x0, int y0, int x1, int y1);
    void                BlitGraphicToHDCWithAlpha(HDC hdc, byte *rgba, int imageWidth, int imageHeight, int x0, int y0, int x1, int y1);
    IVTFTexture         *LoadVTF( CUtlBuffer &temp, const char *szFileName );
    void                RecomputeClientViewRect();

    // Overridden by derived classes
    virtual void        ReleaseFullScreen( void );
    virtual void        ChangeDisplaySettingsToFullscreen( int nWidth, int nHeight, int nBPP );
    virtual void        ReadScreenPixels( int x, int y, int w, int h, void *pBuffer, ImageFormat format );

    // PFM screenshot methods
    ITexture *GetBuildCubemaps16BitTexture( void );
    ITexture *GetFullFrameFB0( void );

    void BlitHiLoScreenBuffersTo16Bit( void );
    void TakeSnapshotPFMRect( const char *pFilename, int x, int y, int w, int h, int resampleWidth, int resampleHeight, CubeMapFaceIndex_t faceIndex );

protected:
    enum
    {
#if !defined( _X360 )
        MAX_MODE_LIST = 512
#else
        MAX_MODE_LIST = 2
#endif
    };

    enum
    {
        VIDEO_MODE_DEFAULT = -1,
        VIDEO_MODE_REQUESTED_WINDOW_SIZE = -2,
        CUSTOM_VIDEO_MODES = 2
    };

    // Master mode list
    int                 m_nNumModes;
    vmode_t             m_rgModeList[MAX_MODE_LIST];
    vmode_t             m_nCustomModeList[CUSTOM_VIDEO_MODES];
    bool                m_bInitialized;
    bool                m_bPlayedStartupVideo;

    // Renderable surface information
    int                 m_nModeWidth;
    int                 m_nModeHeight;
    int                 m_nStereoWidth;
    int                 m_nStereoHeight;
    int                 m_nUIWidth;
    int                 m_nUIHeight;
	int					m_nVROverrideX;
	int					m_nVROverrideY;
#if defined( USE_SDL )
	int					m_nRenderWidth;
	int					m_nRenderHeight;
#endif
    bool                m_bWindowed;
    bool                m_bSetModeOnce;
	bool				m_bVROverride;

    // Client view rectangle
    vrect_t             m_ClientViewRect;
    bool                m_bClientViewRectDirty;

    // loading image
    IVTFTexture         *m_pHL2SBPPLogo;
};

//-----------------------------------------------------------------------------
// The version of the VideoMode class for the material system 
//-----------------------------------------------------------------------------
class CVideoMode_MaterialSystem: public CVideoMode_Common
{
public:
    typedef CVideoMode_Common BaseClass;
    
    CVideoMode_MaterialSystem( );

    virtual bool        Init( );
    virtual void        Shutdown( void );
    virtual void        SetGameWindow( void *hWnd );
    virtual bool        SetMode( int nWidth, int nHeight, bool bWindowed );
    virtual void        ReleaseVideo( void );
    virtual void        RestoreVideo( void );
    virtual void        AdjustForModeChange( void );
    virtual void        ReadScreenPixels( int x, int y, int w, int h, void *pBuffer, ImageFormat format );

private:
    virtual void        ReleaseFullScreen( void );
    virtual void        ChangeDisplaySettingsToFullscreen( int nWidth, int nHeight, int nBPP );

#ifdef WIN32
	int m_nLastCDSWidth;
	int m_nLastCDSHeight;
	int m_nLastCDSBPP;
	int m_nLastCDSFreq;
#endif
};

extern IVideoMode *videomode;

#endif
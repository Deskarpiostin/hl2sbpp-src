#ifndef PNGBUTTON_H
#define PNGBUTTON_H

#include "cbase.h"
#include <vgui_controls/Panel.h>
#include <vgui/ISurface.h>
#include <vgui/IVGui.h>
#include <vgui_controls/Panel.h>
#include "filesystem.h"
#include "libpng/png.h"

struct PngImageData {
    int textureId;
    int width;
    int height;
    bool isValid;
    
    PngImageData() : textureId(-1), width(0), height(0), isValid(false) {}
};

class PngButton : public vgui::Panel {
    DECLARE_CLASS_SIMPLE( PngButton, vgui::Panel );
    
public:
	// @ThePixelMoon: hacky hack
	const char* GetCommand() const { return m_command; }

private:
    PngImageData m_normalImage;
    PngImageData m_mouseOverImage;
    PngImageData m_mouseClickImage;
    
    char m_normalImagePath[MAX_PATH];
    char m_mouseOverImagePath[MAX_PATH];
    char m_mouseClickImagePath[MAX_PATH];
    
    bool m_hasMouseOverImage;
    bool m_hasMouseClickImage;
    bool m_hasCommand;
    bool m_bScaleImage;
    
    char m_command[MAX_PATH];
    
    PngImageData* m_currentImage;
    
    vgui::Panel* m_pParent;
    
    bool LoadPngImage(const char* filename, PngImageData& imageData);
    void SetCurrentImage(PngImageData* image);
    int CreateTextureFromPngData(unsigned char* data, int width, int height);
    
    static void PngReadFromBuffer(png_structp png_ptr, png_bytep data, png_size_t length);
    
    struct PngBufferData {
        const unsigned char* buffer;
        png_size_t size;
        png_size_t offset;
    };

public:
    PngButton(vgui::Panel *parent, const char *panelName,
              const char* normalImage, 
              const char* mouseOverImage = NULL,
              const char* mouseClickImage = NULL, 
              const char* pCmd = NULL);
    
    virtual ~PngButton();
    
    virtual void Paint() OVERRIDE;
    virtual void OnCursorEntered() OVERRIDE;
    virtual void OnCursorExited() OVERRIDE;
    virtual void OnMousePressed(vgui::MouseCode code) OVERRIDE;
    virtual void OnMouseReleased(vgui::MouseCode code) OVERRIDE;
    virtual void ApplySchemeSettings(vgui::IScheme *pScheme) OVERRIDE;
    
    void SetNormalImage();
    void SetMouseOverImage();
    void SetMouseClickImage();
    
    PngImageData* GetCurrentImage() const { return m_currentImage; }
    bool IsValid() const { return m_normalImage.isValid; }
    
    MESSAGE_FUNC_PARAMS( OnCommand, "Command", data );
};

#endif // PNGBUTTON_H
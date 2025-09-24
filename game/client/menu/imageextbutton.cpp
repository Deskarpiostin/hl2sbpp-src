#include "imageextbutton.h"
#include "vgui/IInput.h"
#include "vgui/ISurface.h"
#include "tier0/memdbgon.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

std::unordered_map<std::string, TexInfo> ImageExtButton::s_textureCache;

ImageExtButton::ImageExtButton(vgui::Panel* parent, const char* panelName,
                               const char* normalImage, const char* mouseOverImage,
                               const char* mouseClickImage, const char* pCmd)
    : vgui::Panel(parent, panelName),
      m_hasMouseOverImage(false),
      m_hasMouseClickImage(false),
      m_hasCommand(false),
      m_bScaleImage(false),
      m_currentImage(nullptr),
      m_pParent(parent)
{
    Q_strncpy(m_normalImagePath, normalImage ? normalImage : "", sizeof(m_normalImagePath));
    Q_strncpy(m_mouseOverImagePath, mouseOverImage ? mouseOverImage : "", sizeof(m_mouseOverImagePath));
    Q_strncpy(m_mouseClickImagePath, mouseClickImage ? mouseClickImage : "", sizeof(m_mouseClickImagePath));
    Q_strncpy(m_command, pCmd ? pCmd : "", sizeof(m_command));

    m_hasMouseOverImage = (mouseOverImage && mouseOverImage[0] != '\0');
    m_hasMouseClickImage = (mouseClickImage && mouseClickImage[0] != '\0');
    m_hasCommand = (pCmd && pCmd[0] != '\0');

    if (m_normalImagePath[0] != '\0')
        LoadImage(m_normalImagePath, m_normalImage);

    if (m_hasMouseOverImage && m_mouseOverImagePath[0] != '\0')
        LoadImage(m_mouseOverImagePath, m_mouseOverImage);

    if (m_hasMouseClickImage && m_mouseClickImagePath[0] != '\0')
        LoadImage(m_mouseClickImagePath, m_mouseClickImage);

    if (m_normalImage.isValid && !m_bScaleImage) {
        SetSize(m_normalImage.width, m_normalImage.height);
    }

    SetNormalImage();
}

ImageExtButton::~ImageExtButton() {
    if (m_normalImage.textureId != -1) ReleaseTextureByKey(m_normalImagePath);
    if (m_mouseOverImage.textureId != -1) ReleaseTextureByKey(m_mouseOverImagePath);
    if (m_mouseClickImage.textureId != -1) ReleaseTextureByKey(m_mouseClickImagePath);
}

bool ImageExtButton::LoadImage(const char* filename, ImageData& imageData) {
    if (!filename || filename[0] == '\0')
        return false;

    CUtlBuffer buf;
    if (!g_pFullFileSystem->ReadFile(filename, "MOD", buf)) {
        Warning("ImageExtButton: Could not open image file: %s\n", filename);
        return false;
    }

    int width = 0, height = 0, channels = 0;
    unsigned char* data = stbi_load_from_memory(
        reinterpret_cast<unsigned char*>(buf.Base()),
        buf.TellPut(),
        &width,
        &height,
        &channels,
        4 // force RGBA
    );

    if (!data) {
        Warning("ImageExtButton: Failed to decode '%s' (%s)\n", filename, stbi_failure_reason());
        return false;
    }

    imageData.textureId = CreateOrGetTextureFromImageData(filename, data, width, height);
    imageData.width = width;
    imageData.height = height;
    imageData.isValid = true;

    stbi_image_free(data);
    return true;
}

int ImageExtButton::CreateOrGetTextureFromImageData(const char* key,
                                                    unsigned char* data,
                                                    int width, int height)
{
    std::string keyStr(key);
    auto it = s_textureCache.find(keyStr);
    if (it != s_textureCache.end()) {
        return it->second.texId;
    }

    int texId = vgui::surface()->CreateNewTextureID(true);
    vgui::surface()->DrawSetTextureRGBA(texId, data, width, height, 1, false);
    TexInfo texInfo = { texId, width, height };
    s_textureCache[keyStr] = texInfo;
    return texId;
}

void ImageExtButton::ReleaseTextureByKey(const char* key) {
    std::string keyStr(key);
    auto it = s_textureCache.find(keyStr);
    if (it != s_textureCache.end()) {
        vgui::surface()->DestroyTextureID(it->second.texId);
        s_textureCache.erase(it);
    }
}

bool ImageExtButton::LoadImageIfNeeded(ImageData& imageData, const char* path, bool& flag) {
    if (!imageData.isValid && path && path[0] != '\0') {
        flag = LoadImage(path, imageData);
        return flag;
    }
    return false;
}

void ImageExtButton::SetCurrentImage(ImageData* image) {
    m_currentImage = image;
}

void ImageExtButton::SetNormalImage() { SetCurrentImage(&m_normalImage); }
void ImageExtButton::SetMouseOverImage() { SetCurrentImage(&m_mouseOverImage); }
void ImageExtButton::SetMouseClickImage() { SetCurrentImage(&m_mouseClickImage); }

void ImageExtButton::Paint() {
    if (!m_currentImage) return;

    if (m_currentImage->isValid) {
        vgui::surface()->DrawSetTexture(m_currentImage->textureId);
        vgui::surface()->DrawSetColor(Color(255, 255, 255, 255));

        if (m_bScaleImage) {
            vgui::surface()->DrawTexturedRect(0, 0, GetWide(), GetTall());
        } else {
            int w = m_currentImage->width;
            int h = m_currentImage->height;
            vgui::surface()->DrawTexturedRect(0, 0, w, h);
        }
    }
}

void ImageExtButton::OnCursorEntered() {
    if (m_hasMouseOverImage) SetMouseOverImage();
}

void ImageExtButton::OnCursorExited() {
    SetNormalImage();
}

void ImageExtButton::OnMousePressed(vgui::MouseCode code) {
    if (m_hasMouseClickImage) SetMouseClickImage();
}

void ImageExtButton::OnMouseReleased(vgui::MouseCode code) {
    if (m_hasMouseOverImage) {
        SetMouseOverImage();
    } else {
        SetNormalImage();
    }

    if (m_hasCommand) {
        PostActionSignal(new KeyValues("Command", "command", m_command));
    }
}

void ImageExtButton::ApplySchemeSettings(vgui::IScheme* pScheme) {
    BaseClass::ApplySchemeSettings(pScheme);
}

void ImageExtButton::OnCommand(KeyValues *data) {
    const char *command = data->GetString("command", "");
    if (m_pParent) {
        m_pParent->OnCommand(command);
    }
}
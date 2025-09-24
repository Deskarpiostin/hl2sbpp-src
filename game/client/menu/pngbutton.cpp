#include "pngbutton.h"
#include "tier1/KeyValues.h"
#include "tier1/utlbuffer.h"
#include "vgui/ISurface.h"
#include "vgui/IScheme.h"
#include <memory>
#include <vector>
#include <cassert>

using namespace vgui;

std::unordered_map<std::string, std::pair<int,int>> PngButton::s_textureCache;

// ----------------------------------------------------------------------------
// ctor
// ----------------------------------------------------------------------------
PngButton::PngButton(vgui::Panel *parent, const char *panelName,
                     const char* normalImage,
                     const char* mouseOverImage,
                     const char* mouseClickImage,
                     const char* pCmd)
    : BaseClass(parent, panelName),
      m_hasMouseOverImage(false),
      m_hasMouseClickImage(false),
      m_hasCommand(false),
      m_bScaleImage(true),
      m_currentImage(&m_normalImage),
      m_pParent(parent)
{
    V_memset(m_normalImagePath, 0, sizeof(m_normalImagePath));
    V_memset(m_mouseOverImagePath, 0, sizeof(m_mouseOverImagePath));
    V_memset(m_mouseClickImagePath, 0, sizeof(m_mouseClickImagePath));
    V_memset(m_command, 0, sizeof(m_command));

    if (pCmd != NULL) {
        V_strncpy(m_command, pCmd, sizeof(m_command));
        m_hasCommand = true;
    }

    if (normalImage != NULL) {
        V_strncpy(m_normalImagePath, normalImage, sizeof(m_normalImagePath));
        LoadPngImage(m_normalImagePath, m_normalImage);
    }

    if (mouseOverImage != NULL) {
        V_strncpy(m_mouseOverImagePath, mouseOverImage, sizeof(m_mouseOverImagePath));
        m_hasMouseOverImage = true;
    }

    if (mouseClickImage != NULL) {
        V_strncpy(m_mouseClickImagePath, mouseClickImage, sizeof(m_mouseClickImagePath));
        // mark, load on demand
        m_hasMouseClickImage = true;
    }

    SetNormalImage();

    SetMouseInputEnabled(true);
    SetKeyBoardInputEnabled(false);
}

// ----------------------------------------------------------------------------
// dtor
// ----------------------------------------------------------------------------
PngButton::~PngButton() {
    if (m_normalImage.isValid) {
        ReleaseTextureByKey(m_normalImagePath);
        m_normalImage.isValid = false;
        m_normalImage.textureId = -1;
    }
    if (m_mouseOverImage.isValid) {
        ReleaseTextureByKey(m_mouseOverImagePath);
        m_mouseOverImage.isValid = false;
        m_mouseOverImage.textureId = -1;
    }
    if (m_mouseClickImage.isValid) {
        ReleaseTextureByKey(m_mouseClickImagePath);
        m_mouseClickImage.isValid = false;
        m_mouseClickImage.textureId = -1;
    }
}

void PngButton::PngReadFromBuffer(png_structp png_ptr, png_bytep outBytes, png_size_t byteCount) {
    PngBufferData* reader = (PngBufferData*)png_get_io_ptr(png_ptr);
    if (!reader) {
        png_error(png_ptr, "No buffer reader");
        return;
    }

    if (reader->offset + byteCount > reader->size) {
        // avoid reading past end
        png_error(png_ptr, "Read past end of buffer");
        return;
    }

    memcpy(outBytes, reader->buffer + reader->offset, byteCount);
    reader->offset += byteCount;
}

int PngButton::CreateOrGetTextureFromPngData(const char* key, unsigned char* data, int width, int height) {
    if (!key || !*key || !data || width <= 0 || height <= 0)
        return -1;

    std::string skey(key);
    auto it = s_textureCache.find(skey);
    if (it != s_textureCache.end()) {
        // already cached; increment refcount and return existing id
        it->second.second += 1;
        return it->second.first;
    }

    int texID = surface()->CreateNewTextureID(true);
    surface()->DrawSetTextureRGBA(texID, data, width, height, 1, false);

    s_textureCache.emplace(skey, std::make_pair(texID, 1));
    return texID;
}

void PngButton::ReleaseTextureByKey(const char* key) {
    if (!key || !*key) return;
    std::string skey(key);
    auto it = s_textureCache.find(skey);
    if (it == s_textureCache.end()) return;

    it->second.second -= 1;
    if (it->second.second <= 0) {
        int texID = it->second.first;
        if (texID >= 0) {
            // delete from surface
            surface()->DeleteTextureByID(texID);
        }
        s_textureCache.erase(it);
    }
}

bool PngButton::LoadPngImage(const char* filename, PngImageData& imageData) {
    if (!filename || !*filename) return false;

    // open file
    FileHandle_t file = filesystem->Open(filename, "rb", "GAME");
    if (file == FILESYSTEM_INVALID_HANDLE) {
        Warning("PngButton: Failed to open file %s\n", filename);
        return false;
    }

    int fileSize = filesystem->Size(file);
    if (fileSize <= 0) {
        filesystem->Close(file);
        Warning("PngButton: Empty or invalid file %s\n", filename);
        return false;
    }

    std::vector<unsigned char> fileData;
    fileData.resize(fileSize);
    int bytesRead = filesystem->Read(fileData.data(), fileSize, file);
    filesystem->Close(file);
    if (bytesRead != fileSize) {
        Warning("PngButton: Failed to read file %s completely\n", filename);
        return false;
    }

    if (fileSize < 8 || png_sig_cmp(fileData.data(), 0, 8)) {
        Warning("PngButton: File %s is not a valid PNG\n", filename);
        return false;
    }

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) {
        Warning("PngButton: png_create_read_struct failed for %s\n", filename);
        return false;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        Warning("PngButton: png_create_info_struct failed for %s\n", filename);
        return false;
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        Warning("PngButton: libpng longjmp while reading %s\n", filename);
        return false;
    }

    PngBufferData bufferData;
    bufferData.buffer = fileData.data();
    bufferData.size = (png_size_t)fileData.size();
    bufferData.offset = 8; // skip signature (we already checked it)
    png_set_read_fn(png_ptr, &bufferData, PngReadFromBuffer);
    png_set_sig_bytes(png_ptr, 8);

    png_read_info(png_ptr, info_ptr);

    int width = (int)png_get_image_width(png_ptr, info_ptr);
    int height = (int)png_get_image_height(png_ptr, info_ptr);
    png_byte color_type = png_get_color_type(png_ptr, info_ptr);
    png_byte bit_depth = png_get_bit_depth(png_ptr, info_ptr);

    if (bit_depth == 16) png_set_strip_16(png_ptr);
    if (color_type == PNG_COLOR_TYPE_PALETTE) png_set_palette_to_rgb(png_ptr);
    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) png_set_expand_gray_1_2_4_to_8(png_ptr);
    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) png_set_tRNS_to_alpha(png_ptr);
    if (color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_filler(png_ptr, 0xFF, PNG_FILLER_AFTER); // force alpha channel
    if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
        png_set_gray_to_rgb(png_ptr);

    png_read_update_info(png_ptr, info_ptr);

    const size_t stride = (size_t)width * 4;
    std::vector<unsigned char> imageBuffer;
    try {
        imageBuffer.resize((size_t)height * stride);
    } catch (...) {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        Warning("PngButton: Out of memory allocating imageBuffer for %s\n", filename);
        return false;
    }

    std::vector<png_bytep> row_pointers(height);
    for (int y = 0; y < height; ++y) {
        row_pointers[y] = imageBuffer.data() + (size_t)y * stride;
    }

    png_read_image(png_ptr, row_pointers.data());
    png_read_end(png_ptr, NULL);

    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

    const int MAX_BG_TEXTURE_SIZE = 2048;
    int texW = 1;
    while (texW < width) texW <<= 1;
    int texH = 1;
    while (texH < height) texH <<= 1;
    if (texW > MAX_BG_TEXTURE_SIZE) texW = MAX_BG_TEXTURE_SIZE;
    if (texH > MAX_BG_TEXTURE_SIZE) texH = MAX_BG_TEXTURE_SIZE;

    std::vector<unsigned char> finalData;
    unsigned char* finalPtr = nullptr;
    int finalW = width;
    int finalH = height;

    if (texW != width || texH != height) {
        try {
            finalData.resize((size_t)texW * texH * 4);
        } catch (...) {
            Warning("PngButton: Out of memory resizing %s\n", filename);
            return false;
        }

        float xRatio = (float)width / (float)texW;
        float yRatio = (float)height / (float)texH;
        for (int y = 0; y < texH; ++y) {
            for (int x = 0; x < texW; ++x) {
                int srcX = (int)(x * xRatio);
                int srcY = (int)(y * yRatio);
                if (srcX >= width) srcX = width - 1;
                if (srcY >= height) srcY = height - 1;

                size_t srcIdx = ((size_t)srcY * width + srcX) * 4;
                size_t dstIdx = ((size_t)y * texW + x) * 4;
                finalData[dstIdx + 0] = imageBuffer[srcIdx + 0];
                finalData[dstIdx + 1] = imageBuffer[srcIdx + 1];
                finalData[dstIdx + 2] = imageBuffer[srcIdx + 2];
                finalData[dstIdx + 3] = imageBuffer[srcIdx + 3];
            }
        }
        finalPtr = finalData.data();
        finalW = texW;
        finalH = texH;
    } else {
        finalPtr = imageBuffer.data();
        finalW = width;
        finalH = height;
    }

    int textureId = CreateOrGetTextureFromPngData(filename, finalPtr, finalW, finalH);

    if (textureId != -1) {
        imageData.textureId = textureId;
        imageData.width = finalW;
        imageData.height = finalH;
        imageData.isValid = true;
        return true;
    }

    DevWarning("PngButton: Failed to create texture for %s\n", filename);
    return false;
}

bool PngButton::LoadImageIfNeeded(PngImageData& imageData, const char* path, bool& flag) {
    if (!flag || !path || !*path) return false;
    if (imageData.isValid) return true; // already loaded
    return LoadPngImage(path, imageData);
}

void PngButton::SetCurrentImage(PngImageData* image) {
    if (image && image->isValid) {
        m_currentImage = image;
        Repaint();
    }
}

void PngButton::Paint() {
    if (!m_currentImage || !m_currentImage->isValid) {
        return;
    }

    int wide, tall;
    GetSize(wide, tall);

    surface()->DrawSetTexture(m_currentImage->textureId);
    surface()->DrawSetColor(255, 255, 255, 255);

    if (m_bScaleImage) {
        surface()->DrawTexturedRect(0, 0, wide, tall);
    } else {
        int x = (wide - m_currentImage->width) / 2;
        int y = (tall - m_currentImage->height) / 2;
        surface()->DrawTexturedRect(x, y, x + m_currentImage->width, y + m_currentImage->height);
    }
}

// ----------------------------------------------------------------------------
// input events, fucker
// ----------------------------------------------------------------------------
void PngButton::OnCursorEntered() {
    if (m_hasMouseOverImage) {
        if (!m_mouseOverImage.isValid) {
            LoadImageIfNeeded(m_mouseOverImage, m_mouseOverImagePath, m_hasMouseOverImage);
        }
        SetMouseOverImage();
    }
    BaseClass::OnCursorEntered();
}

void PngButton::OnCursorExited() {
    if (m_hasMouseOverImage) {
        SetNormalImage();
    }
    BaseClass::OnCursorExited();
}

void PngButton::OnMousePressed(vgui::MouseCode code) {
    if (code == MOUSE_LEFT && m_hasMouseClickImage) {
        // lazy load click image cuz why not
        if (!m_mouseClickImage.isValid) {
            LoadImageIfNeeded(m_mouseClickImage, m_mouseClickImagePath, m_hasMouseClickImage);
        }
        SetMouseClickImage();
    }
    BaseClass::OnMousePressed(code);
}

void PngButton::OnMouseReleased(vgui::MouseCode code) {
    if (code == MOUSE_LEFT) {
        if (m_hasCommand && m_pParent) {
            KeyValues *msg = new KeyValues("Command");
            msg->SetString("command", m_command);
            PostActionSignal(msg);
            m_pParent->OnCommand(m_command);
        }

        if (m_hasMouseClickImage) {
            SetNormalImage();
        }
    }
    BaseClass::OnMouseReleased(code);
}

void PngButton::ApplySchemeSettings(vgui::IScheme *pScheme) {
    BaseClass::ApplySchemeSettings(pScheme);
    SetBgColor(Color(0,0,0,0));
    SetPaintBackgroundEnabled(false);
}

void PngButton::SetNormalImage() {
    if (!m_normalImage.isValid && m_normalImagePath[0]) {
        LoadImageIfNeeded(m_normalImage, m_normalImagePath, m_hasCommand); // m_hasCommand not directly related
    }
    SetCurrentImage(&m_normalImage);
}

void PngButton::SetMouseOverImage() {
    if (m_hasMouseOverImage) {
        if (!m_mouseOverImage.isValid) {
            LoadImageIfNeeded(m_mouseOverImage, m_mouseOverImagePath, m_hasMouseOverImage);
        }
        SetCurrentImage(&m_mouseOverImage);
    }
}

void PngButton::SetMouseClickImage() {
    if (m_hasMouseClickImage) {
        if (!m_mouseClickImage.isValid) {
            LoadImageIfNeeded(m_mouseClickImage, m_mouseClickImagePath, m_hasMouseClickImage);
        }
        SetCurrentImage(&m_mouseClickImage);
    }
}

void PngButton::OnCommand(KeyValues *data) {
    const char *command = data->GetString("command", "");
    if (m_pParent) {
        m_pParent->OnCommand(command);
    }
}

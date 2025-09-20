#include "pngbutton.h"
#include "tier1/KeyValues.h"
#include "tier1/utlbuffer.h"
#include "vgui/ISurface.h"
#include "vgui/IScheme.h"

using namespace vgui;

PngButton::PngButton(vgui::Panel *parent, const char *panelName,
                     const char* normalImage, 
                     const char* mouseOverImage,
                     const char* mouseClickImage, 
                     const char* pCmd)
    : BaseClass(parent, panelName)
{
    m_pParent = parent;
    SetParent(parent);
    m_bScaleImage = true;
    m_currentImage = &m_normalImage;
    
    m_hasMouseOverImage = false;
    m_hasMouseClickImage = false;
    m_hasCommand = false;
    
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
        if (LoadPngImage(m_mouseOverImagePath, m_mouseOverImage)) {
            m_hasMouseOverImage = true;
        }
    }
    
    if (mouseClickImage != NULL) {
        V_strncpy(m_mouseClickImagePath, mouseClickImage, sizeof(m_mouseClickImagePath));
        if (LoadPngImage(m_mouseClickImagePath, m_mouseClickImage)) {
            m_hasMouseClickImage = true;
        }
    }
    
    SetNormalImage();
    
    SetMouseInputEnabled(true);
    SetKeyBoardInputEnabled(false);
}

PngButton::~PngButton() {
    if (m_normalImage.textureId != -1) {
        surface()->DeleteTextureByID(m_normalImage.textureId);
    }
    if (m_mouseOverImage.textureId != -1) {
        surface()->DeleteTextureByID(m_mouseOverImage.textureId);
    }
    if (m_mouseClickImage.textureId != -1) {
        surface()->DeleteTextureByID(m_mouseClickImage.textureId);
    }
}

void PngButton::PngReadFromBuffer(png_structp png_ptr, png_bytep data, png_size_t length) {
    PngBufferData* bufferData = (PngBufferData*)png_get_io_ptr(png_ptr);
    
    if (bufferData->offset + length > bufferData->size) {
        png_error(png_ptr, "Read past end of buffer");
        return;
    }
    
    V_memcpy(data, bufferData->buffer + bufferData->offset, length);
    bufferData->offset += length;
}

bool PngButton::LoadPngImage(const char* filename, PngImageData& imageData) {
    FileHandle_t file = filesystem->Open(filename, "rb", "GAME");
    if (file == FILESYSTEM_INVALID_HANDLE) {
        Warning("PngButton: Failed to open file %s\n", filename);
        return false;
    }

    int fileSize = filesystem->Size(file);
    CUtlBuffer buffer;
    buffer.EnsureCapacity(fileSize);

    int bytesRead = filesystem->Read(buffer.Base(), fileSize, file);
    filesystem->Close(file);

    if (bytesRead != fileSize) {
        Warning("PngButton: Failed to read file %s completely\n", filename);
        return false;
    }

    buffer.SeekPut(CUtlBuffer::SEEK_HEAD, bytesRead);

    // check PNG signature
    if (fileSize < 8 || png_sig_cmp((png_const_bytep)buffer.Base(), 0, 8)) {
        Warning("PngButton: File %s is not a valid PNG\n", filename);
        return false;
    }

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (!png_ptr) return false;

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        return false;
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        return false;
    }

    PngBufferData bufferData;
    bufferData.buffer = (const unsigned char*)buffer.Base();
    bufferData.size = fileSize;
    bufferData.offset = 8; // skip signature

    png_set_read_fn(png_ptr, &bufferData, PngReadFromBuffer);
    png_set_sig_bytes(png_ptr, 8);
    png_read_info(png_ptr, info_ptr);

    int width = png_get_image_width(png_ptr, info_ptr);
    int height = png_get_image_height(png_ptr, info_ptr);
    png_byte color_type = png_get_color_type(png_ptr, info_ptr);
    png_byte bit_depth = png_get_bit_depth(png_ptr, info_ptr);

    // normalize formats
    if (bit_depth == 16) png_set_strip_16(png_ptr);
    if (color_type == PNG_COLOR_TYPE_PALETTE) png_set_palette_to_rgb(png_ptr);
    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) png_set_expand_gray_1_2_4_to_8(png_ptr);
    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) png_set_tRNS_to_alpha(png_ptr);
    if (color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_filler(png_ptr, 0xFF, PNG_FILLER_AFTER); // force alpha
    if (color_type == PNG_COLOR_TYPE_GRAY || color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
        png_set_gray_to_rgb(png_ptr);

    png_read_update_info(png_ptr, info_ptr);

    // Force stride to width*4
    const size_t stride = width * 4;
    unsigned char* imageBuffer = new unsigned char[height * stride];

    png_bytep* row_pointers = new png_bytep[height];
    for (int y = 0; y < height; y++) {
        row_pointers[y] = imageBuffer + y * stride;
    }

    png_read_image(png_ptr, row_pointers);
    png_read_end(png_ptr, NULL);

    int textureId = CreateTextureFromPngData(imageBuffer, width, height);

    delete[] row_pointers;
    delete[] imageBuffer;
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

    if (textureId != -1) {
        imageData.textureId = textureId;
        imageData.width = width;
        imageData.height = height;
        imageData.isValid = true;
        //Msg("PngButton: Loaded %s (%dx%d) -> texture %d\n", filename, width, height, textureId);
        return true;
    }

    DevWarning("PngButton: Failed to create texture for %s\n", filename);
    return false;
}

int PngButton::CreateTextureFromPngData(unsigned char* data, int width, int height) {
    int textureId = surface()->CreateNewTextureID(true);
    surface()->DrawSetTextureRGBA(textureId, data, width, height, false, false);
    
    return textureId; 
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

void PngButton::OnCursorEntered() {
    if (m_hasMouseOverImage) {
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
    
    SetBgColor(Color(0, 0, 0, 0));
    SetPaintBackgroundEnabled(false);
}

void PngButton::SetNormalImage() {
    SetCurrentImage(&m_normalImage);
}

void PngButton::SetMouseOverImage() {
    if (m_hasMouseOverImage) {
        SetCurrentImage(&m_mouseOverImage);
    }
}

void PngButton::SetMouseClickImage() {
    if (m_hasMouseClickImage) {
        SetCurrentImage(&m_mouseClickImage);
    }
}

void PngButton::OnCommand(KeyValues *data) {
    const char *command = data->GetString("command", "");
    if (m_pParent) {
        m_pParent->OnCommand(command);
    }
}
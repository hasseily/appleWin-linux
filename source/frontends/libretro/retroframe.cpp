#include "StdAfx.h"
#include "frontends/libretro/retroframe.h"
#include "frontends/libretro/environment.h"
#include "frontends/common2/utils.h"

#include "Interface.h"
#include "Core.h"
#include "Utilities.h"

#include "../resource/resource.h"
#include "rom_Apple2.inl"
#include "rom_Apple2_Plus.inl"
#include "rom_Apple2_JPlus.inl"
#include "rom_Apple2e.inl"
#include "rom_Apple2e_Enhanced.inl"
#include "rom_PRAVETS82.inl"
#include "rom_PRAVETS8M.inl"
#include "rom_PRAVETS8C.inl"
#include "rom_TK3000e.inl"
#include "rom_Base64A.inl"
#include "bmp_CHARSET8C.inl"
#include "bmp_CHARSET8M.inl"
#include "bmp_CHARSET82.inl"

#include <fstream>

namespace
{

  void readFileToBuffer(const std::string & filename, std::vector<char> & buffer)
  {
    std::ifstream file(filename.c_str(), std::ios::binary | std::ios::ate);
    const std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    buffer.resize(size);
    file.read(buffer.data(), size);
  }

  template<typename T>
  T getAs(const std::vector<char> & buffer, const size_t offset)
  {
    if (offset + sizeof(T) > buffer.size())
    {
      throw std::runtime_error("Invalid bitmap");
    }
    const T * ptr = reinterpret_cast<const T *>(buffer.data() + offset);
    return * ptr;
  }

  // libretro cannot parse BMP with 1 bpp
  // simple version implemented here
  bool getBitmapData(const std::vector<char> & buffer, int32_t & width, int32_t & height, uint16_t & bpp, const char * & data, uint32_t & size)
  {
    if (buffer.size() < 2)
    {
      return false;
    }

    if (buffer[0] != 0x42 || buffer[1] != 0x4D)
    {
      return false;
    }

    const uint32_t fileSize = getAs<uint32_t>(buffer, 2);
    if (fileSize != buffer.size())
    {
      return false;
    }

    const uint32_t offset = getAs<uint32_t>(buffer, 10);
    const uint32_t header = getAs<uint32_t>(buffer, 14);
    if (header != 40)
    {
      return false;
    }

    width = getAs<int32_t>(buffer, 18);
    height = getAs<int32_t>(buffer, 22);
    bpp = getAs<uint16_t>(buffer, 28);
    const uint32_t imageSize = getAs<uint32_t>(buffer, 34);
    if (offset + imageSize > fileSize)
    {
      return false;
    }
    data = buffer.data() + offset;
    size = imageSize;
    return true;
  }

}

namespace ra2
{

  RetroFrame::RetroFrame(const common2::EmulatorOptions & options)
  : common2::CommonFrame(options)
  {
  }

  void RetroFrame::FrameRefreshStatus(int drawflags)
  {
    if (drawflags & DRAW_TITLE)
    {
      GetAppleWindowTitle();
      display_message(g_pAppTitle.c_str());
    }
  }

  void RetroFrame::VideoPresentScreen()
  {
    // this should not be necessary
    // either libretro handles it
    // or we should change AW
    // but for now, there is no alternative
    for (size_t row = 0; row < myHeight; ++row)
    {
      const uint8_t * src = myFrameBuffer + row * myPitch;
      uint8_t * dst = myVideoBuffer.data() + (myHeight - row - 1) * myPitch;
      memcpy(dst, src, myPitch);
    }

    video_cb(myVideoBuffer.data() + myOffset, myBorderlessWidth, myBorderlessHeight, myPitch);
  }

  void RetroFrame::Initialize(bool resetVideoState)
  {
    CommonFrame::Initialize(resetVideoState);
    FrameRefreshStatus(DRAW_TITLE);

    Video & video = GetVideo();

    myBorderlessWidth = video.GetFrameBufferBorderlessWidth();
    myBorderlessHeight = video.GetFrameBufferBorderlessHeight();
    const size_t borderWidth = video.GetFrameBufferBorderWidth();
    const size_t borderHeight = video.GetFrameBufferBorderHeight();
    const size_t width = video.GetFrameBufferWidth();
    myHeight = video.GetFrameBufferHeight();

    myFrameBuffer = video.GetFrameBuffer();

    myPitch = width * sizeof(bgra_t);
    myOffset = (width * borderHeight + borderWidth) * sizeof(bgra_t);

    const size_t size = myHeight * myPitch;
    myVideoBuffer.resize(size);
  }

  void RetroFrame::Destroy()
  {
    CommonFrame::Destroy();
    myFrameBuffer = nullptr;
    myVideoBuffer.clear();
  }

  void RetroFrame::GetBitmap(LPCSTR lpBitmapName, LONG cb, LPVOID lpvBits)
  {
    if (strcmp(lpBitmapName, "CHARSET8C") == 0)
      memcpy(lpvBits, _bmp_CHARSET8C, cb);
    else if (strcmp(lpBitmapName, "CHARSET8M") == 0)
      memcpy(lpvBits, _bmp_CHARSET8M, cb);
    else if (strcmp(lpBitmapName, "CHARSET82") == 0)
      memcpy(lpvBits, _bmp_CHARSET82, cb);
    else
    {
      log_cb(RETRO_LOG_INFO, "RA2: %s. Missing bitmap '%s'\n", __FUNCTION__, lpBitmapName);
      memset(lpvBits, 0, cb);
    }
  }

  int RetroFrame::FrameMessageBox(LPCSTR lpText, LPCSTR lpCaption, UINT uType)
  {
    display_message(lpText, 60);
    log_cb(RETRO_LOG_INFO, "RA2: %s: %s - %s\n", __FUNCTION__, lpCaption, lpText);
    return IDOK;
  }

  BYTE* RetroFrame::GetResource(WORD id, LPCSTR lpType, DWORD expectedSize)
  {
      switch (id)
      {
      case IDR_APPLE2_ROM:
          return _rom_Apple2;
      case IDR_APPLE2_PLUS_ROM:
          return _rom_Apple2_Plus;
      case IDR_APPLE2_JPLUS_ROM:
          return _rom_Apple2_JPlus;
      case IDR_APPLE2E_ROM:
          return _rom_Apple2e;
      case IDR_APPLE2E_ENHANCED_ROM:
          return _rom_Apple2e_Enhanced;
      case IDR_PRAVETS_82_ROM:
          return _rom_PRAVETS82;
      case IDR_PRAVETS_8M_ROM:
          return _rom_PRAVETS8M;
      case IDR_PRAVETS_8C_ROM:
          return _rom_PRAVETS8C;
      case IDR_TK3000_2E_ROM:
          return _rom_TK3000e;
      case IDR_BASE_64A_ROM:
          return _rom_Base64A;
      default:
          return NULL;
      }
  }

  void RetroFrame::SetFullSpeed(const bool /* value */)
  {
    // do nothing
  }
  
  bool RetroFrame::CanDoFullSpeed()
  {
    // Let libretro deal with it.
    return false;
  }

  void RetroFrame::Begin()
  {
    const common2::RestoreCurrentDirectory restoreChDir;
    common2::CommonFrame::Begin();
  }


}

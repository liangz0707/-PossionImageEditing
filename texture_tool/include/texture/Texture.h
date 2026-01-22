#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "texture/TextureExport.h"

namespace texture {

enum class PixelType {
  UInt8,
  UInt16,
  Float32
};

enum class TextureDimension {
  Tex1D,
  Tex2D,
  Tex3D,
  Cube
};

enum class ChannelLayout {
  Unknown,
  R,
  RG,
  RGB,
  RGBA
};

struct TextureMipLevel {
  int width = 0;
  int height = 0;
  int depth = 1;
  std::vector<uint8_t> pixels;
};

class TEXTURE_API TextureResource {
 public:
  TextureResource();
  TextureResource(TextureDimension dimension,
                  ChannelLayout layout,
                  int channelCount,
                  PixelType type);

  void Reset(TextureDimension dimension,
             ChannelLayout layout,
             int channelCount,
             PixelType type);
  void Clear();

  TextureDimension Dimension() const;
  ChannelLayout Layout() const;
  int ChannelCount() const;
  PixelType Type() const;

  void SetLayout(ChannelLayout layout);
  void SetChannelCount(int channelCount);
  void SetType(PixelType type);

  void SetName(const std::string& name);
  const std::string& Name() const;

  void SetChannelNames(std::vector<std::string> names);
  const std::vector<std::string>& ChannelNames() const;

  int Width() const;
  int Height() const;
  int Depth() const;
  int MipCount() const;

  TextureMipLevel& Mip(int index);
  const TextureMipLevel& Mip(int index) const;
  std::vector<TextureMipLevel>& Mips();
  const std::vector<TextureMipLevel>& Mips() const;

  void AllocateMipChain(int width, int height, int depth, int mipLevels);

 private:
  TextureDimension dimension_ = TextureDimension::Tex2D;
  ChannelLayout layout_ = ChannelLayout::RGBA;
  int channelCount_ = 4;
  PixelType type_ = PixelType::UInt8;
  std::vector<TextureMipLevel> mips_;
  std::vector<std::string> channelNames_;
  std::string name_;
};

TEXTURE_API int ChannelCount(ChannelLayout layout);
TEXTURE_API ChannelLayout GuessLayout(int channelCount);
TEXTURE_API size_t BytesPerChannel(PixelType type);
TEXTURE_API size_t BytesPerPixel(int channelCount, PixelType type);
TEXTURE_API size_t MipByteSize(int width,
                               int height,
                               int depth,
                               int channelCount,
                               PixelType type);

TEXTURE_API const char* ToString(PixelType type);
TEXTURE_API const char* ToString(ChannelLayout layout);
TEXTURE_API const char* ToString(TextureDimension dimension);

}  // namespace texture

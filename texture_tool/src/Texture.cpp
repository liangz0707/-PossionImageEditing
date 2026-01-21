#include "texture/Texture.h"

#include <algorithm>

namespace texture {

int Texture::Width() const {
  if (mips.empty()) {
    return 0;
  }
  return mips[0].width;
}

int Texture::Height() const {
  if (mips.empty()) {
    return 0;
  }
  return mips[0].height;
}

int Texture::Depth() const {
  if (mips.empty()) {
    return 0;
  }
  return mips[0].depth;
}

int Texture::MipCount() const {
  return static_cast<int>(mips.size());
}

int ChannelCount(ColorFormat format) {
  return static_cast<int>(format);
}

size_t BytesPerChannel(PixelType type) {
  switch (type) {
    case PixelType::UInt8:
      return 1;
    case PixelType::UInt16:
      return 2;
    case PixelType::Float32:
      return 4;
    default:
      return 0;
  }
}

size_t BytesPerPixel(ColorFormat format, PixelType type) {
  return static_cast<size_t>(ChannelCount(format)) * BytesPerChannel(type);
}

size_t MipByteSize(int width,
                   int height,
                   int depth,
                   ColorFormat format,
                   PixelType type) {
  if (width <= 0 || height <= 0 || depth <= 0) {
    return 0;
  }
  return static_cast<size_t>(width) * static_cast<size_t>(height) *
         static_cast<size_t>(depth) * BytesPerPixel(format, type);
}

Texture CreateTexture2D(int width,
                        int height,
                        ColorFormat format,
                        PixelType type,
                        int mipLevels) {
  Texture texture;
  texture.dimension = TextureDimension::Tex2D;
  texture.format = format;
  texture.type = type;

  int levels = std::max(1, mipLevels);
  texture.mips.resize(levels);

  int levelWidth = std::max(1, width);
  int levelHeight = std::max(1, height);
  for (int level = 0; level < levels; ++level) {
    MipLevel& mip = texture.mips[level];
    mip.width = levelWidth;
    mip.height = levelHeight;
    mip.depth = 1;
    mip.pixels.resize(MipByteSize(levelWidth, levelHeight, 1, format, type));

    levelWidth = std::max(1, levelWidth / 2);
    levelHeight = std::max(1, levelHeight / 2);
  }

  return texture;
}

const char* ToString(PixelType type) {
  switch (type) {
    case PixelType::UInt8:
      return "uint8";
    case PixelType::UInt16:
      return "uint16";
    case PixelType::Float32:
      return "float32";
    default:
      return "unknown";
  }
}

const char* ToString(ColorFormat format) {
  switch (format) {
    case ColorFormat::R:
      return "r";
    case ColorFormat::RG:
      return "rg";
    case ColorFormat::RGB:
      return "rgb";
    case ColorFormat::RGBA:
      return "rgba";
    default:
      return "unknown";
  }
}

const char* ToString(TextureDimension dimension) {
  switch (dimension) {
    case TextureDimension::Tex1D:
      return "1d";
    case TextureDimension::Tex2D:
      return "2d";
    case TextureDimension::Tex3D:
      return "3d";
    case TextureDimension::Cube:
      return "cube";
    default:
      return "unknown";
  }
}

}  // namespace texture

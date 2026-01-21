#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace texture {

enum class PixelType {
  UInt8,
  UInt16,
  Float32
};

enum class ColorFormat {
  R = 1,
  RG = 2,
  RGB = 3,
  RGBA = 4
};

enum class TextureDimension {
  Tex1D,
  Tex2D,
  Tex3D,
  Cube
};

struct MipLevel {
  int width = 0;
  int height = 0;
  int depth = 1;
  std::vector<uint8_t> pixels;
};

struct Texture {
  TextureDimension dimension = TextureDimension::Tex2D;
  ColorFormat format = ColorFormat::RGBA;
  PixelType type = PixelType::UInt8;
  std::vector<MipLevel> mips;

  int Width() const;
  int Height() const;
  int Depth() const;
  int MipCount() const;
};

int ChannelCount(ColorFormat format);
size_t BytesPerChannel(PixelType type);
size_t BytesPerPixel(ColorFormat format, PixelType type);
size_t MipByteSize(int width, int height, int depth, ColorFormat format, PixelType type);

Texture CreateTexture2D(int width,
                        int height,
                        ColorFormat format,
                        PixelType type,
                        int mipLevels);

const char* ToString(PixelType type);
const char* ToString(ColorFormat format);
const char* ToString(TextureDimension dimension);

}  // namespace texture

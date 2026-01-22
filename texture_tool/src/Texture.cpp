#include "texture/Texture.h"

#include <algorithm>

namespace texture {

TextureResource::TextureResource() = default;

TextureResource::TextureResource(TextureDimension dimension,
                                 ChannelLayout layout,
                                 int channelCount,
                                 PixelType type) {
  Reset(dimension, layout, channelCount, type);
}

void TextureResource::Reset(TextureDimension dimension,
                            ChannelLayout layout,
                            int channelCount,
                            PixelType type) {
  dimension_ = dimension;
  layout_ = layout;
  channelCount_ = channelCount;
  type_ = type;
  mips_.clear();
  channelNames_.clear();
}

void TextureResource::Clear() {
  mips_.clear();
}

TextureDimension TextureResource::Dimension() const {
  return dimension_;
}

ChannelLayout TextureResource::Layout() const {
  return layout_;
}

int TextureResource::ChannelCount() const {
  return channelCount_;
}

PixelType TextureResource::Type() const {
  return type_;
}

void TextureResource::SetLayout(ChannelLayout layout) {
  layout_ = layout;
}

void TextureResource::SetChannelCount(int channelCount) {
  channelCount_ = channelCount;
}

void TextureResource::SetType(PixelType type) {
  type_ = type;
}

void TextureResource::SetName(const std::string& name) {
  name_ = name;
}

const std::string& TextureResource::Name() const {
  return name_;
}

void TextureResource::SetChannelNames(std::vector<std::string> names) {
  channelNames_ = std::move(names);
}

const std::vector<std::string>& TextureResource::ChannelNames() const {
  return channelNames_;
}

int TextureResource::Width() const {
  if (mips_.empty()) {
    return 0;
  }
  return mips_[0].width;
}

int TextureResource::Height() const {
  if (mips_.empty()) {
    return 0;
  }
  return mips_[0].height;
}

int TextureResource::Depth() const {
  if (mips_.empty()) {
    return 0;
  }
  return mips_[0].depth;
}

int TextureResource::MipCount() const {
  return static_cast<int>(mips_.size());
}

TextureMipLevel& TextureResource::Mip(int index) {
  return mips_.at(static_cast<size_t>(index));
}

const TextureMipLevel& TextureResource::Mip(int index) const {
  return mips_.at(static_cast<size_t>(index));
}

std::vector<TextureMipLevel>& TextureResource::Mips() {
  return mips_;
}

const std::vector<TextureMipLevel>& TextureResource::Mips() const {
  return mips_;
}

void TextureResource::AllocateMipChain(int width,
                                       int height,
                                       int depth,
                                       int mipLevels) {
  const int levels = std::max(1, mipLevels);
  mips_.resize(levels);

  int levelWidth = std::max(1, width);
  int levelHeight = std::max(1, height);
  int levelDepth = std::max(1, depth);
  for (int level = 0; level < levels; ++level) {
    TextureMipLevel& mip = mips_[level];
    mip.width = levelWidth;
    mip.height = levelHeight;
    mip.depth = levelDepth;
    mip.pixels.resize(MipByteSize(levelWidth,
                                  levelHeight,
                                  levelDepth,
                                  channelCount_,
                                  type_));

    levelWidth = std::max(1, levelWidth / 2);
    levelHeight = std::max(1, levelHeight / 2);
    levelDepth = std::max(1, levelDepth / 2);
  }
}

int ChannelCount(ChannelLayout layout) {
  switch (layout) {
    case ChannelLayout::R:
      return 1;
    case ChannelLayout::RG:
      return 2;
    case ChannelLayout::RGB:
      return 3;
    case ChannelLayout::RGBA:
      return 4;
    case ChannelLayout::Unknown:
    default:
      return 0;
  }
}

ChannelLayout GuessLayout(int channelCount) {
  switch (channelCount) {
    case 1:
      return ChannelLayout::R;
    case 2:
      return ChannelLayout::RG;
    case 3:
      return ChannelLayout::RGB;
    case 4:
      return ChannelLayout::RGBA;
    default:
      return ChannelLayout::Unknown;
  }
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

size_t BytesPerPixel(int channelCount, PixelType type) {
  if (channelCount <= 0) {
    return 0;
  }
  return static_cast<size_t>(channelCount) * BytesPerChannel(type);
}

size_t MipByteSize(int width,
                   int height,
                   int depth,
                   int channelCount,
                   PixelType type) {
  if (width <= 0 || height <= 0 || depth <= 0 || channelCount <= 0) {
    return 0;
  }
  return static_cast<size_t>(width) * static_cast<size_t>(height) *
         static_cast<size_t>(depth) * BytesPerPixel(channelCount, type);
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

const char* ToString(ChannelLayout layout) {
  switch (layout) {
    case ChannelLayout::R:
      return "r";
    case ChannelLayout::RG:
      return "rg";
    case ChannelLayout::RGB:
      return "rgb";
    case ChannelLayout::RGBA:
      return "rgba";
    case ChannelLayout::Unknown:
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

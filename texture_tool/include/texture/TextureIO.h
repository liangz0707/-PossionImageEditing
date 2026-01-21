#pragma once

#include <string>

#include "texture/Texture.h"

namespace texture {

struct LoadOptions {
  PixelType preferredType = PixelType::UInt8;
  ColorFormat preferredFormat = ColorFormat::RGBA;
  bool flipY = false;
  bool generateMipmaps = false;
  int maxMipLevels = 0;
};

struct SaveOptions {
  PixelType outputType = PixelType::UInt8;
  ColorFormat outputFormat = ColorFormat::RGBA;
  int jpgQuality = 90;
  bool flipY = false;
  bool writeMipChain = false;
};

bool LoadTextureFromFile(const std::string& path,
                         const LoadOptions& options,
                         Texture* outTexture,
                         std::string* error);

bool SaveTextureToFile(const Texture& texture,
                       const std::string& path,
                       const SaveOptions& options,
                       std::string* error);

Texture ConvertTexture(const Texture& source,
                       ColorFormat targetFormat,
                       PixelType targetType);

bool GenerateMipmaps(Texture* texture, int maxLevels, std::string* error);

}  // namespace texture

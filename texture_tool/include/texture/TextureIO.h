#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "texture/Texture.h"

namespace texture {

struct TextureLoadOptions {
  PixelType preferredType = PixelType::UInt8;
  ChannelLayout preferredLayout = ChannelLayout::Unknown;
  int preferredChannelCount = 0;
  bool flipY = false;
  bool generateMipmaps = false;
  int maxMipLevels = 0;
  std::string mipGenerator = "box";
};

struct TextureSaveOptions {
  PixelType outputType = PixelType::UInt8;
  ChannelLayout outputLayout = ChannelLayout::Unknown;
  int outputChannelCount = 0;
  bool flipY = false;
  int jpgQuality = 90;
  std::string compression;
};

class TEXTURE_API TextureCodec {
 public:
  virtual ~TextureCodec() = default;
  virtual const char* Name() const = 0;
  virtual std::vector<std::string> Extensions() const = 0;
  virtual bool CanLoad() const = 0;
  virtual bool CanSave() const = 0;
  virtual bool Load(const std::string& path,
                    const TextureLoadOptions& options,
                    TextureResource* outTexture,
                    std::string* error) const = 0;
  virtual bool Save(const TextureResource& texture,
                    const std::string& path,
                    const TextureSaveOptions& options,
                    std::string* error) const = 0;
};

struct TextureCodecInfo {
  std::string name;
  std::vector<std::string> extensions;
  bool canLoad = false;
  bool canSave = false;
};

class TEXTURE_API TextureCodecRegistry {
 public:
  void Register(std::unique_ptr<TextureCodec> codec);
  std::vector<TextureCodecInfo> ListCodecs() const;
  bool Load(const std::string& path,
            const TextureLoadOptions& options,
            TextureResource* outTexture,
            std::string* error) const;
  bool Save(const TextureResource& texture,
            const std::string& path,
            const TextureSaveOptions& options,
            std::string* error) const;

 private:
  std::vector<std::unique_ptr<TextureCodec>> codecs_;
};

struct MipmapOptions {
  int maxLevels = 0;
};

class TEXTURE_API MipmapGenerator {
 public:
  virtual ~MipmapGenerator() = default;
  virtual const char* Name() const = 0;
  virtual bool Generate(TextureResource* texture,
                        const MipmapOptions& options,
                        std::string* error) const = 0;
};

class TEXTURE_API MipmapGeneratorRegistry {
 public:
  void Register(std::unique_ptr<MipmapGenerator> generator);
  const MipmapGenerator* Find(const std::string& name) const;
  std::vector<std::string> List() const;

 private:
  std::vector<std::unique_ptr<MipmapGenerator>> generators_;
};

class TEXTURE_API TextureToolkit {
 public:
  TextureToolkit();

  TextureCodecRegistry& Codecs();
  MipmapGeneratorRegistry& MipmapGenerators();

  bool Load(const std::string& path,
            const TextureLoadOptions& options,
            TextureResource* outTexture,
            std::string* error) const;
  bool Save(const TextureResource& texture,
            const std::string& path,
            const TextureSaveOptions& options,
            std::string* error) const;
  bool GenerateMipmaps(TextureResource* texture,
                       const std::string& generator,
                       int maxLevels,
                       std::string* error) const;

 private:
  TextureCodecRegistry codecRegistry_;
  MipmapGeneratorRegistry mipmapRegistry_;
};

TEXTURE_API TextureResource ConvertTexture(const TextureResource& source,
                                           int targetChannelCount,
                                           PixelType targetType,
                                           ChannelLayout targetLayout);

}  // namespace texture

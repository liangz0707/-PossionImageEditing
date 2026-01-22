#include "texture/TextureIO.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>

#include <qoi.h>
#include <stb_image.h>
#include <stb_image_write.h>
#include <tinyexr.h>

#ifdef TEXTURE_HAS_OIIO
  #include <OpenImageIO/imageio.h>
#endif

namespace texture {
namespace {

std::string ToLower(std::string value) {
  std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
    return static_cast<char>(std::tolower(c));
  });
  return value;
}

std::string GetExtension(const std::string& path) {
  const std::string::size_type dot = path.find_last_of('.');
  if (dot == std::string::npos) {
    return std::string();
  }
  return ToLower(path.substr(dot + 1));
}

bool ReadBinaryFile(const std::string& path,
                    std::vector<uint8_t>* out,
                    std::string* error) {
  if (!out) {
    if (error) {
      *error = "Output buffer is null.";
    }
    return false;
  }
  std::ifstream file(path, std::ios::binary | std::ios::ate);
  if (!file) {
    if (error) {
      *error = "Failed to open input file: " + path;
    }
    return false;
  }
  const std::streamsize size = file.tellg();
  if (size <= 0) {
    if (error) {
      *error = "Input file is empty: " + path;
    }
    return false;
  }
  out->resize(static_cast<size_t>(size));
  file.seekg(0, std::ios::beg);
  if (!file.read(reinterpret_cast<char*>(out->data()), size)) {
    if (error) {
      *error = "Failed to read input file: " + path;
    }
    return false;
  }
  return true;
}

bool WriteBinaryFile(const std::string& path,
                     const uint8_t* data,
                     size_t size,
                     std::string* error) {
  std::ofstream file(path, std::ios::binary);
  if (!file) {
    if (error) {
      *error = "Failed to open output file: " + path;
    }
    return false;
  }
  file.write(reinterpret_cast<const char*>(data),
             static_cast<std::streamsize>(size));
  if (!file) {
    if (error) {
      *error = "Failed to write output file: " + path;
    }
    return false;
  }
  return true;
}

float ReadChannel(const uint8_t* data, PixelType type) {
  if (type == PixelType::UInt8) {
    return static_cast<float>(*data) / 255.0f;
  }
  if (type == PixelType::UInt16) {
    uint16_t value = 0;
    std::memcpy(&value, data, sizeof(value));
    return static_cast<float>(value) / 65535.0f;
  }
  float value = 0.0f;
  std::memcpy(&value, data, sizeof(value));
  return value;
}

void WriteChannel(uint8_t* data, PixelType type, float value) {
  if (type != PixelType::Float32) {
    value = std::min(1.0f, std::max(0.0f, value));
  }

  if (type == PixelType::UInt8) {
    const uint8_t out = static_cast<uint8_t>(std::lround(value * 255.0f));
    std::memcpy(data, &out, sizeof(out));
    return;
  }
  if (type == PixelType::UInt16) {
    const uint16_t out = static_cast<uint16_t>(std::lround(value * 65535.0f));
    std::memcpy(data, &out, sizeof(out));
    return;
  }
  std::memcpy(data, &value, sizeof(value));
}

void ReadPixel(const uint8_t* base,
               int channels,
               PixelType type,
               size_t pixelIndex,
               std::vector<float>* out) {
  if (!out) {
    return;
  }
  out->assign(static_cast<size_t>(channels), 0.0f);
  const size_t bytesPerChannel = BytesPerChannel(type);
  const size_t pixelStride =
      static_cast<size_t>(channels) * bytesPerChannel;
  const uint8_t* pixel = base + pixelIndex * pixelStride;
  for (int c = 0; c < channels; ++c) {
    (*out)[static_cast<size_t>(c)] =
        ReadChannel(pixel + static_cast<size_t>(c) * bytesPerChannel, type);
  }
}

void WritePixel(uint8_t* base,
                int channels,
                PixelType type,
                size_t pixelIndex,
                const std::vector<float>& values) {
  const size_t bytesPerChannel = BytesPerChannel(type);
  const size_t pixelStride =
      static_cast<size_t>(channels) * bytesPerChannel;
  uint8_t* pixel = base + pixelIndex * pixelStride;
  for (int c = 0; c < channels; ++c) {
    const float value =
        c < static_cast<int>(values.size()) ? values[static_cast<size_t>(c)] : 0.0f;
    WriteChannel(pixel + static_cast<size_t>(c) * bytesPerChannel, type, value);
  }
}

std::vector<uint8_t> FlipY(const TextureMipLevel& level,
                           int channelCount,
                           PixelType type) {
  std::vector<uint8_t> flipped(level.pixels.size());
  const size_t rowBytes =
      static_cast<size_t>(level.width) * BytesPerPixel(channelCount, type);
  for (int y = 0; y < level.height; ++y) {
    const size_t srcOffset = static_cast<size_t>(y) * rowBytes;
    const size_t dstOffset =
        static_cast<size_t>(level.height - 1 - y) * rowBytes;
    std::memcpy(flipped.data() + dstOffset,
                level.pixels.data() + srcOffset,
                rowBytes);
  }
  return flipped;
}

std::vector<std::string> DefaultChannelNames(ChannelLayout layout,
                                             int channels) {
  if (layout == ChannelLayout::R && channels == 1) {
    return {"R"};
  }
  if (layout == ChannelLayout::RG && channels == 2) {
    return {"R", "G"};
  }
  if (layout == ChannelLayout::RGB && channels == 3) {
    return {"R", "G", "B"};
  }
  if (layout == ChannelLayout::RGBA && channels == 4) {
    return {"R", "G", "B", "A"};
  }

  std::vector<std::string> names;
  names.reserve(static_cast<size_t>(channels));
  for (int i = 0; i < channels; ++i) {
    names.push_back("C" + std::to_string(i));
  }
  return names;
}

int ResolveChannelCount(int requested,
                        ChannelLayout layout,
                        int fallback) {
  if (requested > 0) {
    return requested;
  }
  const int layoutChannels = ChannelCount(layout);
  if (layoutChannels > 0) {
    return layoutChannels;
  }
  return fallback;
}

ChannelLayout ResolveLayout(ChannelLayout requested,
                            int channelCount,
                            ChannelLayout fallback) {
  if (requested != ChannelLayout::Unknown) {
    return requested;
  }
  if (fallback != ChannelLayout::Unknown) {
    return fallback;
  }
  return GuessLayout(channelCount);
}

float LuminanceFromRGB(float r, float g, float b) {
  return 0.2126f * r + 0.7152f * g + 0.0722f * b;
}

bool Downsample2x2(const TextureMipLevel& src,
                   int channels,
                   PixelType type,
                   TextureMipLevel* dst) {
  if (!dst) {
    return false;
  }

  const int srcWidth = src.width;
  const int srcHeight = src.height;
  const int dstWidth = std::max(1, srcWidth / 2);
  const int dstHeight = std::max(1, srcHeight / 2);

  dst->width = dstWidth;
  dst->height = dstHeight;
  dst->depth = 1;
  dst->pixels.resize(MipByteSize(dstWidth, dstHeight, 1, channels, type));

  std::vector<float> pixel;
  std::vector<float> accum(static_cast<size_t>(channels), 0.0f);

  for (int y = 0; y < dstHeight; ++y) {
    for (int x = 0; x < dstWidth; ++x) {
      std::fill(accum.begin(), accum.end(), 0.0f);
      int sampleCount = 0;

      for (int oy = 0; oy < 2; ++oy) {
        const int srcY = std::min(srcHeight - 1, y * 2 + oy);
        for (int ox = 0; ox < 2; ++ox) {
          const int srcX = std::min(srcWidth - 1, x * 2 + ox);
          const size_t srcIndex =
              static_cast<size_t>(srcY) * static_cast<size_t>(srcWidth) +
              static_cast<size_t>(srcX);

          ReadPixel(src.pixels.data(), channels, type, srcIndex, &pixel);
          for (int c = 0; c < channels; ++c) {
            accum[static_cast<size_t>(c)] += pixel[static_cast<size_t>(c)];
          }
          ++sampleCount;
        }
      }

      const float inv = 1.0f / static_cast<float>(sampleCount);
      for (float& value : accum) {
        value *= inv;
      }

      const size_t dstIndex =
          static_cast<size_t>(y) * static_cast<size_t>(dstWidth) +
          static_cast<size_t>(x);
      WritePixel(dst->pixels.data(), channels, type, dstIndex, accum);
    }
  }

  return true;
}

bool NearestDownsample(const TextureMipLevel& src,
                       int channels,
                       PixelType type,
                       TextureMipLevel* dst) {
  if (!dst) {
    return false;
  }

  const int srcWidth = src.width;
  const int srcHeight = src.height;
  const int dstWidth = std::max(1, srcWidth / 2);
  const int dstHeight = std::max(1, srcHeight / 2);

  dst->width = dstWidth;
  dst->height = dstHeight;
  dst->depth = 1;
  dst->pixels.resize(MipByteSize(dstWidth, dstHeight, 1, channels, type));

  std::vector<float> pixel;
  for (int y = 0; y < dstHeight; ++y) {
    for (int x = 0; x < dstWidth; ++x) {
      const int srcX = std::min(srcWidth - 1, x * 2);
      const int srcY = std::min(srcHeight - 1, y * 2);
      const size_t srcIndex =
          static_cast<size_t>(srcY) * static_cast<size_t>(srcWidth) +
          static_cast<size_t>(srcX);

      ReadPixel(src.pixels.data(), channels, type, srcIndex, &pixel);
      const size_t dstIndex =
          static_cast<size_t>(y) * static_cast<size_t>(dstWidth) +
          static_cast<size_t>(x);
      WritePixel(dst->pixels.data(), channels, type, dstIndex, pixel);
    }
  }

  return true;
}

class BoxFilterMipmapGenerator final : public MipmapGenerator {
 public:
  const char* Name() const override {
    return "box";
  }

  bool Generate(TextureResource* texture,
                const MipmapOptions& options,
                std::string* error) const override {
    if (!texture || texture->Mips().empty()) {
      if (error) {
        *error = "Texture has no base mip level.";
      }
      return false;
    }
    if (texture->Dimension() != TextureDimension::Tex2D ||
        texture->Mip(0).depth != 1) {
      if (error) {
        *error = "Only 2D textures with depth 1 are supported.";
      }
      return false;
    }

    int maxChain = 1;
    int width = texture->Mip(0).width;
    int height = texture->Mip(0).height;
    while (width > 1 || height > 1) {
      width = std::max(1, width / 2);
      height = std::max(1, height / 2);
      ++maxChain;
    }

    int targetLevels = maxChain;
    if (options.maxLevels > 0) {
      targetLevels = std::min(options.maxLevels, maxChain);
    }

    texture->Mips().resize(1);

    for (int level = 1; level < targetLevels; ++level) {
      TextureMipLevel next;
      if (!Downsample2x2(texture->Mip(level - 1),
                         texture->ChannelCount(),
                         texture->Type(),
                         &next)) {
        if (error) {
          *error = "Failed to generate mipmap level.";
        }
        return false;
      }
      texture->Mips().push_back(std::move(next));
    }

    return true;
  }
};

class NearestMipmapGenerator final : public MipmapGenerator {
 public:
  const char* Name() const override {
    return "nearest";
  }

  bool Generate(TextureResource* texture,
                const MipmapOptions& options,
                std::string* error) const override {
    if (!texture || texture->Mips().empty()) {
      if (error) {
        *error = "Texture has no base mip level.";
      }
      return false;
    }
    if (texture->Dimension() != TextureDimension::Tex2D ||
        texture->Mip(0).depth != 1) {
      if (error) {
        *error = "Only 2D textures with depth 1 are supported.";
      }
      return false;
    }

    int maxChain = 1;
    int width = texture->Mip(0).width;
    int height = texture->Mip(0).height;
    while (width > 1 || height > 1) {
      width = std::max(1, width / 2);
      height = std::max(1, height / 2);
      ++maxChain;
    }

    int targetLevels = maxChain;
    if (options.maxLevels > 0) {
      targetLevels = std::min(options.maxLevels, maxChain);
    }

    texture->Mips().resize(1);

    for (int level = 1; level < targetLevels; ++level) {
      TextureMipLevel next;
      if (!NearestDownsample(texture->Mip(level - 1),
                             texture->ChannelCount(),
                             texture->Type(),
                             &next)) {
        if (error) {
          *error = "Failed to generate mipmap level.";
        }
        return false;
      }
      texture->Mips().push_back(std::move(next));
    }

    return true;
  }
};

class StbCodec final : public TextureCodec {
 public:
  const char* Name() const override {
    return "stb_image";
  }

  std::vector<std::string> Extensions() const override {
    return {"png", "jpg", "jpeg", "bmp", "tga", "psd", "gif", "hdr", "pic"};
  }

  bool CanLoad() const override {
    return true;
  }

  bool CanSave() const override {
    return true;
  }

  bool Load(const std::string& path,
            const TextureLoadOptions& options,
            TextureResource* outTexture,
            std::string* error) const override {
    if (!outTexture) {
      if (error) {
        *error = "Output texture pointer is null.";
      }
      return false;
    }

    stbi_set_flip_vertically_on_load(options.flipY ? 1 : 0);

    int width = 0;
    int height = 0;
    int channels = 0;
    int desiredChannels = 0;
    if (options.preferredChannelCount > 0 &&
        options.preferredChannelCount <= 4) {
      desiredChannels = options.preferredChannelCount;
    } else if (options.preferredLayout != ChannelLayout::Unknown) {
      const int layoutChannels = ChannelCount(options.preferredLayout);
      if (layoutChannels > 0 && layoutChannels <= 4) {
        desiredChannels = layoutChannels;
      }
    }

    const bool isHdr = stbi_is_hdr(path.c_str()) != 0;

    void* data = nullptr;
    PixelType loadedType = PixelType::UInt8;
    if (options.preferredType == PixelType::Float32 || isHdr) {
      data = stbi_loadf(path.c_str(), &width, &height, &channels, desiredChannels);
      loadedType = PixelType::Float32;
    } else if (options.preferredType == PixelType::UInt16) {
      data = stbi_load_16(path.c_str(), &width, &height, &channels, desiredChannels);
      loadedType = PixelType::UInt16;
    } else {
      data = stbi_load(path.c_str(), &width, &height, &channels, desiredChannels);
      loadedType = PixelType::UInt8;
    }

    if (!data) {
      if (error) {
        *error = stbi_failure_reason();
      }
      return false;
    }

    const int finalChannels = desiredChannels > 0 ? desiredChannels : channels;
    ChannelLayout layout =
        ResolveLayout(options.preferredLayout, finalChannels, GuessLayout(finalChannels));

    TextureResource texture(TextureDimension::Tex2D, layout, finalChannels, loadedType);
    texture.SetName(path);
    texture.Mips().resize(1);
    texture.Mip(0).width = width;
    texture.Mip(0).height = height;
    texture.Mip(0).depth = 1;
    texture.Mip(0).pixels.resize(
        MipByteSize(width, height, 1, finalChannels, loadedType));
    std::memcpy(texture.Mip(0).pixels.data(),
                data,
                texture.Mip(0).pixels.size());
    texture.SetChannelNames(DefaultChannelNames(layout, finalChannels));

    stbi_image_free(data);

    *outTexture = std::move(texture);
    return true;
  }

  bool Save(const TextureResource& texture,
            const std::string& path,
            const TextureSaveOptions& options,
            std::string* error) const override {
    if (texture.Mips().empty()) {
      if (error) {
        *error = "Texture has no data to save.";
      }
      return false;
    }

    const std::string ext = GetExtension(path);
    if (ext != "png" && ext != "bmp" && ext != "tga" &&
        ext != "jpg" && ext != "jpeg" && ext != "hdr") {
      return false;
    }

    const bool isHdr = ext == "hdr";
    PixelType targetType = isHdr ? PixelType::Float32 : PixelType::UInt8;
    const int targetChannels = ResolveChannelCount(
        options.outputChannelCount,
        options.outputLayout,
        texture.ChannelCount());
    if (targetChannels <= 0 || targetChannels > 4) {
      if (error) {
        *error = "stb_image_write only supports 1-4 channels.";
      }
      return false;
    }

    const ChannelLayout targetLayout = ResolveLayout(
        options.outputLayout, targetChannels, texture.Layout());

    TextureResource converted =
        ConvertTexture(texture, targetChannels, targetType, targetLayout);
    const TextureMipLevel& base = converted.Mip(0);
    const int width = base.width;
    const int height = base.height;

    std::vector<uint8_t> staging;
    const uint8_t* data = base.pixels.data();
    if (options.flipY) {
      staging = FlipY(base, targetChannels, targetType);
      data = staging.data();
    }

    int success = 0;
    if (ext == "png") {
      success = stbi_write_png(path.c_str(),
                               width,
                               height,
                               targetChannels,
                               data,
                               width * targetChannels);
    } else if (ext == "bmp") {
      success = stbi_write_bmp(path.c_str(),
                               width,
                               height,
                               targetChannels,
                               data);
    } else if (ext == "tga") {
      success = stbi_write_tga(path.c_str(),
                               width,
                               height,
                               targetChannels,
                               data);
    } else if (ext == "jpg" || ext == "jpeg") {
      success = stbi_write_jpg(path.c_str(),
                               width,
                               height,
                               targetChannels,
                               data,
                               options.jpgQuality);
    } else if (ext == "hdr") {
      const size_t count = static_cast<size_t>(width) *
                           static_cast<size_t>(height) *
                           static_cast<size_t>(targetChannels);
      std::vector<float> floatPixels(count);
      std::memcpy(floatPixels.data(),
                  data,
                  count * sizeof(float));
      success = stbi_write_hdr(path.c_str(),
                               width,
                               height,
                               targetChannels,
                               floatPixels.data());
    }

    if (success == 0) {
      if (error) {
        *error = "Failed to write output file.";
      }
      return false;
    }

    return true;
  }
};

class PnmCodec final : public TextureCodec {
 public:
  const char* Name() const override {
    return "pnm";
  }

  std::vector<std::string> Extensions() const override {
    return {"pnm", "ppm", "pgm"};
  }

  bool CanLoad() const override {
    return true;
  }

  bool CanSave() const override {
    return true;
  }

  bool Load(const std::string& path,
            const TextureLoadOptions& options,
            TextureResource* outTexture,
            std::string* error) const override {
    if (!outTexture) {
      if (error) {
        *error = "Output texture pointer is null.";
      }
      return false;
    }

    std::ifstream file(path, std::ios::binary);
    if (!file) {
      if (error) {
        *error = "Failed to open input file: " + path;
      }
      return false;
    }

    auto readToken = [&file, error]() -> std::string {
      std::string token;
      char c = 0;
      while (file.get(c)) {
        if (std::isspace(static_cast<unsigned char>(c))) {
          if (!token.empty()) {
            break;
          }
          continue;
        }
        if (c == '#') {
          std::string comment;
          std::getline(file, comment);
          continue;
        }
        token.push_back(c);
      }
      return token;
    };

    const std::string magic = readToken();
    if (magic != "P5" && magic != "P6") {
      if (error) {
        *error = "Unsupported PNM magic: " + magic;
      }
      return false;
    }

    const std::string widthToken = readToken();
    const std::string heightToken = readToken();
    const std::string maxToken = readToken();
    if (widthToken.empty() || heightToken.empty() || maxToken.empty()) {
      if (error) {
        *error = "Invalid PNM header.";
      }
      return false;
    }

    const int width = std::stoi(widthToken);
    const int height = std::stoi(heightToken);
    const int maxValue = std::stoi(maxToken);
    if (maxValue != 255) {
      if (error) {
        *error = "Only 8-bit PNM (max=255) is supported.";
      }
      return false;
    }

    const int channels = magic == "P5" ? 1 : 3;
    const size_t dataSize =
        static_cast<size_t>(width) * static_cast<size_t>(height) *
        static_cast<size_t>(channels);

    std::vector<uint8_t> pixels(dataSize);
    if (!file.read(reinterpret_cast<char*>(pixels.data()),
                   static_cast<std::streamsize>(dataSize))) {
      if (error) {
        *error = "Failed to read PNM data.";
      }
      return false;
    }

    TextureResource texture(TextureDimension::Tex2D,
                            GuessLayout(channels),
                            channels,
                            PixelType::UInt8);
    texture.SetName(path);
    texture.Mips().resize(1);
    texture.Mip(0).width = width;
    texture.Mip(0).height = height;
    texture.Mip(0).depth = 1;
    texture.Mip(0).pixels = std::move(pixels);
    texture.SetChannelNames(DefaultChannelNames(texture.Layout(), channels));

    if (options.flipY) {
      texture.Mip(0).pixels =
          FlipY(texture.Mip(0), channels, texture.Type());
    }

    *outTexture = std::move(texture);
    return true;
  }

  bool Save(const TextureResource& texture,
            const std::string& path,
            const TextureSaveOptions& options,
            std::string* error) const override {
    if (texture.Mips().empty()) {
      if (error) {
        *error = "Texture has no data to save.";
      }
      return false;
    }

    const std::string ext = GetExtension(path);
    if (ext != "pnm" && ext != "ppm" && ext != "pgm") {
      return false;
    }

    int targetChannels = ResolveChannelCount(
        options.outputChannelCount,
        options.outputLayout,
        texture.ChannelCount());
    if (targetChannels != 1 && targetChannels != 3) {
      targetChannels = texture.ChannelCount() == 1 ? 1 : 3;
    }

    ChannelLayout targetLayout = ResolveLayout(
        options.outputLayout, targetChannels, texture.Layout());
    TextureResource converted =
        ConvertTexture(texture, targetChannels, PixelType::UInt8, targetLayout);

    const TextureMipLevel& base = converted.Mip(0);
    const int width = base.width;
    const int height = base.height;

    std::vector<uint8_t> staging;
    const uint8_t* data = base.pixels.data();
    if (options.flipY) {
      staging = FlipY(base, targetChannels, converted.Type());
      data = staging.data();
    }

    std::string header = targetChannels == 1 ? "P5\n" : "P6\n";
    header += std::to_string(width) + " " + std::to_string(height) + "\n255\n";

    std::vector<uint8_t> output;
    output.reserve(header.size() + base.pixels.size());
    output.insert(output.end(), header.begin(), header.end());
    output.insert(output.end(), data, data + base.pixels.size());

    return WriteBinaryFile(path, output.data(), output.size(), error);
  }
};

class QoiCodec final : public TextureCodec {
 public:
  const char* Name() const override {
    return "qoi";
  }

  std::vector<std::string> Extensions() const override {
    return {"qoi"};
  }

  bool CanLoad() const override {
    return true;
  }

  bool CanSave() const override {
    return true;
  }

  bool Load(const std::string& path,
            const TextureLoadOptions& options,
            TextureResource* outTexture,
            std::string* error) const override {
    if (!outTexture) {
      if (error) {
        *error = "Output texture pointer is null.";
      }
      return false;
    }

    std::vector<uint8_t> fileData;
    if (!ReadBinaryFile(path, &fileData, error)) {
      return false;
    }

    qoi_desc desc = {};
    void* decoded =
        qoi_decode(fileData.data(), static_cast<int>(fileData.size()), &desc, 0);
    if (!decoded) {
      if (error) {
        *error = "Failed to decode QOI.";
      }
      return false;
    }

    const int channels = desc.channels;
    TextureResource texture(TextureDimension::Tex2D,
                            GuessLayout(channels),
                            channels,
                            PixelType::UInt8);
    texture.SetName(path);
    texture.Mips().resize(1);
    texture.Mip(0).width = static_cast<int>(desc.width);
    texture.Mip(0).height = static_cast<int>(desc.height);
    texture.Mip(0).depth = 1;
    texture.Mip(0).pixels.resize(
        MipByteSize(texture.Mip(0).width,
                    texture.Mip(0).height,
                    1,
                    channels,
                    PixelType::UInt8));
    std::memcpy(texture.Mip(0).pixels.data(),
                decoded,
                texture.Mip(0).pixels.size());
    texture.SetChannelNames(DefaultChannelNames(texture.Layout(), channels));
    std::free(decoded);

    if (options.flipY) {
      texture.Mip(0).pixels =
          FlipY(texture.Mip(0), channels, texture.Type());
    }

    *outTexture = std::move(texture);
    return true;
  }

  bool Save(const TextureResource& texture,
            const std::string& path,
            const TextureSaveOptions& options,
            std::string* error) const override {
    if (texture.Mips().empty()) {
      if (error) {
        *error = "Texture has no data to save.";
      }
      return false;
    }

    const std::string ext = GetExtension(path);
    if (ext != "qoi") {
      return false;
    }

    int targetChannels = ResolveChannelCount(
        options.outputChannelCount,
        options.outputLayout,
        texture.ChannelCount());
    if (targetChannels != 3 && targetChannels != 4) {
      targetChannels = texture.ChannelCount() == 3 ? 3 : 4;
    }

    ChannelLayout targetLayout = ResolveLayout(
        options.outputLayout, targetChannels, texture.Layout());
    TextureResource converted =
        ConvertTexture(texture, targetChannels, PixelType::UInt8, targetLayout);

    const TextureMipLevel& base = converted.Mip(0);

    std::vector<uint8_t> staging;
    const uint8_t* data = base.pixels.data();
    if (options.flipY) {
      staging = FlipY(base, targetChannels, converted.Type());
      data = staging.data();
    }

    qoi_desc desc = {};
    desc.width = static_cast<unsigned int>(base.width);
    desc.height = static_cast<unsigned int>(base.height);
    desc.channels = static_cast<unsigned char>(targetChannels);
    desc.colorspace = QOI_SRGB;

    int outSize = 0;
    void* encoded = qoi_encode(data, &desc, &outSize);
    if (!encoded || outSize <= 0) {
      if (error) {
        *error = "Failed to encode QOI.";
      }
      if (encoded) {
        std::free(encoded);
      }
      return false;
    }

    const bool ok = WriteBinaryFile(
        path, reinterpret_cast<uint8_t*>(encoded), static_cast<size_t>(outSize), error);
    std::free(encoded);
    return ok;
  }
};

class TinyExrCodec final : public TextureCodec {
 public:
  const char* Name() const override {
    return "tinyexr";
  }

  std::vector<std::string> Extensions() const override {
    return {"exr"};
  }

  bool CanLoad() const override {
    return true;
  }

  bool CanSave() const override {
    return true;
  }

  bool Load(const std::string& path,
            const TextureLoadOptions& options,
            TextureResource* outTexture,
            std::string* error) const override {
    if (!outTexture) {
      if (error) {
        *error = "Output texture pointer is null.";
      }
      return false;
    }

    EXRVersion version;
    if (ParseEXRVersionFromFile(&version, path.c_str()) != TINYEXR_SUCCESS) {
      if (error) {
        *error = "Failed to parse EXR version.";
      }
      return false;
    }

    EXRHeader header;
    InitEXRHeader(&header);
    const char* err = nullptr;
    int ret = ParseEXRHeaderFromFile(&header, &version, path.c_str(), &err);
    if (ret != TINYEXR_SUCCESS) {
      if (error) {
        *error = err ? err : "Failed to parse EXR header.";
      }
      if (err) {
        FreeEXRErrorMessage(err);
      }
      return false;
    }

    header.requested_pixel_types =
        static_cast<int*>(std::malloc(sizeof(int) * header.num_channels));
    for (int i = 0; i < header.num_channels; ++i) {
      header.requested_pixel_types[i] = TINYEXR_PIXELTYPE_FLOAT;
    }

    EXRImage image;
    InitEXRImage(&image);
    ret = LoadEXRImageFromFile(&image, &header, path.c_str(), &err);
    if (ret != TINYEXR_SUCCESS) {
      if (error) {
        *error = err ? err : "Failed to load EXR image.";
      }
      if (err) {
        FreeEXRErrorMessage(err);
      }
      FreeEXRHeader(&header);
      return false;
    }

    const int channels = header.num_channels;
    const int width = image.width;
    const int height = image.height;

    TextureResource base(TextureDimension::Tex2D,
                         GuessLayout(channels),
                         channels,
                         PixelType::Float32);
    base.SetName(path);
    base.Mips().resize(1);
    base.Mip(0).width = width;
    base.Mip(0).height = height;
    base.Mip(0).depth = 1;
    base.Mip(0).pixels.resize(
        MipByteSize(width, height, 1, channels, PixelType::Float32));

    std::vector<std::string> channelNames;
    channelNames.reserve(static_cast<size_t>(channels));
    for (int i = 0; i < channels; ++i) {
      channelNames.push_back(header.channels[i].name);
    }
    base.SetChannelNames(channelNames);

    float* outPixels = reinterpret_cast<float*>(base.Mip(0).pixels.data());
    for (int c = 0; c < channels; ++c) {
      const float* srcChannel =
          reinterpret_cast<const float*>(image.images[c]);
      for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
          const size_t idx = static_cast<size_t>(y) * width + x;
          outPixels[idx * static_cast<size_t>(channels) +
                    static_cast<size_t>(c)] = srcChannel[idx];
        }
      }
    }

    FreeEXRImage(&image);
    FreeEXRHeader(&header);

    if (options.flipY) {
      base.Mip(0).pixels =
          FlipY(base.Mip(0), channels, base.Type());
    }

    const int targetChannels = ResolveChannelCount(
        options.preferredChannelCount,
        options.preferredLayout,
        base.ChannelCount());
    const ChannelLayout targetLayout = ResolveLayout(
        options.preferredLayout, targetChannels, base.Layout());

    TextureResource converted =
        ConvertTexture(base, targetChannels, options.preferredType, targetLayout);
    *outTexture = std::move(converted);
    return true;
  }

  bool Save(const TextureResource& texture,
            const std::string& path,
            const TextureSaveOptions& options,
            std::string* error) const override {
    if (texture.Mips().empty()) {
      if (error) {
        *error = "Texture has no data to save.";
      }
      return false;
    }

    const std::string ext = GetExtension(path);
    if (ext != "exr") {
      return false;
    }

    int targetChannels = ResolveChannelCount(
        options.outputChannelCount,
        options.outputLayout,
        texture.ChannelCount());
    if (targetChannels <= 0) {
      targetChannels = texture.ChannelCount();
    }
    ChannelLayout targetLayout = ResolveLayout(
        options.outputLayout, targetChannels, texture.Layout());

    TextureResource converted =
        ConvertTexture(texture, targetChannels, PixelType::Float32, targetLayout);
    TextureMipLevel base = converted.Mip(0);

    if (options.flipY) {
      base.pixels = FlipY(base, targetChannels, converted.Type());
    }

    std::vector<std::vector<float>> channels(
        static_cast<size_t>(targetChannels),
        std::vector<float>(static_cast<size_t>(base.width) *
                           static_cast<size_t>(base.height)));

    const float* pixels = reinterpret_cast<const float*>(base.pixels.data());
    for (int y = 0; y < base.height; ++y) {
      for (int x = 0; x < base.width; ++x) {
        const size_t idx = static_cast<size_t>(y) * base.width + x;
        for (int c = 0; c < targetChannels; ++c) {
          channels[static_cast<size_t>(c)][idx] =
              pixels[idx * static_cast<size_t>(targetChannels) +
                     static_cast<size_t>(c)];
        }
      }
    }

    EXRHeader header;
    InitEXRHeader(&header);
    header.num_channels = targetChannels;
    header.channels = static_cast<EXRChannelInfo*>(
        std::malloc(sizeof(EXRChannelInfo) * targetChannels));
    header.pixel_types =
        static_cast<int*>(std::malloc(sizeof(int) * targetChannels));
    header.requested_pixel_types =
        static_cast<int*>(std::malloc(sizeof(int) * targetChannels));

    std::vector<std::string> channelNames = converted.ChannelNames();
    if (channelNames.size() != static_cast<size_t>(targetChannels)) {
      channelNames = DefaultChannelNames(targetLayout, targetChannels);
    }

    for (int i = 0; i < targetChannels; ++i) {
      std::strncpy(header.channels[i].name,
                   channelNames[static_cast<size_t>(i)].c_str(),
                   sizeof(header.channels[i].name));
      header.channels[i].name[sizeof(header.channels[i].name) - 1] = '\0';
      header.pixel_types[i] = TINYEXR_PIXELTYPE_FLOAT;
      header.requested_pixel_types[i] = TINYEXR_PIXELTYPE_FLOAT;
    }

    const std::string compression = ToLower(options.compression);
    if (compression == "zip") {
      header.compression_type = TINYEXR_COMPRESSIONTYPE_ZIP;
    } else if (compression == "zips") {
      header.compression_type = TINYEXR_COMPRESSIONTYPE_ZIPS;
    } else if (compression == "piz") {
      header.compression_type = TINYEXR_COMPRESSIONTYPE_PIZ;
    } else if (compression == "rle") {
      header.compression_type = TINYEXR_COMPRESSIONTYPE_RLE;
    } else {
      header.compression_type = TINYEXR_COMPRESSIONTYPE_NONE;
    }

    EXRImage image;
    InitEXRImage(&image);
    image.num_channels = targetChannels;
    image.width = base.width;
    image.height = base.height;

    std::vector<unsigned char*> channelPointers;
    channelPointers.reserve(static_cast<size_t>(targetChannels));
    for (int c = 0; c < targetChannels; ++c) {
      channelPointers.push_back(
          reinterpret_cast<unsigned char*>(channels[static_cast<size_t>(c)].data()));
    }
    image.images = channelPointers.data();

    const char* err = nullptr;
    const int ret = SaveEXRImageToFile(&image, &header, path.c_str(), &err);
    FreeEXRHeader(&header);
    if (ret != TINYEXR_SUCCESS) {
      if (error) {
        *error = err ? err : "Failed to write EXR.";
      }
      if (err) {
        FreeEXRErrorMessage(err);
      }
      return false;
    }

    return true;
  }
};

#ifdef TEXTURE_HAS_OIIO
class OiioCodec final : public TextureCodec {
 public:
  const char* Name() const override {
    return "openimageio";
  }

  std::vector<std::string> Extensions() const override {
    return {"*"};
  }

  bool CanLoad() const override {
    return true;
  }

  bool CanSave() const override {
    return true;
  }

  bool Load(const std::string& path,
            const TextureLoadOptions& options,
            TextureResource* outTexture,
            std::string* error) const override {
    using namespace OIIO;
    auto in = ImageInput::open(path);
    if (!in) {
      if (error) {
        *error = "OpenImageIO failed to open input: " + path;
      }
      return false;
    }

    const ImageSpec spec = in->spec();
    const int width = spec.width;
    const int height = spec.height;
    const int channels = spec.nchannels;

    PixelType targetType = options.preferredType;
    TypeDesc oiioType = TypeDesc::FLOAT;
    if (targetType == PixelType::UInt8) {
      oiioType = TypeDesc::UINT8;
    } else if (targetType == PixelType::UInt16) {
      oiioType = TypeDesc::UINT16;
    }

    TextureResource texture(TextureDimension::Tex2D,
                            GuessLayout(channels),
                            channels,
                            targetType);
    texture.SetName(path);
    texture.Mips().resize(1);
    texture.Mip(0).width = width;
    texture.Mip(0).height = height;
    texture.Mip(0).depth = 1;
    texture.Mip(0).pixels.resize(
        MipByteSize(width, height, 1, channels, targetType));

    if (!in->read_image(oiioType, texture.Mip(0).pixels.data())) {
      if (error) {
        *error = "OpenImageIO failed to read image.";
      }
      in->close();
      return false;
    }
    in->close();

    std::vector<std::string> names;
    names.reserve(static_cast<size_t>(channels));
    for (const auto& name : spec.channelnames) {
      names.push_back(name);
    }
    texture.SetChannelNames(names);

    if (options.flipY) {
      texture.Mip(0).pixels =
          FlipY(texture.Mip(0), channels, texture.Type());
    }

    const int targetChannels = ResolveChannelCount(
        options.preferredChannelCount,
        options.preferredLayout,
        texture.ChannelCount());
    const ChannelLayout targetLayout = ResolveLayout(
        options.preferredLayout, targetChannels, texture.Layout());

    *outTexture =
        ConvertTexture(texture, targetChannels, options.preferredType, targetLayout);
    return true;
  }

  bool Save(const TextureResource& texture,
            const std::string& path,
            const TextureSaveOptions& options,
            std::string* error) const override {
    using namespace OIIO;
    if (texture.Mips().empty()) {
      if (error) {
        *error = "Texture has no data to save.";
      }
      return false;
    }

    int targetChannels = ResolveChannelCount(
        options.outputChannelCount,
        options.outputLayout,
        texture.ChannelCount());
    ChannelLayout targetLayout = ResolveLayout(
        options.outputLayout, targetChannels, texture.Layout());
    TextureResource converted =
        ConvertTexture(texture, targetChannels, options.outputType, targetLayout);
    TextureMipLevel base = converted.Mip(0);

    if (options.flipY) {
      base.pixels = FlipY(base, targetChannels, converted.Type());
    }

    TypeDesc oiioType = TypeDesc::FLOAT;
    if (options.outputType == PixelType::UInt8) {
      oiioType = TypeDesc::UINT8;
    } else if (options.outputType == PixelType::UInt16) {
      oiioType = TypeDesc::UINT16;
    }

    ImageSpec spec(base.width, base.height, targetChannels, oiioType);
    if (!converted.ChannelNames().empty()) {
      spec.channelnames = converted.ChannelNames();
    }

    auto out = ImageOutput::create(path);
    if (!out) {
      if (error) {
        *error = "OpenImageIO failed to create output: " + path;
      }
      return false;
    }
    if (!out->open(path, spec)) {
      if (error) {
        *error = "OpenImageIO failed to open output.";
      }
      out->close();
      return false;
    }
    if (!out->write_image(oiioType, base.pixels.data())) {
      if (error) {
        *error = "OpenImageIO failed to write image.";
      }
      out->close();
      return false;
    }
    out->close();
    return true;
  }
};
#endif

bool CodecSupportsExtension(const TextureCodec& codec, const std::string& ext) {
  const std::vector<std::string> extensions = codec.Extensions();
  for (const auto& item : extensions) {
    const std::string lower = ToLower(item);
    if (lower == "*" || lower == ext) {
      return true;
    }
  }
  return false;
}

}  // namespace

TextureResource ConvertTexture(const TextureResource& source,
                               int targetChannelCount,
                               PixelType targetType,
                               ChannelLayout targetLayout) {
  const int resolvedChannels = ResolveChannelCount(
      targetChannelCount, targetLayout, source.ChannelCount());
  const ChannelLayout resolvedLayout =
      ResolveLayout(targetLayout, resolvedChannels, source.Layout());

  TextureResource result(source.Dimension(),
                         resolvedLayout,
                         resolvedChannels,
                         targetType);
  result.SetName(source.Name());

  const bool copyNames =
      source.ChannelNames().size() == static_cast<size_t>(resolvedChannels) &&
      source.ChannelCount() == resolvedChannels &&
      source.Layout() == resolvedLayout;
  if (copyNames) {
    result.SetChannelNames(source.ChannelNames());
  } else {
    result.SetChannelNames(DefaultChannelNames(resolvedLayout, resolvedChannels));
  }

  result.Mips().resize(source.Mips().size());

  const int srcChannels = source.ChannelCount();
  const ChannelLayout srcLayout = source.Layout();

  std::vector<float> srcPixel;
  std::vector<float> dstPixel(static_cast<size_t>(resolvedChannels), 0.0f);

  for (size_t levelIndex = 0; levelIndex < source.Mips().size(); ++levelIndex) {
    const TextureMipLevel& src = source.Mips()[levelIndex];
    TextureMipLevel& dst = result.Mips()[levelIndex];
    dst.width = src.width;
    dst.height = src.height;
    dst.depth = src.depth;
    dst.pixels.resize(MipByteSize(dst.width,
                                  dst.height,
                                  dst.depth,
                                  resolvedChannels,
                                  targetType));

    const size_t pixelCount =
        static_cast<size_t>(src.width) * static_cast<size_t>(src.height) *
        static_cast<size_t>(src.depth);
    for (size_t i = 0; i < pixelCount; ++i) {
      ReadPixel(src.pixels.data(), srcChannels, source.Type(), i, &srcPixel);

      if (resolvedChannels == srcChannels) {
        dstPixel.assign(srcPixel.begin(), srcPixel.end());
      } else if (resolvedChannels == 1) {
        float value = 0.0f;
        if ((srcLayout == ChannelLayout::RGB ||
             srcLayout == ChannelLayout::RGBA) &&
            srcChannels >= 3) {
          value = LuminanceFromRGB(srcPixel[0], srcPixel[1], srcPixel[2]);
        } else if (!srcPixel.empty()) {
          value = srcPixel[0];
        }
        dstPixel.assign(1, value);
      } else {
        dstPixel.assign(static_cast<size_t>(resolvedChannels), 0.0f);
        const int copyCount = std::min(srcChannels, resolvedChannels);
        for (int c = 0; c < copyCount; ++c) {
          dstPixel[static_cast<size_t>(c)] = srcPixel[static_cast<size_t>(c)];
        }
        if (resolvedLayout == ChannelLayout::RGBA && resolvedChannels >= 4) {
          dstPixel[3] = 1.0f;
        }
      }

      WritePixel(dst.pixels.data(), resolvedChannels, targetType, i, dstPixel);
    }
  }

  return result;
}

void TextureCodecRegistry::Register(std::unique_ptr<TextureCodec> codec) {
  if (codec) {
    codecs_.push_back(std::move(codec));
  }
}

std::vector<TextureCodecInfo> TextureCodecRegistry::ListCodecs() const {
  std::vector<TextureCodecInfo> infos;
  infos.reserve(codecs_.size());
  for (const auto& codec : codecs_) {
    TextureCodecInfo info;
    info.name = codec->Name();
    info.extensions = codec->Extensions();
    info.canLoad = codec->CanLoad();
    info.canSave = codec->CanSave();
    infos.push_back(std::move(info));
  }
  return infos;
}

bool TextureCodecRegistry::Load(const std::string& path,
                                const TextureLoadOptions& options,
                                TextureResource* outTexture,
                                std::string* error) const {
  const std::string ext = GetExtension(path);
  std::string lastError;

  for (const auto& codec : codecs_) {
    if (!codec->CanLoad()) {
      continue;
    }
    if (!CodecSupportsExtension(*codec, ext)) {
      continue;
    }
    if (codec->Load(path, options, outTexture, &lastError)) {
      return true;
    }
  }

  if (error) {
    *error = lastError.empty()
                 ? "No suitable codec found to load: " + path
                 : lastError;
  }
  return false;
}

bool TextureCodecRegistry::Save(const TextureResource& texture,
                                const std::string& path,
                                const TextureSaveOptions& options,
                                std::string* error) const {
  const std::string ext = GetExtension(path);
  std::string lastError;

  for (const auto& codec : codecs_) {
    if (!codec->CanSave()) {
      continue;
    }
    if (!CodecSupportsExtension(*codec, ext)) {
      continue;
    }
    if (codec->Save(texture, path, options, &lastError)) {
      return true;
    }
  }

  if (error) {
    *error = lastError.empty()
                 ? "No suitable codec found to save: " + path
                 : lastError;
  }
  return false;
}

void MipmapGeneratorRegistry::Register(std::unique_ptr<MipmapGenerator> generator) {
  if (generator) {
    generators_.push_back(std::move(generator));
  }
}

const MipmapGenerator* MipmapGeneratorRegistry::Find(const std::string& name) const {
  const std::string lower = ToLower(name);
  for (const auto& generator : generators_) {
    if (ToLower(generator->Name()) == lower) {
      return generator.get();
    }
  }
  return nullptr;
}

std::vector<std::string> MipmapGeneratorRegistry::List() const {
  std::vector<std::string> names;
  names.reserve(generators_.size());
  for (const auto& generator : generators_) {
    names.push_back(generator->Name());
  }
  return names;
}

TextureToolkit::TextureToolkit() {
  mipmapRegistry_.Register(std::make_unique<BoxFilterMipmapGenerator>());
  mipmapRegistry_.Register(std::make_unique<NearestMipmapGenerator>());

  codecRegistry_.Register(std::make_unique<PnmCodec>());
  codecRegistry_.Register(std::make_unique<TinyExrCodec>());
  codecRegistry_.Register(std::make_unique<QoiCodec>());
  codecRegistry_.Register(std::make_unique<StbCodec>());

#ifdef TEXTURE_HAS_OIIO
  codecRegistry_.Register(std::make_unique<OiioCodec>());
#endif
}

TextureCodecRegistry& TextureToolkit::Codecs() {
  return codecRegistry_;
}

MipmapGeneratorRegistry& TextureToolkit::MipmapGenerators() {
  return mipmapRegistry_;
}

bool TextureToolkit::Load(const std::string& path,
                          const TextureLoadOptions& options,
                          TextureResource* outTexture,
                          std::string* error) const {
  if (!codecRegistry_.Load(path, options, outTexture, error)) {
    return false;
  }

  if (options.generateMipmaps) {
    if (!GenerateMipmaps(outTexture,
                         options.mipGenerator,
                         options.maxMipLevels,
                         error)) {
      return false;
    }
  }
  return true;
}

bool TextureToolkit::Save(const TextureResource& texture,
                          const std::string& path,
                          const TextureSaveOptions& options,
                          std::string* error) const {
  return codecRegistry_.Save(texture, path, options, error);
}

bool TextureToolkit::GenerateMipmaps(TextureResource* texture,
                                     const std::string& generator,
                                     int maxLevels,
                                     std::string* error) const {
  const MipmapGenerator* mipGenerator = mipmapRegistry_.Find(generator);
  if (!mipGenerator) {
    if (error) {
      *error = "Unknown mipmap generator: " + generator;
    }
    return false;
  }
  MipmapOptions options;
  options.maxLevels = maxLevels;
  return mipGenerator->Generate(texture, options, error);
}

}  // namespace texture

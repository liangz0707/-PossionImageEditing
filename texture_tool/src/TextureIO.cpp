#include "texture/TextureIO.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>

#include <stb_image.h>
#include <stb_image_write.h>
#include <tinyexr.h>

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
               PixelType type,
               ColorFormat format,
               size_t pixelIndex,
               float out[4]) {
  const int channels = ChannelCount(format);
  const size_t bytesPerChannel = BytesPerChannel(type);
  const size_t pixelStride = static_cast<size_t>(channels) * bytesPerChannel;
  const uint8_t* pixel = base + pixelIndex * pixelStride;

  float values[4] = {0.0f, 0.0f, 0.0f, 1.0f};
  for (int c = 0; c < channels; ++c) {
    values[c] = ReadChannel(pixel + static_cast<size_t>(c) * bytesPerChannel, type);
  }

  if (format == ColorFormat::R) {
    values[1] = values[0];
    values[2] = values[0];
  } else if (format == ColorFormat::RG) {
    values[2] = 0.0f;
    values[3] = 1.0f;
  } else if (format == ColorFormat::RGB) {
    values[3] = 1.0f;
  }

  out[0] = values[0];
  out[1] = values[1];
  out[2] = values[2];
  out[3] = values[3];
}

void WritePixel(uint8_t* base,
                PixelType type,
                ColorFormat format,
                size_t pixelIndex,
                const float in[4]) {
  const int channels = ChannelCount(format);
  const size_t bytesPerChannel = BytesPerChannel(type);
  const size_t pixelStride = static_cast<size_t>(channels) * bytesPerChannel;
  uint8_t* pixel = base + pixelIndex * pixelStride;

  const float r = in[0];
  const float g = in[1];
  const float b = in[2];
  const float a = in[3];

  if (format == ColorFormat::R) {
    const float luma = 0.2126f * r + 0.7152f * g + 0.0722f * b;
    WriteChannel(pixel, type, luma);
    return;
  }
  if (format == ColorFormat::RG) {
    WriteChannel(pixel, type, r);
    WriteChannel(pixel + bytesPerChannel, type, g);
    return;
  }
  if (format == ColorFormat::RGB) {
    WriteChannel(pixel, type, r);
    WriteChannel(pixel + bytesPerChannel, type, g);
    WriteChannel(pixel + bytesPerChannel * 2, type, b);
    return;
  }

  WriteChannel(pixel, type, r);
  WriteChannel(pixel + bytesPerChannel, type, g);
  WriteChannel(pixel + bytesPerChannel * 2, type, b);
  WriteChannel(pixel + bytesPerChannel * 3, type, a);
}

bool Downsample2x2(const MipLevel& src,
                   ColorFormat format,
                   PixelType type,
                   MipLevel* dst) {
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
  dst->pixels.resize(MipByteSize(dstWidth, dstHeight, 1, format, type));

  for (int y = 0; y < dstHeight; ++y) {
    for (int x = 0; x < dstWidth; ++x) {
      float accum[4] = {0.0f, 0.0f, 0.0f, 0.0f};
      int sampleCount = 0;

      for (int oy = 0; oy < 2; ++oy) {
        const int srcY = std::min(srcHeight - 1, y * 2 + oy);
        for (int ox = 0; ox < 2; ++ox) {
          const int srcX = std::min(srcWidth - 1, x * 2 + ox);
          const size_t srcIndex =
              static_cast<size_t>(srcY) * static_cast<size_t>(srcWidth) +
              static_cast<size_t>(srcX);

          float pixel[4];
          ReadPixel(src.pixels.data(), type, format, srcIndex, pixel);
          accum[0] += pixel[0];
          accum[1] += pixel[1];
          accum[2] += pixel[2];
          accum[3] += pixel[3];
          ++sampleCount;
        }
      }

      const float inv = 1.0f / static_cast<float>(sampleCount);
      accum[0] *= inv;
      accum[1] *= inv;
      accum[2] *= inv;
      accum[3] *= inv;

      const size_t dstIndex =
          static_cast<size_t>(y) * static_cast<size_t>(dstWidth) +
          static_cast<size_t>(x);
      WritePixel(dst->pixels.data(), type, format, dstIndex, accum);
    }
  }

  return true;
}

std::vector<uint8_t> FlipY(const MipLevel& level,
                           ColorFormat format,
                           PixelType type) {
  std::vector<uint8_t> flipped(level.pixels.size());
  const size_t rowBytes =
      static_cast<size_t>(level.width) * BytesPerPixel(format, type);
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

bool SavePnmTexture(const Texture& texture,
                    const SaveOptions& options,
                    const std::string& path,
                    std::string* error) {
  const std::string ext = GetExtension(path);
  const bool isPgm = ext == "pgm";
  const bool isPpm = ext == "ppm";
  const bool isPnm = ext == "pnm";
  if (!isPgm && !isPpm && !isPnm) {
    return false;
  }

  ColorFormat targetFormat = ColorFormat::RGB;
  if (isPgm) {
    targetFormat = ColorFormat::R;
  } else if (isPnm && texture.format == ColorFormat::R) {
    targetFormat = ColorFormat::R;
  }

  Texture converted = ConvertTexture(texture, targetFormat, PixelType::UInt8);
  const MipLevel& base = converted.mips[0];
  const int width = base.width;
  const int height = base.height;

  std::vector<uint8_t> staging;
  const uint8_t* data = base.pixels.data();
  if (options.flipY) {
    staging = FlipY(base, converted.format, converted.type);
    data = staging.data();
  }

  std::string header;
  if (targetFormat == ColorFormat::R) {
    header = "P5\n";
  } else {
    header = "P6\n";
  }
  header += std::to_string(width) + " " + std::to_string(height) + "\n255\n";

  std::vector<uint8_t> output;
  output.reserve(header.size() + base.pixels.size());
  output.insert(output.end(), header.begin(), header.end());
  output.insert(output.end(), data, data + base.pixels.size());

  return WriteBinaryFile(path, output.data(), output.size(), error);
}

bool LoadExrTexture(const std::string& path,
                    const LoadOptions& options,
                    Texture* outTexture,
                    std::string* error) {
  float* rgba = nullptr;
  int width = 0;
  int height = 0;
  const char* err = nullptr;
  const int ret = LoadEXR(&rgba, &width, &height, path.c_str(), &err);
  if (ret != TINYEXR_SUCCESS) {
    if (error) {
      *error = err ? err : "Failed to load EXR.";
    }
    if (err) {
      FreeEXRErrorMessage(err);
    }
    return false;
  }

  Texture base;
  base.dimension = TextureDimension::Tex2D;
  base.format = ColorFormat::RGBA;
  base.type = PixelType::Float32;
  base.mips.resize(1);
  base.mips[0].width = width;
  base.mips[0].height = height;
  base.mips[0].depth = 1;
  base.mips[0].pixels.resize(
      MipByteSize(width, height, 1, base.format, base.type));
  std::memcpy(base.mips[0].pixels.data(),
              rgba,
              base.mips[0].pixels.size());
  std::free(rgba);

  Texture result =
      ConvertTexture(base, options.preferredFormat, options.preferredType);
  if (options.flipY) {
    result.mips[0].pixels = FlipY(result.mips[0], result.format, result.type);
  }

  if (options.generateMipmaps) {
    std::string mipError;
    if (!GenerateMipmaps(&result, options.maxMipLevels, &mipError)) {
      if (error) {
        *error = mipError;
      }
      return false;
    }
  }

  *outTexture = std::move(result);
  return true;
}

bool SaveExrTexture(const Texture& texture,
                    const SaveOptions& options,
                    const std::string& path,
                    std::string* error) {
  ColorFormat targetFormat = options.outputFormat;
  if (targetFormat == ColorFormat::RG) {
    targetFormat = ColorFormat::RGB;
  }
  Texture converted = ConvertTexture(texture, targetFormat, PixelType::Float32);
  const MipLevel& base = converted.mips[0];
  const int width = base.width;
  const int height = base.height;
  const int channels = ChannelCount(converted.format);

  std::vector<uint8_t> staging;
  const uint8_t* data = base.pixels.data();
  if (options.flipY) {
    staging = FlipY(base, converted.format, converted.type);
    data = staging.data();
  }

  const float* floatData = reinterpret_cast<const float*>(data);
  const char* err = nullptr;
  const int ret =
      SaveEXR(floatData, width, height, channels, 0, path.c_str(), &err);
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

}  // namespace

bool LoadTextureFromFile(const std::string& path,
                         const LoadOptions& options,
                         Texture* outTexture,
                         std::string* error) {
  if (!outTexture) {
    if (error) {
      *error = "Output texture pointer is null.";
    }
    return false;
  }

  const std::string ext = GetExtension(path);
  if (ext == "exr") {
    return LoadExrTexture(path, options, outTexture, error);
  }

  stbi_set_flip_vertically_on_load(options.flipY ? 1 : 0);

  int width = 0;
  int height = 0;
  int channels = 0;
  const int desiredChannels = ChannelCount(options.preferredFormat);
  const bool isHdr = stbi_is_hdr(path.c_str()) != 0;

  void* data = nullptr;
  PixelType loadedType = options.preferredType;

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

  Texture texture;
  texture.dimension = TextureDimension::Tex2D;
  texture.format = options.preferredFormat;
  texture.type = loadedType;
  texture.mips.resize(1);
  texture.mips[0].width = width;
  texture.mips[0].height = height;
  texture.mips[0].depth = 1;
  texture.mips[0].pixels.resize(
      MipByteSize(width, height, 1, texture.format, texture.type));
  std::memcpy(texture.mips[0].pixels.data(),
              data,
              texture.mips[0].pixels.size());

  stbi_image_free(data);

  if (options.generateMipmaps) {
    std::string mipError;
    if (!GenerateMipmaps(&texture, options.maxMipLevels, &mipError)) {
      if (error) {
        *error = mipError;
      }
      return false;
    }
  }

  *outTexture = std::move(texture);
  return true;
}

Texture ConvertTexture(const Texture& source,
                       ColorFormat targetFormat,
                       PixelType targetType) {
  Texture result;
  result.dimension = source.dimension;
  result.format = targetFormat;
  result.type = targetType;
  result.mips.resize(source.mips.size());

  for (size_t levelIndex = 0; levelIndex < source.mips.size(); ++levelIndex) {
    const MipLevel& src = source.mips[levelIndex];
    MipLevel& dst = result.mips[levelIndex];
    dst.width = src.width;
    dst.height = src.height;
    dst.depth = src.depth;
    dst.pixels.resize(
        MipByteSize(dst.width, dst.height, dst.depth, targetFormat, targetType));

    const size_t pixelCount = static_cast<size_t>(src.width) *
                              static_cast<size_t>(src.height) *
                              static_cast<size_t>(src.depth);
    for (size_t i = 0; i < pixelCount; ++i) {
      float pixel[4];
      ReadPixel(src.pixels.data(), source.type, source.format, i, pixel);
      WritePixel(dst.pixels.data(), targetType, targetFormat, i, pixel);
    }
  }

  return result;
}

bool GenerateMipmaps(Texture* texture, int maxLevels, std::string* error) {
  if (!texture || texture->mips.empty()) {
    if (error) {
      *error = "Texture has no base mip level.";
    }
    return false;
  }
  if (texture->dimension != TextureDimension::Tex2D ||
      texture->mips[0].depth != 1) {
    if (error) {
      *error = "Only 2D textures with depth 1 are supported.";
    }
    return false;
  }

  int maxChain = 1;
  int width = texture->mips[0].width;
  int height = texture->mips[0].height;
  while (width > 1 || height > 1) {
    width = std::max(1, width / 2);
    height = std::max(1, height / 2);
    ++maxChain;
  }

  int targetLevels = maxChain;
  if (maxLevels > 0) {
    targetLevels = std::min(maxLevels, maxChain);
  }

  texture->mips.resize(1);

  for (int level = 1; level < targetLevels; ++level) {
    MipLevel next;
    if (!Downsample2x2(texture->mips[level - 1],
                       texture->format,
                       texture->type,
                       &next)) {
      if (error) {
        *error = "Failed to generate mipmap level.";
      }
      return false;
    }
    texture->mips.push_back(std::move(next));
  }

  return true;
}

bool SaveTextureToFile(const Texture& texture,
                       const std::string& path,
                       const SaveOptions& options,
                       std::string* error) {
  if (texture.mips.empty()) {
    if (error) {
      *error = "Texture has no data to save.";
    }
    return false;
  }
  if (options.writeMipChain) {
    if (error) {
      *error = "Writing full mip chains is not supported for this format.";
    }
    return false;
  }

  const std::string ext = GetExtension(path);
  if (ext == "exr") {
    return SaveExrTexture(texture, options, path, error);
  }
  if (ext == "ppm" || ext == "pgm" || ext == "pnm") {
    return SavePnmTexture(texture, options, path, error);
  }

  const bool isHdr = ext == "hdr";

  PixelType targetType = isHdr ? PixelType::Float32 : PixelType::UInt8;
  Texture converted =
      ConvertTexture(texture, options.outputFormat, targetType);

  const MipLevel& base = converted.mips[0];
  const int width = base.width;
  const int height = base.height;
  const int channels = ChannelCount(converted.format);

  std::vector<uint8_t> staging;
  const uint8_t* data = base.pixels.data();
  if (options.flipY) {
    staging = FlipY(base, converted.format, converted.type);
    data = staging.data();
  }

  int success = 0;
  if (ext == "png") {
    success = stbi_write_png(path.c_str(),
                             width,
                             height,
                             channels,
                             data,
                             width * channels);
  } else if (ext == "bmp") {
    success = stbi_write_bmp(path.c_str(),
                             width,
                             height,
                             channels,
                             data);
  } else if (ext == "tga") {
    success = stbi_write_tga(path.c_str(),
                             width,
                             height,
                             channels,
                             data);
  } else if (ext == "jpg" || ext == "jpeg") {
    success = stbi_write_jpg(path.c_str(),
                             width,
                             height,
                             channels,
                             data,
                             options.jpgQuality);
  } else if (ext == "hdr") {
    std::vector<float> floatPixels(
        static_cast<size_t>(width) * static_cast<size_t>(height) *
        static_cast<size_t>(channels));
    std::memcpy(floatPixels.data(),
                data,
                floatPixels.size() * sizeof(float));
    success = stbi_write_hdr(path.c_str(),
                             width,
                             height,
                             channels,
                             floatPixels.data());
  } else {
    if (error) {
      *error = "Unsupported output format: " + ext;
    }
    return false;
  }

  if (success == 0) {
    if (error) {
      *error = "Failed to write output file.";
    }
    return false;
  }

  return true;
}

}  // namespace texture

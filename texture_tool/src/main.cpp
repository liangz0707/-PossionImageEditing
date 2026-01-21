#include <cstdlib>
#include <iostream>
#include <string>

#include "texture/TextureIO.h"

namespace {

struct ParsedFormat {
  texture::ColorFormat format = texture::ColorFormat::RGBA;
  texture::PixelType type = texture::PixelType::UInt8;
};

std::string ToLower(std::string value) {
  for (char& c : value) {
    if (c >= 'A' && c <= 'Z') {
      c = static_cast<char>(c - 'A' + 'a');
    }
  }
  return value;
}

bool ParseFormatString(const std::string& value, ParsedFormat* out) {
  if (!out) {
    return false;
  }
  const std::string lower = ToLower(value);

  if (lower == "r8") {
    out->format = texture::ColorFormat::R;
    out->type = texture::PixelType::UInt8;
    return true;
  }
  if (lower == "rg8") {
    out->format = texture::ColorFormat::RG;
    out->type = texture::PixelType::UInt8;
    return true;
  }
  if (lower == "rgb8") {
    out->format = texture::ColorFormat::RGB;
    out->type = texture::PixelType::UInt8;
    return true;
  }
  if (lower == "rgba8") {
    out->format = texture::ColorFormat::RGBA;
    out->type = texture::PixelType::UInt8;
    return true;
  }
  if (lower == "r16") {
    out->format = texture::ColorFormat::R;
    out->type = texture::PixelType::UInt16;
    return true;
  }
  if (lower == "rg16") {
    out->format = texture::ColorFormat::RG;
    out->type = texture::PixelType::UInt16;
    return true;
  }
  if (lower == "rgb16") {
    out->format = texture::ColorFormat::RGB;
    out->type = texture::PixelType::UInt16;
    return true;
  }
  if (lower == "rgba16") {
    out->format = texture::ColorFormat::RGBA;
    out->type = texture::PixelType::UInt16;
    return true;
  }
  if (lower == "r32f") {
    out->format = texture::ColorFormat::R;
    out->type = texture::PixelType::Float32;
    return true;
  }
  if (lower == "rg32f") {
    out->format = texture::ColorFormat::RG;
    out->type = texture::PixelType::Float32;
    return true;
  }
  if (lower == "rgb32f") {
    out->format = texture::ColorFormat::RGB;
    out->type = texture::PixelType::Float32;
    return true;
  }
  if (lower == "rgba32f") {
    out->format = texture::ColorFormat::RGBA;
    out->type = texture::PixelType::Float32;
    return true;
  }

  return false;
}

void PrintUsage() {
  std::cout << "texture_tool - unified texture converter\n";
  std::cout << "\n";
  std::cout << "Usage:\n";
  std::cout << "  texture_tool info <input> [--format=rgba8]\n";
  std::cout << "  texture_tool convert <input> <output> [options]\n";
  std::cout << "\n";
  std::cout << "Options:\n";
  std::cout << "  --format=rgba8|rgb8|rg8|r8|rgba16|rgb16|rg16|r16|rgba32f|rgb32f|rg32f|r32f\n";
  std::cout << "  --mips=N (0 means full chain)\n";
  std::cout << "  --flip (flip vertically on save)\n";
}

bool ParseCommonArgs(int startIndex,
                     int argc,
                     char** argv,
                     ParsedFormat* format,
                     bool* hasFormat,
                     int* mipLevels,
                     bool* hasMips,
                     bool* flip,
                     std::string* error) {
  for (int i = startIndex; i < argc; ++i) {
    const std::string arg = argv[i];
    if (arg.rfind("--format=", 0) == 0) {
      const std::string value = arg.substr(std::string("--format=").size());
      if (!ParseFormatString(value, format)) {
        if (error) {
          *error = "Invalid format: " + value;
        }
        return false;
      }
      if (hasFormat) {
        *hasFormat = true;
      }
    } else if (arg.rfind("--mips=", 0) == 0) {
      const std::string value = arg.substr(std::string("--mips=").size());
      char* end = nullptr;
      long parsed = std::strtol(value.c_str(), &end, 10);
      if (!end || *end != '\0' || parsed < 0) {
        if (error) {
          *error = "Invalid mip level count: " + value;
        }
        return false;
      }
      if (mipLevels) {
        *mipLevels = static_cast<int>(parsed);
      }
      if (hasMips) {
        *hasMips = true;
      }
    } else if (arg == "--flip") {
      if (flip) {
        *flip = true;
      }
    } else {
      if (error) {
        *error = "Unknown option: " + arg;
      }
      return false;
    }
  }
  return true;
}

void PrintTextureInfo(const texture::Texture& texture) {
  std::cout << "dimension: " << texture::ToString(texture.dimension) << "\n";
  std::cout << "format: " << texture::ToString(texture.format) << "\n";
  std::cout << "pixel_type: " << texture::ToString(texture.type) << "\n";
  std::cout << "channels: " << texture::ChannelCount(texture.format) << "\n";
  std::cout << "width: " << texture.Width() << "\n";
  std::cout << "height: " << texture.Height() << "\n";
  std::cout << "depth: " << texture.Depth() << "\n";
  std::cout << "mip_levels: " << texture.MipCount() << "\n";
  for (size_t i = 0; i < texture.mips.size(); ++i) {
    const texture::MipLevel& mip = texture.mips[i];
    std::cout << "  mip[" << i << "]: " << mip.width << "x" << mip.height << "\n";
  }
}

}  // namespace

int main(int argc, char** argv) {
  if (argc < 2) {
    PrintUsage();
    return 1;
  }

  const std::string command = argv[1];
  if (command == "info") {
    if (argc < 3) {
      PrintUsage();
      return 1;
    }

    ParsedFormat format;
    bool hasFormat = false;
    int mipLevels = 0;
    bool hasMips = false;
    bool flip = false;
    std::string error;

    if (!ParseCommonArgs(3,
                         argc,
                         argv,
                         &format,
                         &hasFormat,
                         &mipLevels,
                         &hasMips,
                         &flip,
                         &error)) {
      std::cerr << error << "\n";
      return 1;
    }

    texture::LoadOptions loadOptions;
    if (hasFormat) {
      loadOptions.preferredFormat = format.format;
      loadOptions.preferredType = format.type;
    }
    loadOptions.flipY = false;
    texture::Texture texture;
    if (!texture::LoadTextureFromFile(argv[2], loadOptions, &texture, &error)) {
      std::cerr << "Load failed: " << error << "\n";
      return 1;
    }

    if (hasMips) {
      if (!texture::GenerateMipmaps(&texture, mipLevels, &error)) {
        std::cerr << "Mipmap generation failed: " << error << "\n";
        return 1;
      }
    }

    PrintTextureInfo(texture);
    return 0;
  }

  if (command == "convert") {
    if (argc < 4) {
      PrintUsage();
      return 1;
    }

    ParsedFormat format;
    bool hasFormat = false;
    int mipLevels = 0;
    bool hasMips = false;
    bool flip = false;
    std::string error;

    if (!ParseCommonArgs(4,
                         argc,
                         argv,
                         &format,
                         &hasFormat,
                         &mipLevels,
                         &hasMips,
                         &flip,
                         &error)) {
      std::cerr << error << "\n";
      return 1;
    }

    texture::LoadOptions loadOptions;
    if (hasFormat) {
      loadOptions.preferredFormat = format.format;
      loadOptions.preferredType = format.type;
    }

    texture::Texture texture;
    if (!texture::LoadTextureFromFile(argv[2], loadOptions, &texture, &error)) {
      std::cerr << "Load failed: " << error << "\n";
      return 1;
    }

    if (hasMips) {
      if (!texture::GenerateMipmaps(&texture, mipLevels, &error)) {
        std::cerr << "Mipmap generation failed: " << error << "\n";
        return 1;
      }
    }

    texture::SaveOptions saveOptions;
    if (hasFormat) {
      saveOptions.outputFormat = format.format;
      saveOptions.outputType = format.type;
    }
    saveOptions.flipY = flip;

    if (!texture::SaveTextureToFile(texture, argv[3], saveOptions, &error)) {
      std::cerr << "Save failed: " << error << "\n";
      return 1;
    }
    return 0;
  }

  PrintUsage();
  return 1;
}

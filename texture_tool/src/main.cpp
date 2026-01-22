#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include "texture/TextureIO.h"

namespace {

struct ParsedOptions {
  bool hasFormat = false;
  bool hasChannels = false;
  bool hasType = false;
  bool hasMips = false;
  bool flip = false;
  int channelCount = 0;
  int mipLevels = 0;
  int jpgQuality = 90;
  texture::ChannelLayout layout = texture::ChannelLayout::Unknown;
  texture::PixelType type = texture::PixelType::UInt8;
  std::string mipGenerator = "box";
  std::string compression;
};

std::string ToLower(std::string value) {
  for (char& c : value) {
    if (c >= 'A' && c <= 'Z') {
      c = static_cast<char>(c - 'A' + 'a');
    }
  }
  return value;
}

bool ParsePixelType(const std::string& value, texture::PixelType* out) {
  if (!out) {
    return false;
  }
  const std::string lower = ToLower(value);
  if (lower == "uint8" || lower == "u8") {
    *out = texture::PixelType::UInt8;
    return true;
  }
  if (lower == "uint16" || lower == "u16") {
    *out = texture::PixelType::UInt16;
    return true;
  }
  if (lower == "float32" || lower == "f32") {
    *out = texture::PixelType::Float32;
    return true;
  }
  return false;
}

bool ParseFormatString(const std::string& value,
                       texture::ChannelLayout* layout,
                       int* channels,
                       texture::PixelType* type) {
  if (!layout || !channels || !type) {
    return false;
  }
  const std::string lower = ToLower(value);

  if (lower == "r8") {
    *layout = texture::ChannelLayout::R;
    *channels = 1;
    *type = texture::PixelType::UInt8;
    return true;
  }
  if (lower == "rg8") {
    *layout = texture::ChannelLayout::RG;
    *channels = 2;
    *type = texture::PixelType::UInt8;
    return true;
  }
  if (lower == "rgb8") {
    *layout = texture::ChannelLayout::RGB;
    *channels = 3;
    *type = texture::PixelType::UInt8;
    return true;
  }
  if (lower == "rgba8") {
    *layout = texture::ChannelLayout::RGBA;
    *channels = 4;
    *type = texture::PixelType::UInt8;
    return true;
  }
  if (lower == "r16") {
    *layout = texture::ChannelLayout::R;
    *channels = 1;
    *type = texture::PixelType::UInt16;
    return true;
  }
  if (lower == "rg16") {
    *layout = texture::ChannelLayout::RG;
    *channels = 2;
    *type = texture::PixelType::UInt16;
    return true;
  }
  if (lower == "rgb16") {
    *layout = texture::ChannelLayout::RGB;
    *channels = 3;
    *type = texture::PixelType::UInt16;
    return true;
  }
  if (lower == "rgba16") {
    *layout = texture::ChannelLayout::RGBA;
    *channels = 4;
    *type = texture::PixelType::UInt16;
    return true;
  }
  if (lower == "r32f") {
    *layout = texture::ChannelLayout::R;
    *channels = 1;
    *type = texture::PixelType::Float32;
    return true;
  }
  if (lower == "rg32f") {
    *layout = texture::ChannelLayout::RG;
    *channels = 2;
    *type = texture::PixelType::Float32;
    return true;
  }
  if (lower == "rgb32f") {
    *layout = texture::ChannelLayout::RGB;
    *channels = 3;
    *type = texture::PixelType::Float32;
    return true;
  }
  if (lower == "rgba32f") {
    *layout = texture::ChannelLayout::RGBA;
    *channels = 4;
    *type = texture::PixelType::Float32;
    return true;
  }
  return false;
}

void PrintUsage() {
  std::cout << "texture_tool - unified texture toolkit\n\n";
  std::cout << "Usage:\n";
  std::cout << "  texture_tool list\n";
  std::cout << "  texture_tool info <input> [options]\n";
  std::cout << "  texture_tool convert <input> <output> [options]\n\n";
  std::cout << "Options:\n";
  std::cout << "  --format=rgba8|rgb8|rg8|r8|rgba16|rgb16|rg16|r16|rgba32f|rgb32f|rg32f|r32f\n";
  std::cout << "  --channels=N\n";
  std::cout << "  --type=uint8|uint16|float32\n";
  std::cout << "  --mips=N (0 means full chain)\n";
  std::cout << "  --mipgen=box|nearest\n";
  std::cout << "  --flip (flip vertically on save)\n";
  std::cout << "  --quality=N (JPEG quality)\n";
  std::cout << "  --compression=none|zip|zips|piz|rle (EXR)\n";
}

bool ParseCommonArgs(int startIndex,
                     int argc,
                     char** argv,
                     ParsedOptions* options,
                     std::string* error) {
  if (!options) {
    if (error) {
      *error = "Options output is null.";
    }
    return false;
  }
  for (int i = startIndex; i < argc; ++i) {
    const std::string arg = argv[i];
    if (arg.rfind("--format=", 0) == 0) {
      const std::string value = arg.substr(std::string("--format=").size());
      if (!ParseFormatString(value,
                             &options->layout,
                             &options->channelCount,
                             &options->type)) {
        if (error) {
          *error = "Invalid format: " + value;
        }
        return false;
      }
      options->hasFormat = true;
      options->hasChannels = true;
      options->hasType = true;
    } else if (arg.rfind("--channels=", 0) == 0) {
      const std::string value = arg.substr(std::string("--channels=").size());
      char* end = nullptr;
      long parsed = std::strtol(value.c_str(), &end, 10);
      if (!end || *end != '\0' || parsed <= 0) {
        if (error) {
          *error = "Invalid channel count: " + value;
        }
        return false;
      }
      options->channelCount = static_cast<int>(parsed);
      options->hasChannels = true;
    } else if (arg.rfind("--type=", 0) == 0) {
      const std::string value = arg.substr(std::string("--type=").size());
      if (!ParsePixelType(value, &options->type)) {
        if (error) {
          *error = "Invalid pixel type: " + value;
        }
        return false;
      }
      options->hasType = true;
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
      options->mipLevels = static_cast<int>(parsed);
      options->hasMips = true;
    } else if (arg.rfind("--mipgen=", 0) == 0) {
      options->mipGenerator = arg.substr(std::string("--mipgen=").size());
    } else if (arg == "--flip") {
      options->flip = true;
    } else if (arg.rfind("--quality=", 0) == 0) {
      const std::string value = arg.substr(std::string("--quality=").size());
      char* end = nullptr;
      long parsed = std::strtol(value.c_str(), &end, 10);
      if (!end || *end != '\0' || parsed < 1 || parsed > 100) {
        if (error) {
          *error = "Invalid JPEG quality: " + value;
        }
        return false;
      }
      options->jpgQuality = static_cast<int>(parsed);
    } else if (arg.rfind("--compression=", 0) == 0) {
      options->compression = arg.substr(std::string("--compression=").size());
    } else {
      if (error) {
        *error = "Unknown option: " + arg;
      }
      return false;
    }
  }
  return true;
}

void PrintTextureInfo(const texture::TextureResource& texture) {
  std::cout << "dimension: " << texture::ToString(texture.Dimension()) << "\n";
  std::cout << "layout: " << texture::ToString(texture.Layout()) << "\n";
  std::cout << "pixel_type: " << texture::ToString(texture.Type()) << "\n";
  std::cout << "channels: " << texture.ChannelCount() << "\n";
  std::cout << "width: " << texture.Width() << "\n";
  std::cout << "height: " << texture.Height() << "\n";
  std::cout << "depth: " << texture.Depth() << "\n";
  std::cout << "mip_levels: " << texture.MipCount() << "\n";
  if (!texture.ChannelNames().empty()) {
    std::cout << "channel_names: ";
    for (size_t i = 0; i < texture.ChannelNames().size(); ++i) {
      if (i > 0) {
        std::cout << ", ";
      }
      std::cout << texture.ChannelNames()[i];
    }
    std::cout << "\n";
  }
  for (size_t i = 0; i < texture.Mips().size(); ++i) {
    const texture::TextureMipLevel& mip = texture.Mips()[i];
    std::cout << "  mip[" << i << "]: " << mip.width << "x" << mip.height << "\n";
  }
}

}  // namespace

int main(int argc, char** argv) {
  if (argc < 2) {
    PrintUsage();
    return 1;
  }

  texture::TextureToolkit toolkit;

  const std::string command = argv[1];
  if (command == "list") {
    const auto codecs = toolkit.Codecs().ListCodecs();
    std::cout << "Codecs:\n";
    for (const auto& codec : codecs) {
      std::cout << "  " << codec.name << " (";
      std::cout << (codec.canLoad ? "read" : "") <<
                   ((codec.canLoad && codec.canSave) ? "/" : "") <<
                   (codec.canSave ? "write" : "") << "): ";
      for (size_t i = 0; i < codec.extensions.size(); ++i) {
        if (i > 0) {
          std::cout << ", ";
        }
        std::cout << codec.extensions[i];
      }
      std::cout << "\n";
    }

    const auto generators = toolkit.MipmapGenerators().List();
    std::cout << "Mipmap generators:\n";
    for (const auto& name : generators) {
      std::cout << "  " << name << "\n";
    }
    return 0;
  }

  if (command == "info") {
    if (argc < 3) {
      PrintUsage();
      return 1;
    }

    ParsedOptions options;
    std::string error;
    if (!ParseCommonArgs(3, argc, argv, &options, &error)) {
      std::cerr << error << "\n";
      return 1;
    }

    texture::TextureLoadOptions loadOptions;
    if (options.hasFormat || options.hasChannels) {
      loadOptions.preferredChannelCount = options.channelCount;
      loadOptions.preferredLayout = options.layout;
    } else {
      loadOptions.preferredChannelCount = 0;
      loadOptions.preferredLayout = texture::ChannelLayout::Unknown;
    }
    if (options.hasType) {
      loadOptions.preferredType = options.type;
    }
    loadOptions.generateMipmaps = options.hasMips;
    loadOptions.maxMipLevels = options.mipLevels;
    loadOptions.mipGenerator = options.mipGenerator;

    texture::TextureResource texture;
    if (!toolkit.Load(argv[2], loadOptions, &texture, &error)) {
      std::cerr << "Load failed: " << error << "\n";
      return 1;
    }

    PrintTextureInfo(texture);
    return 0;
  }

  if (command == "convert") {
    if (argc < 4) {
      PrintUsage();
      return 1;
    }

    ParsedOptions options;
    std::string error;
    if (!ParseCommonArgs(4, argc, argv, &options, &error)) {
      std::cerr << error << "\n";
      return 1;
    }

    texture::TextureLoadOptions loadOptions;
    if (options.hasFormat || options.hasChannels) {
      loadOptions.preferredChannelCount = options.channelCount;
      loadOptions.preferredLayout = options.layout;
    } else {
      loadOptions.preferredChannelCount = 0;
      loadOptions.preferredLayout = texture::ChannelLayout::Unknown;
    }
    if (options.hasType) {
      loadOptions.preferredType = options.type;
    }
    loadOptions.generateMipmaps = options.hasMips;
    loadOptions.maxMipLevels = options.mipLevels;
    loadOptions.mipGenerator = options.mipGenerator;

    texture::TextureResource texture;
    if (!toolkit.Load(argv[2], loadOptions, &texture, &error)) {
      std::cerr << "Load failed: " << error << "\n";
      return 1;
    }

    texture::TextureSaveOptions saveOptions;
    if (options.hasFormat || options.hasChannels) {
      saveOptions.outputChannelCount = options.channelCount;
      saveOptions.outputLayout = options.layout;
    }
    if (options.hasType) {
      saveOptions.outputType = options.type;
    }
    saveOptions.flipY = options.flip;
    saveOptions.jpgQuality = options.jpgQuality;
    saveOptions.compression = options.compression;

    if (!toolkit.Save(texture, argv[3], saveOptions, &error)) {
      std::cerr << "Save failed: " << error << "\n";
      return 1;
    }
    return 0;
  }

  PrintUsage();
  return 1;
}

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <queue>
#include <set>

#include <Poco/UUID.h>
#include <Poco/UUIDGenerator.h>

#include <cmrc/cmrc.hpp>

CMRC_DECLARE(assets);

namespace engine::utility
{
	template<typename Type>
	using Shared = std::shared_ptr<Type>;

	template<typename Type>
	using Weak = std::weak_ptr<Type>;

	/// <summary>
	/// Basically an alias for std::dynamic_pointer_cast
	/// </summary>
	template<typename Type1, typename Type2>
	Type1 DynamicCast(Type2 object)
	{
		return std::dynamic_pointer_cast<Type1, Type2>(object);
	}

	template<typename Type1, typename Type2>
	Shared<Type1> SharedDynamicCast(Shared<Type2> object)
	{
		return std::dynamic_pointer_cast<Type1, Type2>(object);

	}

	template<typename Type1, typename Type2>
	Weak<Type1> WeakDynamicCast(Weak<Type2> object)
	{
		return std::dynamic_pointer_cast<Type1, Type2>(object);

	}

	/// <summary>
	/// Class that could be derived to add UUID functionality
	/// </summary>
	class UUIDable
	{
		static Poco::UUIDGenerator uuid_generator;

	public:
		const Poco::UUID uuid;

		UUIDable() : uuid(uuid_generator.createRandom()) {}

		UUIDable(const char* uuid) : uuid(uuid) {}
		UUIDable(const std::string& uuid) : uuid(uuid) {}

		std::string getUUID()
		{
			return uuid.toString();
		}
	};

	/// <summary>
	/// Structure that holds RGB color
	/// </summary>
	/// <typeparam name="Type">Type for values</typeparam>
	template<typename Type = uint8_t>
	struct RGBColor
	{
		Type R, G, B;

		RGBColor(Type brightness = 0) : R(brightness), G(brightness), B(brightness) {}
		RGBColor(Type red, Type green, Type blue) : R(red), G(green), B(blue) {}
	};

	/// <summary>
	/// Structure that holds one alpha value
	/// </summary>
	/// <typeparam name="Type">Type for alpha value</typeparam>
	template<typename Type = uint8_t>
	struct AlphaChannel
	{
		Type A;

		AlphaChannel(Type opacity = 0) : A(opacity) {}
	};

	/// <summary>
	/// Structure that holds both RGB color and alpha channel for it
	/// </summary>
	/// <typeparam name="Type">Type for both RGB color struct and alpha channel value</typeparam>
	template<typename Type = uint8_t>
	struct RGBAColor : RGBColor<Type>, AlphaChannel<Type>
	{
		RGBAColor(Type brightness = 0, Type opacity = 0) : RGBColor(brightness), AlphaChannel(opacity) {}
		RGBAColor(Type R, Type G, Type B, Type A = std::numeric_limits<Type>::max()) : RGBColor(R, G, B), AlphaChannel(A) {}
	};

	/// <summary>
	/// Struct containing rectangle of width x height pixels
	/// </summary>
	/// <typeparam name="PixelType">Pixel format type</typeparam>
	template<typename PixelType = RGBAColor<uint8_t>>
	struct Frame
	{
		std::shared_ptr<PixelType[]> pixels;

		const uint16_t width;
		const uint16_t height;

		Frame(uint16_t width, uint16_t height) : width(width), height(height)
		{
			pixels = std::make_shared<PixelType[]>(width * height);
		}

		PixelType getPixel(uint16_t x, uint16_t y)
		{
			return pixels[y * width + x];
		}

		void setPixel(uint16_t x, uint16_t y, PixelType color)
		{
			pixels[y * width + x] = color;
		}
	};

	typedef float_t AudioSample;

	typedef std::vector<AudioSample> AudioSampleVector;

	/// <summary>
	/// Slightly improved CMRC filesystem class
	/// </summary>
	class CMRCFilesystem : protected cmrc::embedded_filesystem
	{
	public:
		// Average usage: CMRCFilesystem cool_object(cmrc::the_thing::get_filesystem());
		CMRCFilesystem(cmrc::embedded_filesystem filesystem) : cmrc::embedded_filesystem(filesystem) {}

		bool exists(std::string path)
		{
			return cmrc::embedded_filesystem::exists(path);
		}

		bool isFile(std::string path)
		{
			return cmrc::embedded_filesystem::is_file(path);
		}

		bool isDirectory(std::string path)
		{
			return cmrc::embedded_filesystem::is_directory(path);
		}

		std::vector<char> loadFile(std::string path)
		{
			if (!exists(path))
				throw std::exception("File doesn't exist.");

			auto file = cmrc::embedded_filesystem::open(path);

			return std::vector<char>(file.begin(), file.end());
		}

		std::pair<std::set<std::string>, std::set<std::string>> getFilesAndDirs(std::string path = "")
		{
			auto dir = cmrc::embedded_filesystem::iterate_directory(path);
			auto end = dir.end();

			std::set<std::string> filenames;
			std::set<std::string> dirnames;

			for (auto iter = dir.begin(); iter != end; iter++)
			{
				auto element = *iter;
				auto element_filename = element.filename();

				if (element.is_file())
					filenames.insert(element_filename);
				else
					dirnames.insert(element_filename);
			}

			return std::make_pair(filenames, dirnames);
		}

		std::set<std::string> getFilenames(std::string path = "")
		{
			return getFilesAndDirs(path).first;
		}

		std::set<std::string> getDirectories(std::string path = "")
		{
			return getFilesAndDirs(path).second;
		}
	};
}
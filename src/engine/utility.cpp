#include "utility.hpp"

namespace engine::utility
{
	Poco::UUIDGenerator UUIDable::uuid_generator;

	UUIDable::UUIDable() : uuid(uuid_generator.createRandom()) {}

	UUIDable::UUIDable(const char* uuid) : uuid(uuid) {}
	UUIDable::UUIDable(const std::string& uuid) : uuid(uuid) {}

	std::string UUIDable::getUUID()
	{
		return uuid.toString();
	}

	CMRCFilesystem::CMRCFilesystem(cmrc::embedded_filesystem filesystem) : cmrc::embedded_filesystem(filesystem) {}

	bool CMRCFilesystem::exists(std::string path)
	{
		return cmrc::embedded_filesystem::exists(path);
	}

	bool CMRCFilesystem::isFile(std::string path)
	{
		return cmrc::embedded_filesystem::is_file(path);
	}

	bool CMRCFilesystem::isDirectory(std::string path)
	{
		return cmrc::embedded_filesystem::is_directory(path);
	}

	std::vector<char> CMRCFilesystem::loadFile(std::string path)
	{
		if (!exists(path))
			throw std::exception("File doesn't exist.");

		auto file = cmrc::embedded_filesystem::open(path);

		return std::vector<char>(file.begin(), file.end());
	}

	std::pair<std::set<std::string>, std::set<std::string>> CMRCFilesystem::getFilesAndDirs(std::string path)
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

	std::set<std::string> CMRCFilesystem::getFilenames(std::string path)
	{
		return getFilesAndDirs(path).first;
	}

	std::set<std::string> CMRCFilesystem::getDirectories(std::string path)
	{
		return getFilesAndDirs(path).second;
	}
}
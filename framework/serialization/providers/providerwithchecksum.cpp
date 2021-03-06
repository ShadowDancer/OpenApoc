#include "framework/serialization/providers/providerwithchecksum.h"
#include "framework/logger.h"
#include "library/strings.h"
#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

#include "dependencies/pugixml/src/pugixml.hpp"

#include <boost/uuid/sha1.hpp>

namespace OpenApoc
{
static UString calculate_checksum(const std::string &str)
{
	UString hashString;

	boost::uuids::detail::sha1 sha;
	sha.process_bytes(str.c_str(), str.size());
	unsigned int hash[5];
	sha.get_digest(hash);
	for (int i = 0; i < 5; i++)
	{
		unsigned int v = hash[i];
		for (int j = 0; j < 4; j++)
		{
			// FIXME: Probably need to do the reverse for big endian?
			unsigned int byteHex = v & 0xff000000;
			byteHex >>= 24;
			hashString += UString::format("%02x", byteHex).str();
			v <<= 8;
		}
	}

	return hashString;
}
std::string ProviderWithChecksum::serializeManifest()
{
	pugi::xml_document manifestDoc;
	auto decl = manifestDoc.prepend_child(pugi::node_declaration);
	decl.append_attribute("version") = "1.0";
	decl.append_attribute("encoding") = "UTF-8";
	auto root = manifestDoc.root().append_child();
	root.set_name("checksums");
	for (auto &p : contents)
	{
		auto node = root.append_child();
		node.set_name("checksum");
		node.text().set(p.first.cStr());
		node.append_attribute("SHA1") = p.second.c_str();
	}
	std::stringstream ss;
	manifestDoc.save(ss, "  ");
	return ss.str();
}

bool ProviderWithChecksum::parseManifest(const std::string &manifestData)
{
	std::stringstream ss(manifestData);
	pugi::xml_document manifestDoc;
	auto parse_result = manifestDoc.load(ss);
	if (!parse_result)
	{
		LogWarning("Failed to parse checksum.xml : \"%s\" at \"%llu\"", parse_result.description(),
		           (unsigned long long)parse_result.offset);
		return false;
	}
	auto rootNode = manifestDoc.child("checksums");
	if (!rootNode)
	{
		LogWarning("checksum.xml has invalid root node");
		return false;
	}
	auto checksumNode = rootNode.child("checksum");
	while (checksumNode)
	{
		UString fileName = checksumNode.text().get();
		auto shaAttribute = checksumNode.attribute("SHA1");
		if (!shaAttribute)
		{
			LogWarning("checksum.xml checksum node \"%s\" has no SHA1 attribute", fileName.cStr());
			contents[fileName] = "";
		}
		else
		{
			contents[fileName] = shaAttribute.value();
		}
		LogInfo("Manifest \"%s\" sha1=\"%s\"", fileName.cStr(), shaAttribute.value());
		checksumNode = checksumNode.next_sibling("checksum");
	}

	return true;
}
bool ProviderWithChecksum::openArchive(const UString &path, bool write)
{

	if (!inner->openArchive(path, write))
	{
		return false;
	}

	UString result;
	if (!inner->readDocument("checksum.xml", result))
	{
		LogInfo("Missing manifest file in \"%s\"", path.cStr());
		return true;
	}
	parseManifest(result.str());
	return true;
}
bool ProviderWithChecksum::readDocument(const UString &path, UString &result)
{
	if (inner->readDocument(path, result))
	{
		auto sha1Sum = calculate_checksum(result.str());
		auto expectedSha1Sum = contents[path.str()];
		if (expectedSha1Sum != "")
		{
			if (sha1Sum != expectedSha1Sum)
			{
				LogWarning("File \"%s\" has incorrect checksum \"%s\", expected \"%s\"",
				           path.cStr(), sha1Sum.cStr(), expectedSha1Sum.c_str());
			}
		}
		else
		{
			LogInfo("Skipping missing checksum for file \"%s\"", path.cStr());
		}
		return true;
	}

	return false;
}
bool ProviderWithChecksum::saveDocument(const UString &path, UString contents)
{
	if (inner->saveDocument(path, contents))
	{
		this->contents[path.str()] = calculate_checksum(contents.str()).str();
		return true;
	}
	return false;
}
bool ProviderWithChecksum::finalizeSave()
{
	UString manifest = serializeManifest();
	inner->saveDocument("checksum.xml", manifest);
	return inner->finalizeSave();
}
}

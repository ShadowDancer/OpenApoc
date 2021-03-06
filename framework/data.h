#pragma once
#include "library/sp.h"

#include "framework/fs.h"
#include "framework/image.h"
#include "framework/sound.h"

#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <vector>

namespace OpenApoc
{
class ImageLoader;
class SampleLoader;
class MusicLoader;
class ImageWriter;
class VoxelSlice;
class LOFTemps;
class ResourceAliases;
class Video;

class ImageLoaderFactory;
class ImageWriterFactory;
class SampleLoaderFactory;
class MusicLoaderFactory;

class Data
{

  private:
	std::map<UString, std::weak_ptr<Image>> imageCache;
	std::map<UString, std::weak_ptr<Image>> imageCacheLazy;
	std::recursive_mutex imageCacheLock;
	std::map<UString, std::weak_ptr<ImageSet>> imageSetCache;
	std::recursive_mutex imageSetCacheLock;

	std::map<UString, std::weak_ptr<Sample>> sampleCache;
	std::recursive_mutex sampleCacheLock;
	std::map<UString, std::weak_ptr<MusicTrack>> musicCache;
	std::recursive_mutex musicCacheLock;
	std::map<UString, std::weak_ptr<LOFTemps>> LOFVoxelCache;
	std::recursive_mutex voxelCacheLock;

	std::map<UString, std::weak_ptr<Palette>> paletteCache;
	std::recursive_mutex paletteCacheLock;

	// The cache is organised in <font name , <text, image>>
	std::map<UString, std::map<UString, std::weak_ptr<PaletteImage>>> fontStringCache;
	std::recursive_mutex fontStringCacheLock;

	// Pin open 'imageCacheSize' images
	std::queue<sp<Image>> pinnedImages;
	// Pin open 'imageSetCacheSize' image sets
	std::queue<sp<ImageSet>> pinnedImageSets;
	std::queue<sp<LOFTemps>> pinnedLOFVoxels;
	std::queue<sp<PaletteImage>> pinnedFontStrings;
	std::queue<sp<Palette>> pinnedPalettes;
	std::list<std::unique_ptr<ImageLoader>> imageLoaders;
	std::list<std::unique_ptr<SampleLoader>> sampleLoaders;
	std::list<std::unique_ptr<MusicLoader>> musicLoaders;
	std::list<std::unique_ptr<ImageWriter>> imageWriters;

	std::map<UString, std::unique_ptr<ImageLoaderFactory>> registeredImageBackends;
	std::map<UString, std::unique_ptr<ImageWriterFactory>> registeredImageWriters;
	std::map<UString, std::unique_ptr<SampleLoaderFactory>> registeredSampleLoaders;
	std::map<UString, std::unique_ptr<MusicLoaderFactory>> registeredMusicLoaders;

  public:
	std::weak_ptr<ResourceAliases> aliases;
	FileSystem fs;

	Data(std::vector<UString> paths, int imageCacheSize = 100, int imageSetCacheSize = 10,
	     int voxelCacheSize = 1, int fontStringCacheSize = 100, int paletteCacheSize = 10);
	~Data();

	sp<Sample> loadSample(UString path);
	sp<MusicTrack> loadMusic(const UString &path);
	sp<Image> loadImage(const UString &path, bool lazy = false);
	sp<ImageSet> loadImageSet(const UString &path);
	sp<Palette> loadPalette(const UString &path);
	sp<VoxelSlice> loadVoxelSlice(const UString &path);
	sp<Video> loadVideo(const UString &path);

	sp<PaletteImage> getFontStringCacheEntry(const UString &font_name, const UString &text);
	void putFontStringCacheEntry(const UString &font_name, const UString &text,
	                             sp<PaletteImage> &img);

	bool writeImage(UString systemPath, sp<Image> image, sp<Palette> palette = nullptr);
};

} // namespace OpenApoc

#pragma once
#include "framework/fs.h"
#include "image.h"
#include "library/sp.h"
#include "library/strings.h"

namespace OpenApoc
{

class ImageLoader
{
  public:
	virtual ~ImageLoader() = default;
	virtual sp<Image> loadImage(IFile &file) = 0;
	virtual UString getName() = 0;
};

class ImageLoaderFactory
{
  public:
	virtual ImageLoader *create() = 0;
	virtual ~ImageLoaderFactory() = default;
};

ImageLoaderFactory *getLodePNGImageLoaderFactory();
ImageLoaderFactory *getPCXImageLoaderFactory();

class ImageWriter
{
  public:
	virtual ~ImageWriter() = default;
	virtual bool writeImage(sp<RGBImage> image, std::ostream &stream) = 0;
	virtual bool writeImage(sp<PaletteImage> image, std::ostream &stream,
	                        sp<Palette> defaultPalette = nullptr) = 0;
	virtual UString getName() = 0;
};

class ImageWriterFactory
{
  public:
	virtual ImageWriter *create() = 0;
	virtual ~ImageWriterFactory() = default;
};

ImageWriterFactory *getLodePNGImageWriterFactory();
};

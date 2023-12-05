/*
   CGUITTFont FreeType class for Irrlicht
   Copyright (c) 2009-2010 John Norman

   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any
   damages arising from the use of this software.

   Permission is granted to anyone to use this software for any
   purpose, including commercial applications, and to alter it and
   redistribute it freely, subject to the following restrictions:

   1. The origin of this software must not be misrepresented; you
      must not claim that you wrote the original software. If you use
      this software in a product, an acknowledgment in the product
      documentation would be appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and
      must not be misrepresented as being the original software.

   3. This notice may not be removed or altered from any source
      distribution.

   The original version of this class can be located at:
   http://irrlicht.suckerfreegames.com/

   John Norman
   john@suckerfreegames.com
*/

#include "CGUITTFont.h"

#ifdef HC1_HAS_FREETYPE

#include "globals.h"

namespace irr
{
namespace gui
{

// Manages the FT_Face cache.
struct SGUITTFace : public virtual irr::IReferenceCounted
{
	SGUITTFace() : face_buffer(0), face_buffer_size(0)
	{
		memset((void*)&face, 0, sizeof(FT_Face));
	}

	~SGUITTFace()
	{
		FT_Done_Face(face);
		delete[] face_buffer;
	}

	FT_Face face;
	FT_Byte* face_buffer;
	FT_Long face_buffer_size;
};

// Static variables.
FT_Library CGUITTFont::c_library;
core::map<io::path, SGUITTFace*> CGUITTFont::c_faces;
bool CGUITTFont::c_libraryLoaded = false;

//

video::IImage* SGUITTGlyph::createGlyphImage(const FT_Bitmap& bits, video::IVideoDriver* driver) const
{
	// Determine what our texture size should be.
	// Add 1 because textures are inclusive-exclusive.
	core::dimension2du d(bits.width + 1, bits.rows + 1);
	core::dimension2du texture_size;
	//core::dimension2du texture_size(bits.width + 1, bits.rows + 1);

	// Create and load our image now.
	video::IImage* image = 0;
	switch (bits.pixel_mode)
	{
		case FT_PIXEL_MODE_MONO:
		{
			// Create a blank image and fill it with transparent pixels.
			texture_size = d.getOptimalSize(true, true);
			image = driver->createImage(video::ECF_A1R5G5B5, texture_size);
			image->fill(video::SColor(0, 255, 255, 255));

			// Load the monochrome data in.
			const u32 image_pitch = image->getPitch() / sizeof(u16);
			u16* image_data = (u16*)image->getData();
			u8* glyph_data = bits.buffer;
			for (unsigned int y = 0; y < bits.rows; ++y)
			{
				u16* row = image_data;
				for (unsigned int x = 0; x < bits.width; ++x)
				{
					// Monochrome bitmaps store 8 pixels per byte.  The left-most pixel is the bit 0x80.
					// So, we go through the data each bit at a time.
					if ((glyph_data[y * bits.pitch + (x / 8)] & (0x80 >> (x % 8))) != 0)
						*row = 0xFFFF;
					++row;
				}
				image_data += image_pitch;
			}
			break;
		}

		case FT_PIXEL_MODE_GRAY:
		{
			// Create our blank image.
			texture_size = d.getOptimalSize(!driver->queryFeature(video::EVDF_TEXTURE_NPOT), !driver->queryFeature(video::EVDF_TEXTURE_NSQUARE), true, 0);
			image = driver->createImage(video::ECF_A8R8G8B8, texture_size);
			image->fill(video::SColor(0, 255, 255, 255));

			// Load the grayscale data in.
			const float gray_count = static_cast<float>(bits.num_grays);
			const u32 image_pitch = image->getPitch() / sizeof(u32);
			u32* image_data = (u32*)image->getData();
			u8* glyph_data = bits.buffer;
			for (unsigned int y = 0; y < bits.rows; ++y)
			{
				u8* row = glyph_data;
				for (unsigned int  x = 0; x < bits.width; ++x)
				{
					image_data[y * image_pitch + x] |= static_cast<u32>(255.0f * (static_cast<float>(*row++) / gray_count)) << 24;
					//data[y * image_pitch + x] |= ((u32)(*bitsdata++) << 24);
				}
				glyph_data += bits.pitch;
			}
			break;
		}
		default:
			// TODO: error message?
			return 0;
	}
	return image;
}

void SGUITTGlyph::preload(u32 char_index, FT_Face face, video::IVideoDriver* driver, u32 font_size, const FT_Int32 loadFlags)
{
	if (isLoaded) return;

	// Set the size of the glyph.
	FT_Set_Pixel_Sizes(face, 0, font_size);

	// Attempt to load the glyph.
	if (FT_Load_Glyph(face, char_index, loadFlags) != FT_Err_Ok)
	{
		// TODO: error message?
		return;
	}

	FT_GlyphSlot glyph = face->glyph;
	FT_Bitmap bits = glyph->bitmap;

	// Setup the glyph information here:
	advance = glyph->advance;
	offset = core::vector2di(glyph->bitmap_left, glyph->bitmap_top);

	// Try to get the last page with available slots.
	CGUITTGlyphPage* page = parent->getLastGlyphPage();

	// If we need to make a new page, do that now.
	if (!page)
	{
		page = parent->createGlyphPage(bits.pixel_mode);
		if (!page)
		{
			// TODO: add error message?
			return;
		}
	}

	glyph_page = parent->getLastGlyphPageIndex();
	u32 texture_side_length = page->texture->getOriginalSize().Width;
	core::vector2di page_position(
		(page->used_slots % (texture_side_length / font_size)) * font_size,
		(page->used_slots / (texture_side_length / font_size)) * font_size
		);
	source_rect.UpperLeftCorner = page_position;
	source_rect.LowerRightCorner = core::vector2di(page_position.X + bits.width, page_position.Y + bits.rows);

	++page->used_slots;
	--page->available_slots;

	// We grab the glyph bitmap here so the data won't be removed when the next glyph is loaded.
	surface = createGlyphImage(bits, driver);

	// Set our glyph as loaded.
	isLoaded = true;
}

void SGUITTGlyph::unload()
{
	if (surface)
	{
		surface->drop();
		surface = 0;
	}
	isLoaded = false;
}

// --------------------------------------------------------
CGUITTGlyphPage::CGUITTGlyphPage(video::IVideoDriver* Driver, const io::path& texture_name)
: texture(0), available_slots(0), used_slots(0), driver(Driver), name(texture_name)
{
}

CGUITTGlyphPage::~CGUITTGlyphPage()
{
	if (texture)
	{
		if (driver)
			driver->removeTexture(texture);
		else texture->drop();
	}
}

bool CGUITTGlyphPage::createPageTexture(const u8& pixel_mode, const core::dimension2du& texture_size)
{
	if( texture )
		return false;

	bool flgmip = driver->getTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS);
	driver->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, false);

#if defined(_IRR_COMPILE_WITH_OGLES2_)
	bool keepImage = driver->getTextureCreationFlag ( video::ETCF_ALLOW_MEMORY_COPY );
	driver->setTextureCreationFlag(video::ETCF_ALLOW_MEMORY_COPY, true );
#endif

	// Set the texture color format.
	switch (pixel_mode)
	{
		case FT_PIXEL_MODE_MONO:
			texture = driver->addTexture(texture_size, name, video::ECF_A1R5G5B5);
			break;
		case FT_PIXEL_MODE_GRAY:
		default:
			texture = driver->addTexture(texture_size, name, video::ECF_A8R8G8B8);
			break;
	}

	// Restore our texture creation flags.
	driver->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, flgmip);
#if defined(_IRR_COMPILE_WITH_OGLES2_)
	driver->setTextureCreationFlag(video::ETCF_ALLOW_MEMORY_COPY, keepImage);
#endif

	return texture ? true : false;
}

//! Add the glyph to a list of glyphs to be paged.
//! This collection will be cleared after updateTexture is called.
void CGUITTGlyphPage::pushGlyphToBePaged(const SGUITTGlyph* glyph)
{
	glyph_to_be_paged.push_back(glyph);
}

//! Updates the texture atlas with new glyphs.
void CGUITTGlyphPage::updateTexture()
{
	if (glyph_to_be_paged.empty())
		return;

	void* ptr = texture->lock();
	video::ECOLOR_FORMAT format = texture->getColorFormat();
	core::dimension2du size = texture->getSize();

	video::IImage* pageholder = driver->createImageFromData(format, size, ptr, true, false);

	for (u32 i = 0; i < glyph_to_be_paged.size(); ++i)
	{
		const SGUITTGlyph* glyph = glyph_to_be_paged[i];
		if (glyph && glyph->isLoaded)
		{
			if (glyph->surface)
			{
				glyph->surface->copyTo(pageholder, glyph->source_rect.UpperLeftCorner);
				glyph->surface->drop();
				glyph->surface = 0;
			}
			else
			{
				; // TODO: add error message?
				//currently, if we failed to create the image, just ignore this operation.
			}
		}
	}

	texture->unlock();
	pageholder->drop();
	glyph_to_be_paged.clear();
}


// --------------------------------------------------------
CGUITTFont* CGUITTFont::createTTFont(irr::video::IVideoDriver* driver, irr::io::IFileSystem* fileSystem, const io::path& filename, u32 size, bool antialias, bool transparency, const wchar_t *invisibleChars, irr::ILogger* logger)
{
	if ( !driver )
		return 0;

	if (!c_libraryLoaded)
	{
		if (FT_Init_FreeType(&c_library))
			return 0;
		c_libraryLoaded = true;
	}

	CGUITTFont* font = new CGUITTFont(driver, fileSystem);
	font->Logger = logger;
	bool ret = font->load(filename, size, antialias, transparency);
	if (!ret)
	{
		font->drop();
		return 0;
	}

	font->setInvisibleCharacters(invisibleChars);

	return font;
}

//! Constructor.
CGUITTFont::CGUITTFont(irr::video::IVideoDriver* driver, irr::io::IFileSystem* fileSystem)
: use_monochrome(false), use_transparency(true), use_hinting(false), use_auto_hinting(false),
batch_load_size(1), Driver(driver), FileSystem(fileSystem), Logger(0), GlobalKerningWidth(0), GlobalKerningHeight(0)
{
	#ifdef _DEBUG
	setDebugName("CGUITTFont");
	#endif
}

bool CGUITTFont::load(const io::path& filename, u32 size, bool antialias, bool transparency)
{
	// Some sanity checks.
	if (Driver == 0) return false;
	if (size == 0) return false;
	if (filename.size() == 0) return false;

	this->size = size;
	this->filename = filename;

	// Update the font loading flags when the font is first loaded.
	this->use_monochrome = !antialias;
	this->use_transparency = transparency;
	update_load_flags();

	// Log.
	if (Logger)
		Logger->log(L"CGUITTFont", core::stringw(core::stringw(L"Creating new font: ") + core::stringw(filename) + L" " + core::stringc(size) + L"pt " + (antialias ? L"+antialias " : L"-antialias ") + (transparency ? L"+transparency" : L"-transparency")).c_str(), irr::ELL_INFORMATION);

	// Grab the face.
	SGUITTFace* face = 0;
	core::map<io::path, SGUITTFace*>::Node* node = c_faces.find(filename);
	if (node == 0)
	{
		face = new SGUITTFace();
		c_faces.set(filename, face);

		if (FileSystem)
		{
			// Read in the file data.
			io::IReadFile* file = FileSystem->createAndOpenFile(filename);
			if (file == 0)
			{
				if (Logger) Logger->log(L"CGUITTFont", L"Failed to open the file.", irr::ELL_INFORMATION);

				c_faces.remove(filename);
				delete face;
				face = 0;
				return false;
			}
			face->face_buffer = new FT_Byte[file->getSize()];
			file->read(face->face_buffer, file->getSize());
			face->face_buffer_size = file->getSize();
			file->drop();

			// Create the face.
			if (FT_New_Memory_Face(c_library, face->face_buffer, face->face_buffer_size, 0, &face->face))
			{
				if (Logger) Logger->log(L"CGUITTFont", L"FT_New_Memory_Face failed.", irr::ELL_INFORMATION);

				c_faces.remove(filename);
				delete face;
				face = 0;
				return false;
			}
		}
		else
		{
			core::stringc converter(filename);
			if (FT_New_Face(c_library, converter.c_str(), 0, &face->face))
			{
				if (Logger) Logger->log(L"CGUITTFont", L"FT_New_Face failed.", irr::ELL_INFORMATION);

				c_faces.remove(filename);
				delete face;
				face = 0;
				return false;
			}
		}
	}
	else
	{
		// Using another instance of this face.
		face = node->getValue();
		face->grab();
	}

	// Store our face.
	tt_face = face->face;

	// Store font metrics.
	FT_Set_Pixel_Sizes(tt_face, size, 0);
	font_metrics = tt_face->size->metrics;

	// Allocate our glyphs.
	Glyphs.clear();
	Glyphs.set_used(tt_face->num_glyphs);
	for (FT_Long i = 0; i < tt_face->num_glyphs; ++i)
	{
		Glyphs[i].isLoaded = false;
		Glyphs[i].glyph_page = 0;
		Glyphs[i].source_rect = core::recti();
		Glyphs[i].offset = core::vector2di();
		Glyphs[i].advance = FT_Vector();
		Glyphs[i].surface = 0;
		Glyphs[i].parent = this;
	}

	// Cache the first 127 ascii characters.
	u32 old_size = batch_load_size;
	batch_load_size = 127;
	getGlyphIndexByChar((wchar_t)0);
	batch_load_size = old_size;

	return true;
}

CGUITTFont::~CGUITTFont()
{
	// Delete the glyphs and glyph pages.
	reset_images();

	// We aren't using this face anymore.
	core::map<io::path, SGUITTFace*>::Node* n = c_faces.find(filename);
	if (n)
	{
		SGUITTFace* f = n->getValue();

		// Drop our face.  If this was the last face, the destructor will clean up.
		if (f->drop())
			c_faces.remove(filename);

		// If there are no more faces referenced by FreeType, clean up.
		if (c_faces.size() == 0)
		{
			FT_Done_FreeType(c_library);
			c_libraryLoaded = false;
		}
	}
}

void CGUITTFont::reset_images()
{
	// Delete the glyphs.
	for (u32 i = 0; i != Glyphs.size(); ++i)
		Glyphs[i].unload();

	// Unload the glyph pages from video memory.
	for (u32 i = 0; i != Glyph_Pages.size(); ++i)
		delete Glyph_Pages[i];
	Glyph_Pages.clear();

	// Always update the internal FreeType loading flags after resetting.
	update_load_flags();
}

void CGUITTFont::update_glyph_pages() const
{
	for (u32 i = 0; i != Glyph_Pages.size(); ++i)
	{
		Glyph_Pages[i]->updateTexture();
	}
}

CGUITTGlyphPage* CGUITTFont::getLastGlyphPage() const
{
	CGUITTGlyphPage* page = 0;
	if (Glyph_Pages.empty())
		return 0;
	else
	{
		page = Glyph_Pages[getLastGlyphPageIndex()];
		if (page->available_slots == 0)
			page = 0;
	}
	return page;
}

CGUITTGlyphPage* CGUITTFont::createGlyphPage(const u8& pixel_mode)
{
	CGUITTGlyphPage* page = 0;

	// Name of our page.
	io::path name("TTFontGlyphPage_");
	name += tt_face->family_name;
	name += ".";
	name += tt_face->style_name;
	name += ".";
	name += size;
	name += "_";
	name += Glyph_Pages.size(); // The newly created page will be at the end of the collection.

	// Create the new page.
	page = new CGUITTGlyphPage(Driver, name);

	// Determine our maximum texture size.
	// If we keep getting 0, set it to 1024x1024, as that number is pretty safe.
	core::dimension2du max_texture_size = max_page_texture_size;
	if (max_texture_size.Width == 0 || max_texture_size.Height == 0)
		max_texture_size = Driver->getMaxTextureSize();
	if (max_texture_size.Width == 0 || max_texture_size.Height == 0)
		max_texture_size = core::dimension2du(1024, 1024);

	// We want to try to put at least 144 glyphs on a single texture.
	core::dimension2du page_texture_size;
	if (size <= 21) page_texture_size = core::dimension2du(256, 256);
	else if (size <= 42) page_texture_size = core::dimension2du(512, 512);
	else if (size <= 84) page_texture_size = core::dimension2du(1024, 1024);
	else if (size <= 168) page_texture_size = core::dimension2du(2048, 2048);
	else page_texture_size = core::dimension2du(4096, 4096);

	if (page_texture_size.Width > max_texture_size.Width || page_texture_size.Height > max_texture_size.Height)
		page_texture_size = max_texture_size;

	if (page)
	{
		if (!page->createPageTexture(pixel_mode, page_texture_size))
			// TODO: add error message?
			return 0;

		// Determine the number of glyph slots on the page and add it to the list of pages.
		page->available_slots = (page_texture_size.Width / size) * (page_texture_size.Height / size);
		Glyph_Pages.push_back(page);
	}
	return page;
}

void CGUITTFont::setTransparency(const bool flag)
{
	use_transparency = flag;
	reset_images();
}

void CGUITTFont::setMonochrome(const bool flag)
{
	use_monochrome = flag;
	reset_images();
}

void CGUITTFont::setFontHinting(const bool enable, const bool enable_auto_hinting)
{
	use_hinting = enable;
	use_auto_hinting = enable_auto_hinting;
	reset_images();
}

void CGUITTFont::draw(const core::stringw& text, const core::rect<s32>& position, video::SColor color, bool hcenter, bool vcenter, const core::rect<s32>* clip)
{
//	HC1_PROFILE( s32 profParse = getProfiler().add(L"parse", L"PGROUP_TTF"); )
//	HC1_PROFILE( s32 profImage = getProfiler().add(L"img", L"PGROUP_TTF"); )
//	HC1_PROFILE(CProfileScope ps1(L"TTFdraw", L"PGROUP_TTF");)

	if (!Driver)
		return;

	// Clear the glyph pages of their render information.
	for (u32 i = 0; i < Glyph_Pages.size(); ++i)
	{
		Glyph_Pages[i]->render_positions.set_used(0);
		Glyph_Pages[i]->render_source_rects.set_used(0);
	}

	// Set up some variables.
	core::dimension2d<s32> textDimension;
	core::position2d<s32> offset = position.UpperLeftCorner;

	// Determine offset positions.
	if (hcenter || vcenter)
	{
		textDimension = getDimension(text.c_str());

		if (hcenter)
			offset.X = ((position.getWidth() - textDimension.Width) >> 1) + offset.X;

		if (vcenter)
			offset.Y = ((position.getHeight() - textDimension.Height) >> 1) + offset.Y;
	}

	// Start parsing characters.

//	HC1_PROFILE(getProfiler().start(profParse);)

	u32 n;
	wchar_t previousChar = 0;
	const wchar_t * iter = text.c_str();
	while (*iter)
	{
		wchar_t currentChar = *iter;
		n = getGlyphIndexByChar(currentChar);
		bool visible = (Invisible.findFirst(currentChar) == -1);
		if (n > 0 && visible)
		{
			bool lineBreak=false;
			if (currentChar == L'\r') // Mac or Windows breaks
			{
				lineBreak = true;
				if (*(iter + 1) == L'\n')	// Windows line breaks.
					currentChar = *(++iter);
			}
			else if (currentChar == L'\n') // Unix breaks
			{
				lineBreak = true;
			}

			if (lineBreak)
			{
				previousChar = 0;
				offset.Y += font_metrics.ascender / 64;
				offset.X = position.UpperLeftCorner.X;

				if (hcenter)
					offset.X += (position.getWidth() - textDimension.Width) >> 1;
				++iter;
				continue;
			}

			SGUITTGlyph& glyph = Glyphs[n-1];

			// Calculate the glyph offset.
			s32 offx = glyph.offset.X;
			s32 offy = (font_metrics.ascender / 64) - glyph.offset.Y;

			// Apply kerning.
			core::vector2di k = getKerning(currentChar, previousChar);
			offset.X += k.X;
			offset.Y += k.Y;

			// Determine rendering information.
			CGUITTGlyphPage* const page = Glyph_Pages[glyph.glyph_page];
			page->render_positions.push_back(core::position2di(offset.X + offx, offset.Y + offy));
			page->render_source_rects.push_back(glyph.source_rect);
		}
		offset.X += getWidthFromCharacter(currentChar);

		previousChar = currentChar;
		++iter;
	}
//	HC1_PROFILE(getProfiler().stop(profParse);)

	// Draw now.
	update_glyph_pages();
	if (!use_transparency)
		color.color |= 0xff000000;

//	HC1_PROFILE(getProfiler().start(profImage);)
	for (u32 i = 0; i < Glyph_Pages.size(); ++i)
	{
		CGUITTGlyphPage* page = Glyph_Pages[i];

		if ( !page->render_positions.empty() )
		{
			Driver->draw2DImageBatch(page->texture, page->render_positions, page->render_source_rects, clip, color, true);
		}
	}
//	HC1_PROFILE(getProfiler().stop(profImage);)
}

core::dimension2d<u32> CGUITTFont::getCharDimension(const wchar_t ch) const
{
	return core::dimension2d<u32>(getWidthFromCharacter(ch), getHeightFromCharacter(ch));
}

core::dimension2d<u32> CGUITTFont::getDimension(const wchar_t* text) const
{
	// Get the maximum font height.  Unfortunately, we have to do this hack as
	// Irrlicht will draw things wrong.  In FreeType, the font size is the
	// maximum size for a single glyph, but that glyph may hang "under" the
	// draw line, increasing the total font height to beyond the set size.
	// Irrlicht does not understand this concept when drawing fonts.  Also, I
	// add +1 to give it a 1 pixel blank border.  This makes things like
	// tooltips look nicer.
	s32 test1 = getHeightFromCharacter(L'g') + 1;
	s32 test2 = getHeightFromCharacter(L'j') + 1;
	s32 test3 = getHeightFromCharacter(L'_') + 1;
	s32 max_font_height = core::max_(test1, core::max_(test2, test3));

	core::dimension2d<u32> text_dimension(0, max_font_height);
	core::dimension2d<u32> line(0, max_font_height);

	wchar_t previousChar = 0;
	for(const wchar_t* iter = text; *iter; ++iter)
	{
		bool lineBreak = false;
		if (*iter == L'\r')	// Mac or Windows line breaks.
		{
			lineBreak = true;
			if (*(iter + 1) == L'\n')
			{
				++iter;
			}
		}
		else if (*iter == L'\n')	// Unix line breaks.
		{
			lineBreak = true;
		}

		// Kerning.
		core::vector2di k = getKerning(*iter, previousChar);
		line.Width += k.X;
		previousChar = *iter;

		// Check for linebreak.
		if (lineBreak)
		{
			previousChar = 0;
			text_dimension.Height += line.Height;
			if (text_dimension.Width < line.Width)
				text_dimension.Width = line.Width;
			line.Width = 0;
			line.Height = max_font_height;
			continue;
		}
		line.Width += getWidthFromCharacter(*iter);
	}
	if (text_dimension.Width < line.Width)
		text_dimension.Width = line.Width;

	return text_dimension;
}

inline u32 CGUITTFont::getWidthFromCharacter(wchar_t c) const
{
	// Set the size of the face.
	// This is because we cache faces and the face may have been set to a different size.
	//FT_Set_Pixel_Sizes(tt_face, 0, size);

	u32 n = getGlyphIndexByChar(c);
	if (n > 0)
	{
		int w = Glyphs[n-1].advance.x / 64;
		return w;
	}
	if (c >= 0x2000)
		return (font_metrics.ascender / 64);
	else return (font_metrics.ascender / 64) / 2;
}

inline u32 CGUITTFont::getHeightFromCharacter(wchar_t c) const
{
	// Set the size of the face.
	// This is because we cache faces and the face may have been set to a different size.
	//FT_Set_Pixel_Sizes(tt_face, 0, size);

	u32 n = getGlyphIndexByChar(c);
	if (n > 0)
	{
		// Grab the true height of the character, taking into account underhanging glyphs.
		s32 height = (font_metrics.ascender / 64) - Glyphs[n-1].offset.Y + Glyphs[n-1].source_rect.getHeight();
		return height;
	}
	if (c >= 0x2000)
		return (font_metrics.ascender / 64);
	else return (font_metrics.ascender / 64) / 2;
}

u32 CGUITTFont::getGlyphIndexByChar(wchar_t c) const
{
	// Get the glyph.
	u32 glyph = FT_Get_Char_Index(tt_face, c);

	// If our glyph is already loaded, don't bother doing any batch loading code.
	if (glyph != 0 && Glyphs[glyph - 1].isLoaded)
		return glyph;

	// Determine our batch loading positions.
	u32 half_size = (batch_load_size / 2);
	u32 start_pos = 0;
	if ((u32)c > half_size)
		start_pos = c - half_size;
	u32 end_pos = start_pos + batch_load_size;

	// Load all our characters.
	do
	{
		// Get the character we are going to load.
		u32 char_index = FT_Get_Char_Index(tt_face, start_pos);

		// If the glyph hasn't been loaded yet, do it now.
		if (char_index)
		{
			SGUITTGlyph& glyph = Glyphs[char_index - 1];
			if (!glyph.isLoaded)
			{
				glyph.preload(char_index, tt_face, Driver, size, load_flags);
				Glyph_Pages[glyph.glyph_page]->pushGlyphToBePaged(&glyph);
			}
		}
	}
	while (++start_pos < end_pos);

	// Return our original character.
	return glyph;
}

s32 CGUITTFont::getCharacterFromPos(const wchar_t* text, s32 pixel_x) const
{
	s32 x = 0;

	u32 character = 0;
	wchar_t previousChar = 0;
	while (text[character])
	{
		wchar_t c = text[character];
		x += getWidthFromCharacter(c);

		// Kerning.
		core::vector2di k = getKerning(c, previousChar);
		x += k.X;

		if (x >= pixel_x)
			return character;

		previousChar = c;
		++character;
	}

	return -1;
}

void CGUITTFont::setKerningWidth(s32 kerning)
{
	GlobalKerningWidth = kerning;
}

void CGUITTFont::setKerningHeight(s32 kerning)
{
	GlobalKerningHeight = kerning;
}

s32 CGUITTFont::getKerningWidth(const wchar_t* thisLetter, const wchar_t* previousLetter) const
{
	if (tt_face == 0)
		return GlobalKerningWidth;
	if (thisLetter == 0 || previousLetter == 0)
		return 0;

	// Return only the kerning width.
	return getKerning(*thisLetter, *previousLetter).X;
}

s32 CGUITTFont::getKerningHeight() const
{
	// FreeType 2 currently doesn't return any height kerning information.
	return GlobalKerningHeight;
}

core::vector2di CGUITTFont::getKerning(const wchar_t thisLetter, const wchar_t previousLetter) const
{
	if (tt_face == 0 || thisLetter == 0 || previousLetter == 0)
		return core::vector2di();

	// Set the size of the face.
	// This is because we cache faces and the face may have been set to a different size.
	FT_Set_Pixel_Sizes(tt_face, 0, size);

	core::vector2di ret(GlobalKerningWidth, GlobalKerningHeight);

	// If we don't have kerning, no point in continuing.
	if (!FT_HAS_KERNING(tt_face))
		return ret;

	// Get the kerning information.
	FT_Vector v;
	FT_Get_Kerning(tt_face, getGlyphIndexByChar(previousLetter), getGlyphIndexByChar(thisLetter), FT_KERNING_DEFAULT, &v);

	// If we have a scalable font, the return value will be in font points.
	if (FT_IS_SCALABLE(tt_face))
	{
		// Font points, so divide by 64.
		ret.X += (v.x / 64);
		ret.Y += (v.y / 64);
	}
	else
	{
		// Pixel units.
		ret.X += v.x;
		ret.Y += v.y;
	}
	return ret;
}

void CGUITTFont::setInvisibleCharacters(const wchar_t *s)
{
	Invisible = s;
}

video::ITexture* CGUITTFont::getPageTextureByIndex(u32 page_index) const
{
	if (page_index < Glyph_Pages.size())
		return Glyph_Pages[page_index]->texture;
	else
		return 0;
}

core::dimension2d<u32> CGUITTFont::getDimensionUntilEndOfLine(const wchar_t* p) const
{
	core::stringw s;
	for (const wchar_t* temp = p; temp && *temp != '\0' && *temp != L'\r' && *temp != L'\n'; ++temp )
		s.append(*temp);

	return getDimension(s.c_str());
}

} // end namespace gui
} // end namespace irr

#endif

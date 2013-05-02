/***************************************************************************//**
 * FILE : graphicsfont.hpp
 */
/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is libZinc.
 *
 * The Initial Developer of the Original Code is
 * Auckland Uniservices Ltd, Auckland, New Zealand.
 * Portions created by the Initial Developer are Copyright (C) 2013
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */
#ifndef __ZN_GRAPHICSFONT_HPP__
#define __ZN_GRAPHICSFONT_HPP__

#include "zinc/graphicsfont.h"

namespace zinc
{

class GraphicsFont
{
protected:
	Cmiss_graphics_font_id id;

public:

	GraphicsFont() : id(0)
	{  }

	// takes ownership of C handle, responsibility for destroying it
	explicit GraphicsFont(Cmiss_graphics_font_id font_id) : id(font_id)
	{  }

	GraphicsFont(const GraphicsFont& font) : id(Cmiss_graphics_font_access(font.id))
	{  }

	GraphicsFont& operator=(const GraphicsFont& font)
	{
		Cmiss_graphics_font_id temp_id = Cmiss_graphics_font_access(font.id);
		Cmiss_graphics_font_destroy(&id);
		id = temp_id;
		return *this;
	}

	~GraphicsFont()
	{
		Cmiss_graphics_font_destroy(&id);
	}

	bool isValid()
	{
		return (0 != id);
	}

	Cmiss_graphics_font_id getId()
	{
		return id;
	}

	char *getName()
	{
		return Cmiss_graphics_font_get_name(id);
	}

	int setName(const char *name)
	{
		return Cmiss_graphics_font_set_name(id, name);
 	}

};

} // namespace zinc

#endif /* __ZN_GRAPHICSFONT_HPP__ */
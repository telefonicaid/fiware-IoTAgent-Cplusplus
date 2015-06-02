//=============================================================================
//	This library is free software; you can redistribute it and/or modify it
//	under the terms of the GNU Library General Public License as published
//	by the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version.
//
//	This library is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//	Library General Public License for more details.
//
//	The GNU Public License is available in the file LICENSE, or you
//	can write to the Free Software Foundation, Inc., 59 Temple Place -
//	Suite 330, Boston, MA 02111-1307, USA, or you can find it on the
//	World Wide Web at http://www.fsf.org.
//=============================================================================
/** \file
 * \author John Bridgman
 * \brief An advanced schema loader.
 */
#ifndef VARIANT_SCHEMALOADER_H
#define VARIANT_SCHEMALOADER_H
#pragma once

#include <Variant/Schema.h>

namespace libvariant {

	//! The advanced schema loader will try to load missing schemas from
	//! the file system and optionally curl if built.
	//! If the url begins with file:// it will always use LoadFromFile
	//! otherwise if the protocol is empty it will try loading from a file
	//! then try loading from curl. Otherwise if the protocol is not file://
	//! it will try to load from curl.
	class AdvSchemaLoader : public SchemaLoader {
	public:

		//! The advanced schema loader will search for a schema in the paths
		//! added to it on the file system.  If the schema uri looks like a
		//! relative path, then it will be searched for in the paths set with
		//! this function. If no paths are added, then only absolute paths will
		//! be looked up in the file system.
		//
		void AddPath(const std::string &path);

		virtual bool HandleMissing(const std::string &uri, LoaderError &error);
	protected:
		bool LoadFromFile(URIInfo &info, LoaderError &error);
		bool LoadFromCurl(URIInfo &info, LoaderError &error);
		std::vector< std::string > paths;
	};
}
#endif

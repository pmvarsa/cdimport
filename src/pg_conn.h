#pragma once

#include <string>

#include <pqxx/pqxx>

#include "cd.h"

/// Data Layer Wrapper. Database operations are encapsulated with this class.
/// Methods in this class have a fair amount of boiler-late code, which is
/// provided in the macros #TRY, #CONN, and #CATCH.
///
/// \TODO Ideally, this class would actually process the data and return it to
/// the rest of the application in a typed format suitable for the application,
/// but that's a lot of work that I don't want to do right now.
class PgConn
{
  private:

	static const std::string DB_NAME;		///< The name of the database.
	static const std::string DB_HOST;		///< The database host.
	static const std::string DB_USER;		///< The user of the database.

	/// Static creation of a database connection string useing other members.
	static const std::string DB_CONNECTION_STRING;

  public:
	/// Query the database to see if a `cd-discid` tool entry already exists.
	/// @param cdDiscId A `cd-discid` string to query for.
	/// @return Returns the raw pqxx::result data.
	static pqxx::result queryCdDiscId(const std::string & cdDiscId);

	/// Query for an album by artist and title.
	/// @param artist The artist's name.
	/// @param title The title of the album.
	/// @return Returns the raw pqxx::result data.
	static pqxx::result queryArtistTitle(const std::string & artist,
										 const std::string & title);

	/// Insert a CD into the database.
	/// @param album The information to store regarding this album.
	/// @param tracks The track information for this album.
	static bool insertCd(const Cd::CdAlbumData & album, const Track::TrackList & tracks);
};


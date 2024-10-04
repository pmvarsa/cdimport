
#pragma once

#include <string>
#include <tuple>

/// Encapsulate CD data into a struct to separate it from track information.
/// \TODO Is a namespace a better option?
struct Cd
{
	/// Use a standard tuple to represent the data required to add an album to the
	/// database. The fields are as follows:
	/// - Medium ID: Should be set to CdImport::CD_MEDIUM_ID.
	/// - Type ID: Single, EP, or LP.
	/// - Category ID: One of the (very) limited CDDB categories.
	/// - Is Compilation: True or false, duh.
	/// - Result ID: The raw cddb-tool output.
	/// - Disc ID: The computed ID from the cd-discid tool.
	/// - Title: The album title, which may be hand edited.
	/// - Artist: The album artist, which may be hand edited.
	/// - Genre: The genre, which may be hand edited.
	/// - Length: The total runtime of the CD in seconds.
	/// - Extra Info: Extra information text, which may be hand edited.
	/// - Year: An integer year, which may be hand edited.
	/// - Number of Tracks: A postitive integer.
	typedef std::tuple<
		int,				// medium ID
		int,				// type ID
		int,				// category ID
		bool,				// is a compilation
		std::string,		// cddb-tool result ID
		std::string,		// cd-discid
		std::string,		// title
		std::string,		// artist
		std::string,		// genre
		int,				// total CD length
		std::string,		// extra info
		int,				// year
		int					// number of tracks
	> CdAlbumData;
	
	/// Provide a convenient way to index into a CdAlbumData tuple.
	enum CdIndexes
	{
		MediumId = 0,
		TypeId,
		CategoryId,
		IsCompilation,
		ResultId,
		DiscId,
		Title,
		Artist,
		Genre,
		Length,
		ExtraInfo,
		Year,
		NumberOfTracks
	};
}; // struct Cd

/// Encapsulate track data into a struct to separate it from CD information.
/// \TODO Is a namespace a better option?
struct Track
{
	/// A std::tuple object used to provide the data for the database. This
	/// represents a single row in the DB.
	/// \TODO Need to have two separate tuples, one for CD Import and a second for
	///       adding an album that contains the "side" (as in sides 1 and 2 of an
	///       LP record) but does not contain length -- or maybe it does?
	typedef std::tuple<
		std::string,		// title
		int,				// length
		std::string			// extra info
	> TrackRecord;
	
	/// The list of tracks on an album.
	/// \TODO Merge this with TrackData.
	typedef std::vector<TrackRecord> TrackList;

	/// Provide a convenient way to index into a TrackRecord tuple.
	enum TrackIndexes
	{
		Title = 0,			// title
		Length_S,			// length in seconds
		ExtraInfo			// extra information
	};
}; // struct Track


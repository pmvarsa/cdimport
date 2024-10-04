
#pragma once

#include <memory>
#include <string>
#include <vector>

#include "cd.h"
#include "exceptions.h"
#include "track_data_model.h"

/// This class encapsulates the data and results of a CDDB entry.
///
/// This class executes command-line shell tools to determine the identifier of
/// the CD (via `cd-discid`) and then to query database information about the CD
/// (via `cddb-tool`). The general work flow is as follows. First, cd-disc ID
/// computes an "ID" for a CD.  ID is used here loosely.  It's a number of a
/// sort, but it's not an exact hash. Sometimes two different CDs will produce
/// the same ID. With the CD *Storm Boy* by Xavier Rudd in a device connected to
/// /dev/cdrom, the following command is executed:
///
/// `% cd-discid /dev/cdrom`
///
/// and this output is:
///
/// `a70d520d 13 150 17810 40193 58124 74930 92012 115019 135982 150866 169655 183316 196229 230599 3412`
///
/// The first outputted value is the ID, namely, a70d520d. The second value is
/// the number of tracks, i.e., 13. The next 13 numbers are the track frame
/// offsets. So, Track 1 starts at fram 150, track 2 starts at frame 17810,
/// etc.. Note that there are 75 "frames" in a second. So, track 1 starts at the
/// 2 second make of the disc, and track 2 starts at the 237.47 second mark.
/// More spcifically, track 1 is approximately 235 seconds long, which is 3m55s.
/// The final output is the length of the entire CD in seconds (not frames).
///
/// Once the discid is obtained, the `cddb-tool` utility is utilized to obtian a
/// list of potential matching CDs. This is performed with the "query" command.
/// Continuing with the example above, we have the command:
///
/// `% cddb-tool query https://gnudb.gnudb.org/~cddb/cddb.cgi 6 pmvarsa`
/// `pumpkin a70d520d 13 150 17810 40193 58124 74930 92012 115019 135982`
/// `150866 169655 183316 196229 230599 3412`
///
/// Note the use of the "query" command to query the CDDB database to search for
/// matching results. The server to query, protocol version (6), querying user,
/// and querying host are specified nex. Last, the output of the `cd-discid`
/// tool is provided to complete the query. The output of this command contains
/// the following lines:
///
/// <pre>
/// 210 Found exact matches, list follows (until terminating `.')
/// data a70d5289 Xavier Rudd / Storm Boy
/// data a60d5288 Xavier Rudd / Storm Boy
/// \.
/// </pre>
///
/// The first line contains the status code, the list of matches, and then the
/// last line terminates the output with a period. The following result codes
/// are possible:
/// - 200: one exact result
/// - 202: unhandled
/// - 210: multiple, exact results returned
/// - 211: inexact matches returned
/// In this case the there are two, exact matches, which is reflected by the
/// result code 210. The user must select one of these two options to continue.
/// Having selected one of the two options, the tracks on the CD can be sought
/// after by using the `cddb-tool read` command. We will arbitrarily select the
/// first option, in order to continue with the example:
///
/// `% cddb-tool read https://gnudb.gnudb.org/~cddb/cddb.cgi 6 pmvarsa pumpkin`
/// `  data a70d5289 Xavier Rudd / Storm Boy`
///
/// The command is structured the same as the above "query" command, except that
/// the "query" command has been replaced by "read", and the output of the
/// `cd-discid` output has been replaced by the output of the previous
/// `cddb-tool query` command execution.
/// The output of this command is quite long. I was going to omit it, but
/// whatever, I'll dump it all here anyway. It's pretty self-explanatory.
///
/// <pre>
/// # xmcd
/// #
/// # Track frame offsets:
/// #        150
/// #        17810
/// #        40193
/// #        58124
/// #        74930
/// #        92012
/// #        115019
/// #        135982
/// #        150866
/// #        169655
/// #        183316
/// #        196229
/// #        230599
/// #
/// # Disc length: 3412 seconds
/// #
/// # Revision: 0
/// # Processed by: gnucddb v1.0.1 Copyright (c) Gnudb.
/// # Submitted via: ExactAudioCopyFreeDBPlugin 1.0
/// #
/// DISCID=a70d520d
/// DTITLE=Xavier Rudd / Storm Boy
/// DYEAR=2018
/// DGENRE=Pop-Folk
/// TTITLE0=Walk Away
/// TTITLE1=Keep It Simple
/// TTITLE2=Storm Boy
/// TTITLE3=Honeymoon Bay
/// TTITLE4=Fly Me High
/// TTITLE5=Gather the Hands
/// TTITLE6=Best That I Can
/// TTITLE7=Feet on the Ground
/// TTITLE8=Growth Lines
/// TTITLE9=True to Yourself
/// TTITLE10=Before I Go
/// TTITLE11=True Love
/// TTITLE12=Times Like These
/// EXTD=
/// EXTT0=
/// EXTT1=
/// EXTT2=
/// EXTT3=
/// EXTT4=
/// EXTT5=
/// EXTT6=
/// EXTT7=
/// EXTT8=
/// EXTT9=
/// EXTT10=
/// EXTT11=
/// EXTT12=
/// PLAYORDER=
/// \.
/// </pre>
///
/// The fields are fairly self explanatory, and quite easy to parse with
/// regular expressions.
///
/// \TODO Create an abstact base class for the Cddb class to act as an API to
///       the database. The purpose of this ABC is to prepare for a switch to using
///       the more modern MusicBrainz database and their API.
class Cddb
{
  public:

	/// The (very limited) set of CDDB music categories.
	static const std::string VALID_CATEGORIES[];

	/// The (very limited) number of CDDB music categories.
	static const size_t NUM_VALID_CATEGORIES = 11;

	/// The CDDB protocol level in use by the class.
	static const int PROTO_LEVEL = 6;

	/// The fully-qualified GNU DB server name.
	static const std::string SERVER;

	/// This is used to compute the length of the tracks.
	static const int CD_FRAME = 75;

	/// The device name of the CDROM drive.
	static const std::string CD_DEVICE;

	/// Construct a Cddb instance. This method looks up the `cd-discid` of the
	/// CD, and gets and stores the results of querying the CDDB for this disc
	/// via the `cddb-tool query` command.
	Cddb();

	/// Not virtual since there is no inhertiance here.
	~Cddb();

	/// Check if a CD was found in the CDROM drive.
	/// @return Returns true if a disc was found in the drive, false otherwise.
	inline bool discFound() const { return _discFound; }

	/// Check if multiple results were returned.
	/// @return Returns true if multiple results were returned for the CD in the drive.
	inline bool isMultiple() const { return _results.size() > 1; }

	/// Test if any results were found by the `cddb-tool query`.
	/// @return Returns true if the size of the search results is zero.
	inline bool noResults() const { return _results.size() == 0; }

	/// Check if inexact matches were found.
	/// Returns true if the `cddb-tool` status code indicates that inexact matches were found.
	inline bool isInexact() const { return _inexact; }

	/// Provide read-only access to the disc ID field returned from the
	/// `cd-discid` query.
	/// @return Returns the disc ID from the CDDB query, or empty string if
	///         there was not disc in the drive.
	inline const std::string & cdDiscId() const { return _cdDiscId; }

	/// For inexact matches, the disc ID selected by the user needs to be
	/// specified for future queries, such as for track information.
	/// @param val The selected disc ID to be used for this CD.
	inline void setCdDiscId(const std::string & val) { _cdDiscId = val; }

	/// Get the album title.
	/// @return Returns a refernce to the internal member.
	inline const std::string & title() const { return _title; }

	/// Get the album artist.
	/// @return Returns a refernce to the internal member.
	inline const std::string & artist() const { return _artist; }

	/// Get the (very limited) category of the CD. This is the original list of
	/// music content categories in the initial protocal. This inlucdes blues,
	/// classical, country, data, folk, jazz, new age, reggae, rock, soundtrack,
	/// and misc.. New age? Really? Can you tell when the protocol was made?
	/// @return Returns a refernce to the internal member.
	inline const std::string & category() const { return _category; }

	/// Get the album genre.
	/// @return Returns a refernce to the internal member.
	inline const std::string & genre() const { return _genre; }

	/// Get the album length.
	/// @return Returns a refernce to the internal member.
	inline int length() const { return _length; }

	/// Get the album's extra information.
	/// @return Returns a refernce to the internal member.
	inline const std::string & extraInfo() const { return _extraInfo; }

	/// Get the album year.
	/// @return Returns a refernce to the internal member.
	inline int year() const { return _year; }

	/// Provide read-only access to the underlying data that represents the
	/// tracks on the CD.
	/// @return Returns a const reference.
	inline const Track::TrackList & tracks() const { return _tracks; }

	/// The raw row value returned from the `cddb-tool query` command that
	/// corresponds to the choice selected by the user.
	/// @return Returns the line entry of the `cddb-tool query` result.
	inline const std::string & selectedResult() const { return _result; }

	/// Return the number of tracks on the CD.
	/// @return Should always return a non-negative number.
	inline int numberOfTracks() { return tracks().size(); }

	/// Fetch all the track information of a `cddb-tool query` result -- either
	/// the sole result of a query, or the one selected by the user. This
	/// employs the `cddb-tool read` command to get the data and populate the
	/// remaining data structures of this class.
	/// @param which Which result to fetch. The default value is zero. For the
	///        cases of multiple or inexact results, the which parameter
	///        indicates the user's selection.
	void fetchTracks(int which = 0);

	/// Query the CDDB server using the `cddb-tool`.
	/// 
	/// **NB**: GnuDB only returns "data" for the category. Frustrating, but
	/// what can you do? They say that removing categories is an improvement,
	/// but they leave "data" for backwards compatibility. Seems odd to me.
	/// However, freedb.org is dead, and MusicBrainz removed access to their
	/// database via the cddb protocol back in 2019, so what else is there to
	/// use?
	/// - https://gnudb.org/ (accessed 2024-09-26)
	/// - https://wiki.musicbrainz.org/History:FreeDB_Gateway
	void cddbToolQuery();

	/// Provide a list of possible CDs resulting from the `cddb-tool query`.
	/// @return Returns the results of querying the CDDB for possible CDs as a
	///         read-only reference.
	inline const std::vector<std::string> & possibleMatches() const { return _results; }

  private:

	/// Return the user name of the individual using the software. This method
	/// uses lazy evaluation to get and store the user name.
	/// @retrun Returns the user name of the individual.
	const std::string & getUser();

	/// Return the host name that the software is being executed on. This method
	/// uses lazy evaluation to get and store the host name. The host name is
	/// not the FQDN, just the simple name.
	/// @retrun Returns the host name of the machine.
	const std::string & getHost();

	/// Execute a system command, but collect the output and return it as a
	/// string.
	/// @param command The command to execute is assumed to be properly formed.
	/// @return Returns the standard output of the command. Standard error is
	///         ignored. Thus, if you want to capture standard are, you need to
	///         construct a command so that it is capture. E.g., using something
	///         like 2>&1.
	const std::string execCommand(const std::string & command);

	/// Separate a multiline string containing CDDB data into separate lines.
	/// Note that the last line of well-formated CDDB data contains only a dot.
	/// @param The multi-line, raw string data.
	/// @return Returns a vector, where each value is a line from the raw string.
	const std::vector<std::string> separateRawCddbData(const std::string & raw);

	/// Given the first line of output from a CDDB query, extract the return code.
	/// @param line The first line returned from CDDB query.
	/// @return The code is the first number in the line, get it, return it.
	int getCddbCode(const std::string & line);

	/// Given a discid, process and store the values contained within it.
	/// @param discId A correctly formatted disc ID.
	void processDiscId(const std::string & discId);

  private:
	std::string _result {""};			///< The selected line item from the `cddb-tool query`.
	bool _discFound {false};			///< True if a CD found in the drive.

	std::string _rawDiscId;				///< The raw output of the `cd-discid` command.
	std::string _cdDiscId;				///< The `cd-discid` ID of the CD.
	bool _inexact;						///< True if inexact matches of the CD were found.
	int _length;						///< The total length of the CD in seconds.
	std::vector<std::string> _results;	///< A list containing all possible results for this CD.
	std::string _artist;				///< Artist of the CD, that must not be empty.
	std::string _title;					///< Title of the CD, that must not be empty.
	std::string _category;				///< The FreeDB category, that must be one of +FreeDb::VALID_CATEGORIES+.
	std::string _genre;					///< An arbitraty string for the genre.
	int _year;							///< The year of the cd (0 if not known).

	Track::TrackList _tracks;			///< The track data loaded via the `cddb-tool read` command.

	std::string _extraInfo;				///< Extra info of the CD.
	std::string * _user { nullptr };	///< Lazily-acquired user name.
	std::string * _host { nullptr };	///< Lazily-acquired host name.
	std::string _rawData;				///< The raw data returned from a complete CDDB entry
	std::vector<std::string> _data;		///< The raw data broken up by lines.
};


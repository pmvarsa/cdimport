
#include <array>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <regex>
#include <sstream>
#include <unistd.h>		// Linux only, needed for gethostname()

#include "cddb.h"

const std::string Cddb::VALID_CATEGORIES[] = {
	"blues",
	"classical",
	"country",
	"data",
	"folk",
	"jazz",
	"newage",
	"reggae",
	"rock",
	"soundtrack",
	"misc"
};

const std::string Cddb::SERVER = std::string {"https://gnudb.gnudb.org/~cddb/cddb.cgi"};

const std::string Cddb::CD_DEVICE = std::string {"/dev/cdrom"};

Cddb::Cddb()
{
	try {
		// Execute the cd-discid command to figure out the "ID" of this disc
		static const std::string cmd("cd-discid " + CD_DEVICE + " 2>&1");
		const std::string & result = execCommand(cmd);
		processDiscId(result);
		_discFound = true;

		// Get the list of possible matches using the cddb-tool query command
		cddbToolQuery();

	} catch(const NoCdFound & e) {
		_discFound = false;
	} catch(CddbError & e) {
		std::cerr <<  "Error: " << e.what() << std::endl;
		_discFound = false;
	}
}

Cddb::~Cddb()
{
	if(_user != nullptr) {
		free(_user);
		_user = nullptr;
	}
	if(_host != nullptr) {
		free(_host);
		_host = nullptr;
	}
}

void Cddb::fetchTracks(int which)
{
	_result = _results[which];
	if(_results.size() < 1) {
		throw std::runtime_error("Fetched tracks when there are no results.");
	}

#ifdef DEBUG
	using std::cout, std::endl;
	cout << "Pulling data for selected item " << which << "." << endl;
#endif
	auto user = getUser();
	auto host = getHost();

	std::stringstream commandSs;
	commandSs << "cddb-tool read "
			  << SERVER << " "
			  << PROTO_LEVEL << " "
			  << user << " "
			  << host << " "
			  << _results[which];

	std::string escapedCommand = commandSs.str();
	if((escapedCommand.find("'") != std::string::npos) or
		(escapedCommand.find("(") != std::string::npos) or
		(escapedCommand.find(")") != std::string::npos))
	{
#ifdef DEBUG
		cout << "Need to escape the raw cddb-tool command: '" << escapedCommand
			 << "', or the query will fail." << endl;
#endif
		std::regex escapeRegex("['()]");
		escapedCommand = std::regex_replace(commandSs.str(), escapeRegex, "\\$&");
	}
#ifdef DEBUG
	cout << "Pulling data for: " << _results[which] << endl;
	cout << "Executing command:" << endl << escapedCommand << endl;
#endif

	_rawData = execCommand(escapedCommand);
	_data = separateRawCddbData(_rawData);
#ifdef DEBUG
	cout << "Got " << _data.size() << " lines of data out of the command." << endl;
	cout << _rawData << endl;
#endif

	// swallow the whole response into a map
	std::map<std::string, std::string> response;
	std::regex kvp_regex("^([A-Z0-9]+)=(.*)");
	std::regex cat_regex("^[0-9]+ ([a-z]+) .*");
	for(auto line : _data) {

		// Pull out the key=value pairs from each relevant line
		std::smatch match;
		if(std::regex_search(line, match, kvp_regex)) {
			assert(match.size() == 3);
			std::string key = match[1];
			std::string value = match[2];
			response[key] = value;
#ifdef DEBUG
			cout << "Adding \"" << key << "/" << value << "\" to repsonse map." << endl;
#endif
		} else if(std::regex_search(line, match, cat_regex)) {
			assert(match.size() == 2);
			std::string value = match[1];
			response["CATEGORY"] = value;
#ifdef DEBUG
			cout << "Adding \"CATEGORY/" << value << "\" to repsonse map." << endl;
		} else {
			cout << "Ignoring line: " << line << endl;
#endif
		}
	}
	_category = response["CATEGORY"];
	_genre = response["DGENRE"];
	if(not response["DYEAR"].empty()) {
		_year = std::stoi(response["DYEAR"]);
	} else {
		_year = 0;
	}
	_extraInfo = response["EXTD"];

	std::regex artist_regex("^([^/]+) / (.*)");
	std::smatch match;
	if(std::regex_search(response["DTITLE"], match, artist_regex)) {
		assert(match.size() == 3);
		_artist = match[1];
		_title = match[2];
	} else {
		// A self-titled album may not have a title part
		_artist = _title = response["DTITLE"];
	}
#ifdef DEBUG
	cout << "Set artist to '" << _artist << "' and title to '" << _title << "'." << endl;
#endif

	for(int i=0;i<_tracks.size(); ++i) {
		std::stringstream hashKey;
		hashKey << "TTITLE" << i;
#ifdef DEBUG
		cout << "Setting track " << i << "'s title to '" << response[hashKey.str()]
			 << "' " << std::flush;
#endif
		std::get<Track::Title>(_tracks[i]) = response[hashKey.str()];
		hashKey.clear();
		hashKey.str("");			// reset hashKey string stream
		hashKey << "EXTT" << i;
		std::get<Track::ExtraInfo>(_tracks[i]) = response[hashKey.str()];
#ifdef DEBUG
		cout << "and extra info to '" << response[hashKey.str()] << "'." << endl;
#endif
	}
#ifdef DEBUG
	cout << "_tracks map contains " << _tracks.size() << " entries." << endl;
#endif
}

void Cddb::cddbToolQuery()
{
#ifdef DEBUG
	using std::cout, std::endl;
#endif
	// Build up the command to execute
	std::stringstream commandStream;
	commandStream
		<< "cddb-tool query "
		<< SERVER << " "
		<< PROTO_LEVEL << " "
		<< getUser() << " "
		<< getHost() << " "
		<< _rawDiscId;

#ifdef DEBUG
	cout << commandStream.str() << endl;
#endif

	auto rawResults = execCommand(commandStream.str());
#ifdef DEBUG
	cout << "Raw CD Results:" << endl << rawResults << endl;
#endif
	auto lines = separateRawCddbData(rawResults);
	auto resultCode = getCddbCode(lines[0]);

	if(resultCode == 200) {
#ifdef DEBUG
		cout << "One exact match: " << lines[1] << endl;
#endif
		//_results << fetch_results[0].scan(/^200 (.*)/).first.last
		_results.push_back(lines[1]);
	} else if(resultCode == 211) {
		// This code means that inexact matches were found.
		_results = std::vector<std::string>(lines.begin() +1, lines.end());
		_inexact = true;
	} else if(resultCode == 202) {
		// TODO How to test this case, no results
		// should be a no-op
	} else if(resultCode == 210) {
		// Multiple results, drop the result code and the terminating "." line.
		_results = std::vector<std::string>(lines.begin() +1, lines.end());
	} else {
		std::string err = std::string("Unhandled return code from FreeDB Query: ");// + resultCode;
		throw CddbError(err);
	}
#ifdef DEBUG
	cout << _results.size()
		 << (_results.size() == 1 ? " match was" : " matches were")
		 << " added to the results list." << endl;
#endif
}

const std::string Cddb::execCommand(const std::string & command)
{
	std::array<char, 128> buffer;
	std::stringstream result;

	std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"), pclose);
	if(not pipe) {
		throw CddbError("Error executing shell command.");
	}
	while (std::fgets(buffer.data(), static_cast<int>(buffer.size()), pipe.get()) != nullptr) {
		result << buffer.data();
	}
	return result.str();
}

const std::vector<std::string> Cddb::separateRawCddbData(const std::string & raw)
{
	std::vector<std::string> retVal;
	std::stringstream ss(raw);
	std::string line;
	while(getline(ss, line)) {
		if(line == ".") {
			// We are done here. The raw output of CDDB contains only a '.' on the last line
			break;
		}
		retVal.push_back(line);
	}
	return retVal;
}

int Cddb::getCddbCode(const std::string & line)
{
	std::istringstream iss(line);
	int code;
	iss >> code;
	return code;
}

void Cddb::processDiscId(const std::string & discId)
{
	// First, verify that a disc was found in the drive
	if (discId.find("No medium found") != std::string::npos) {
		throw NoCdFound();
	} else {
		_rawDiscId = discId;
	}

#ifdef DEBUG
	std::cout << "cd-discid result: " << discId << std::endl;
#endif

	// Process the disc ID as a stream
	std::istringstream iss(discId);
	iss >> _cdDiscId;
	int numTracks;
	iss >> numTracks;

	// Read the values for the track data
	std::vector<int> offsets(numTracks);
	for(int i=0;i<numTracks;++i) {
		iss >> offsets[i];
	}
	iss >> _length;
	// Add the total length to the end for computing differences
	offsets.push_back(_length * CD_FRAME);

	// Convert the values to seconds and store them in the correct data structure
	for(int i=0;i<numTracks;++i) {
		Track::TrackRecord tr;
		float diff = static_cast<float>(offsets[i+1]-offsets[i]);
		int duration = static_cast<int>(std::round(diff/static_cast<float>(CD_FRAME)));
		std::get<Track::Length_S>(tr) = duration;
#ifdef DEBUG
		std::cout << "Track " << i << " has duration " << std::get<Track::Length_S>(tr)
				  << "." << std::endl;
#endif
		_tracks.push_back(tr);
	}
	_inexact = false;

#ifdef DEBUG
	std::cout << _tracks.size() << " tracks were added to the tracks list. "
			  << "The total disc length is " << _length << "." << std::endl;
#endif
}

const std::string & Cddb::getUser()
{
	if(_user == nullptr) {
		const char * user = std::getenv("USER");
		assert(user);
		_user = new std::string(user);
	}
	return *_user;
}

const std::string & Cddb::getHost()
{
	if(_host == nullptr) {
		char host[1024] = "unknown";
		if(gethostname(host, sizeof(host)) != 0) {
			std::cerr << "Error getting hostname of computer. "
					  << "Using something dumb instead." << std::endl;
		}
		_host = new std::string(host);
	}
	return *_host;
}



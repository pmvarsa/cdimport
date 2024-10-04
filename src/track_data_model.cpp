
#include <cassert>
#include <iostream>
#include <sstream>

#include "track_data_model.h"

#include "cddb.h"
#include "macros.h"
#include "utility.h"

TrackDataModel::TrackDataModel(const Track::TrackList & tracks)
  : QAbstractTableModel(nullptr), _tracks(tracks.size())
{
#ifdef DEBUG
	std::cout << "Building data model." << std::endl;
#endif
	assert(tracks.size() > 0);

	// Copy the data
	_tracks = tracks;
}

// Implementation of required pure-virtual method.
int TrackDataModel::rowCount(const QModelIndex & parent) const {
	return _tracks.size();
}

// Implementation of required pure-virtual method.
int TrackDataModel::columnCount(const QModelIndex & parent) const {
	return 3;
}

QVariant TrackDataModel::data(const QModelIndex & index, int role) const
{
	if(index.isValid() and role == Qt::DisplayRole) {
		Track::TrackRecord row = _tracks[index.row()];
		switch(index.column()) {
			case 0:
				return QVariant(QStr(std::get<Track::Title>(row)));
				break;
			case 1:
				if (_hasLen) {
					int seconds = std::get<Track::Length_S>(row);
					auto pretty = Utility::readableLength(seconds);
					return QVariant(QStr(pretty));
#ifdef ONLY_FOR_ADD_ALBUM
				} else {
					return QVariant(QStr(row["side"]));
#endif
				}
				break;
			case 2:
				auto extra = std::get<Track::ExtraInfo>(row);
				return QVariant(QStr(extra));
				break;
		}
	}
	return QVariant();
}

QVariant TrackDataModel::headerData(int section,
									Qt::Orientation orientation,
									int role) const
{
	if(role == Qt::DisplayRole) {
		if(orientation == Qt::Horizontal) {
			switch(section) {
				case 0:
					return QVariant("Track Name");
					break;
				case 1:
					if(_hasLen) {
						return QVariant("Length");
					} else {
						return QVariant("Side");
					}
					break;
				case 2:
					return QVariant("Extra Information");
					break;
			}
		} else {
			std::stringstream ss;
			ss << "Track " << (section + 1);
			return QVariant(QStr(ss.str()));
		}
	}
	return QVariant();
}

void TrackDataModel::updateTrack(int which, const std::string & name,
								 const std::string & ext, const std::string * side)
{
	which -= 1;
	std::get<Track::Title>(_tracks[which]) = name;
	std::get<Track::ExtraInfo>(_tracks[which]) = ext;
#ifdef ONLY_FOR_ADD_ALBUM
	if(not _hasLen) {
		_tracks[which]["side"] = *side;
	}
#endif
#ifdef DEBUG
	std::cout << "Updated (zero-based) track " << which << " to be titled '"
		<< std::get<Track::Title>(_tracks[which]) << "' with extra info '"
		<< std::get<Track::ExtraInfo>(_tracks[which]) << "'." << std::flush;
#ifdef ONLY_FOR_ADD_ALBUM
	if(not _hasLen) {
		std::cout << " Side updated to '" << _tracks[which]["side"] << "'." << std::flush;
	}
#endif
	std::cout << std::endl;
#endif
}


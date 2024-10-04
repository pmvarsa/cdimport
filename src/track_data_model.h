
#pragma once

#include <vector>

#include <QAbstractTableModel>

#include "cd.h"

/// This class specifies the underlying model for the table view that displays
/// the track information of the CD.
/// \TODO Kill _hasLen after making the addalbum application
class TrackDataModel : public QAbstractTableModel
{
	Q_OBJECT

  public:

	/// Construct the data model by providing the track data.
	/// @param tracks The data to be displayed in the table view associated with
	/// this model.
	TrackDataModel(const Track::TrackList & tracks);

	/// Return the total number of tracks on the CD. Required.
	/// @param parent The parent object.
	/// @return The returned value is non-negative.
	virtual int rowCount(const QModelIndex & parent) const override;

	/// Returns 3. Required.
	/// @param parent The parent object.
	/// @return Returns 3.
	virtual int columnCount(const QModelIndex & parent) const override;

	/// Provide the view access to the data to. Required.
	/// @index The index into the model, zero-based.
	/// @role What role does the datum play in the view?
	/// @return For this simple data model, only strings are returned.
	virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const override;

	/// Returns the horizontal header information. Not-required.
	/// @param section The column number.
	/// @param orientation Either column header (horizontal) or row header (vertical).
	/// @param role I only handle the displayed role.
	/// @return Returns the column header names. Namely, "Track Name," "Length"
	///         or "Side," and "Extra Information."
	virtual QVariant headerData(
		int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

	/// Provide read-only access to the underlying data in the model.
	/// @return Returns a constant reference to the underlying data.
	inline const Track::TrackList & tracks() const { return _tracks; }

	/// Provide editable access to the underlying data in the model.
	/// @param i The zero-based index into the underlying data.
	/// @return Returns a reference to the underlying TrackRecord data structure.
	Track::TrackRecord & operator[](size_t i) { return _tracks[i]; }

	/// Update a track in the data model.
	/// @param which A one-based index into the data model.
	/// @param name The new name of the track.
	/// @param ext The new extended information for the track.
	/// @param side Hack. If this instance of the data model is being used to
	///        back the view for the addalbum program, then this will be
	///        non-null.
	void updateTrack(int which, const std::string & name,
					 const std::string & ext, const std::string * side = nullptr);
	
  private:

	Track::TrackList _tracks;	///< The actual data in the data model.
	bool _hasLen;				///< Is the "length" field specified in the data model?
};


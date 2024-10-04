#pragma once

#include <iostream>

#include "designer/ui_edit_track.h"

#include "track_data_model.h"

/// The track editor dialog box.
class EditTrack : public QDialog
{
	Q_OBJECT

  public:

	/// Initialize the track editor dialog box.
	/// @param parent The parent widget.
	/// @param tracks The tracks data model is updated directly by this UI.
	/// @param which  The one-based track number (i.e. the row) to set as the current track.
	/// @param field  If true, the name has focus, if false extra info has focus.
	EditTrack(QWidget * parent, TrackDataModel & tracks, int which = 1, bool field = true);

  signals:

	/// Notify that the data for a track was updates.
	/// @param num The one-based track number that has been changed.
	/// @param name The (potentially) new name of the track.
	/// @param ext The (potentially) new extended information of the track.
	void trackUpdated(int num, const QString & name, const QString & ext);

  public slots:

	/// Update the information for the current track, and present the
	/// information for the next one, numerically.
	void onNextTrackClicked();

	/// Update the information for the current track, and present the
	/// information for the previous one, numerically.
	void onPrevTrackClicked();

  private:

	/// Update the contained TrackDataModel instance.
	void updateTrack();

	Ui::EditTrack _ui;			///< The actual user interface of this dialog box.
	TrackDataModel & _tracks;	///< Store a reference to the data model so it can be updated.
	int _curr;					///< Which, one-based, track is being update.
	int _max;					///< The largest, zero-based, track number.
};


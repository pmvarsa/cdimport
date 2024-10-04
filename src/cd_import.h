#pragma once

#include <pqxx/pqxx>

#include "designer/ui_cd_import.h"

#include "track_data_model.h"

/// The principle dialog box of the application.
class CdImport : public QDialog
{
	Q_OBJECT

  public:

	static const int CD_MEDIUM_ID;	///< This is hard-coded in a database table

	/// Explicitly construct the dialog box with an optional parent.
	/// @param parent Specify an optional parent object. The default value is
	/// nullptr.
	explicit CdImport(QWidget * parent = nullptr);

  public slots:

	/// Qt slot triggered when the eject button has been clicked in the UI. If
	/// the CD tray is open, then close it, if it is closed, then open it. Note
	/// that the text of the button is also updated, however, the hotkey remains
	/// the same.
	void onEjectClicked();

	/// Qt slot triggered when the *Query* button is clicked in the UI. The CD
	/// tray is closed (if open), the CDDB Disc ID is looked up, and a list of
	/// matching CDs is presented to the user. The used chooses the matching CD
	/// and the rest of the UI is populated with the results.
	void onQueryClicked();

	/// Qt slot triggered when the edit tracks button is clicked in the UI. The
	/// user is given the option to edit the tracks, one-by-one, rather than
	/// directly in the table.
	void onEditTracksClicked();

	/// Qt slot triggered when the save button is clicked in the UI.
	void onSaveClicked();

	/// Slot to update this dialog once a change has been made to the
	/// TrackDataModel via the EditTrack dialog box.
	void updateTrack(int, const QString &, const QString &);

	/// Handle double-clicking on t the table view.
	/// @index The row and column of the element that was double-clicked.
	void trackDoubleClicked(const QModelIndex & index);

  private:

	/// Set the state of the CD tray. If set to the open state, then the UI is
	/// cleared of data.
	/// @param state Set to true for open, false for closed.
	void setCdTrayState(bool state);

	/// Given a text-based category name, update the drop-down combo box item in
	/// the UI.
	/// @param category This should be one of the (very limited) categories.
	void setCategoryByName(const std::string & category);

	// Clear all the values from the user interface, and set them to defaults.
	void clear();

	/// Show a message to the user, indicating that the CD already exists in the
	/// database.
	/// @param result The results of the `cd-discid` command.
	void showExistsDialog(const pqxx::result & result);

	/// Initialize a data model for the tracks view.
	/// @param tracks Provide the data to populate the model.
	/// @return Returns a pointer that is ready to be handed over the view.
	TrackDataModel * createTrackDataModel(const Track::TrackList & tracks);

  private:

	Ui::CdImport _ui;	///< The actual user interface instance.
	bool _cdOpen;		///< Is the CDROM tray open? The default value is false.
	int _cdLength;		///< Keep the total runtime of the CD in seconds in a variable.

	///< Keep a reference to the data model.
	TrackDataModel * _trackDataModel { nullptr };
};


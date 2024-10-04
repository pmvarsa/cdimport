
#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <regex>

#include <QMessageBox>

#include "cd_import.h"

#include "cd.h"
#include "cd_chooser.h"
#include "cddb.h"
#include "edit_track.h"
#include "exceptions.h"
#include "macros.h"
#include "pg_conn.h"
#include "utility.h"

const int CdImport::CD_MEDIUM_ID { 1 };

CdImport::CdImport(QWidget * parent)
  : QDialog(parent), _cdOpen(false)
{
	_ui.setupUi(this);

	// Connect up the signals and slots
	QObject::connect(_ui.eject, &QPushButton::clicked,
					 this, &CdImport::onEjectClicked);
	QObject::connect(_ui.query, &QPushButton::clicked,
					 this, &CdImport::onQueryClicked);
	QObject::connect(_ui.editTracks, &QPushButton::clicked,
					 this, &CdImport::onEditTracksClicked);
	QObject::connect(_ui.save, &QPushButton::clicked,
					 this, &CdImport::onSaveClicked);
	QObject::connect(_ui.tracks, &QTableView::doubleClicked,
					 this, &CdImport::trackDoubleClicked);
}

// -----------------------------  Qt Slots  ------------------------------------

void CdImport::onEjectClicked()
{
	setCdTrayState(not _cdOpen);
}

void CdImport::onQueryClicked()
{
#ifdef DEBUG
	using std::cout, std::endl, std::flush;
#endif

	// Clear the UI from old data, in the case of a re-query
	clear();

	// The user could have left the CD tray open or manually closed the
	// tray, so set the state as closed.
	setCdTrayState(false);

	try {
		int which = 0;
		bool foundResults = false;
		Cddb cd;
		if(not cd.discFound()) {
			QMessageBox::critical(this, "No Disc", "No disc was found in the CDROM drive.");
			return;
		} else if(cd.isMultiple() or cd.isInexact()) {
#ifdef DEBUG
			cout << (cd.isMultiple() ? "Multiple" : "")
				 << (cd.isMultiple() and cd.isInexact()? " and " : "")
				 << (cd.isInexact() ? "Inexact" : "")
				 << " matches for the CD were found. " << flush;
#endif

			CdChooser chooser(this, cd.isInexact());
			chooser.addRadioButtons(cd.possibleMatches());
			auto result = chooser.exec();

#ifdef DEBUG
			cout << "Option #" << chooser.selected() << " was selected." << endl;
#endif

			if(result == QDialog::Accepted) {
				which = chooser.selected();
#ifdef DEBUG
				cout << "CdChooer dialog accepted. The user selected option "
					 << which << "." << endl;
#endif
				foundResults = true;

				if(cd.isInexact()) {
					// Get the disc id from the inexact match
					std::regex regex("^[a-z]+ ([a-z0-9]+) .*$");
					std::smatch match;
					if(std::regex_search(cd.possibleMatches()[which], match, regex)) {
						// the matched string should be in location 1, 0 is the whole input string
						assert(match.size() == 2);
						cd.setCdDiscId(match[1]);
					} else {
						std::cerr << "ERROR: Regex didn't match when setting selected disc ID."
								  << std::endl;
						return;
					}
				}
			} else {
				// User rejected choices.  Cancel out.
				return;
			}
		} else if(cd.noResults()) {
			QMessageBox::warning(this, "No Match Found",
								 "No track information found for inserted disc.");
		} else {
			foundResults = true;
		}

#ifdef DEBUG
		cout << "Yay! Found a match!" << endl;
#endif

		if(foundResults) {
			cd.fetchTracks(which);

			// Populate the UI with the results of searching for the CD
			_ui.result->setText(QStr(cd.selectedResult()));
			_ui.title->setText(QStr(cd.title()));
			_ui.artist->setText(QStr(cd.artist()));
			setCategoryByName(cd.category());
			_ui.genre->setText(QStr(cd.genre()));
			_ui.extraInfo->setText(QStr(cd.extraInfo()));
			_ui.year->setText(QString::number(cd.year()));

			// If "Various" appears in the artist field, then this is a compilation
			std::regex various_regex("various", std::regex_constants::icase);
			if(std::regex_search(cd.artist(), various_regex)) {
				_ui.compilation->setChecked(true);
			}
		}

		// These data come from the physical CD
		_ui.discId->setText(QStr(cd.cdDiscId()));
		_ui.length->setText(QStr(Utility::readableLength(cd.length())));

		// Store the total runtime in seconds in a member variable since the UI
		// contains a pretty-printed string
		_cdLength = cd.length();

		_ui.numberOfTracks->setText(QString::number(cd.numberOfTracks()));
		_ui.tracks->setModel(createTrackDataModel(cd.tracks()));
		_ui.tracks->resizeColumnsToContents();

		// Choose a reasonable default for LP/EP/Single
		int numTracks = cd.numberOfTracks();
		if(numTracks <= 4) {
			_ui.type->setCurrentIndex(0);	// single
		} else if(numTracks <= 7) {
			_ui.type->setCurrentIndex(1);	// EP
		} else {
			_ui.type->setCurrentIndex(2);	// LP
		}

		if(foundResults) {
			// Enable the Save and Edit Tracks buttons
			_ui.save->setEnabled(true);
			_ui.editTracks->setEnabled(true);

			// Look if this CD exists
			auto result = PgConn::queryCdDiscId(_ui.discId->text().toStdString());
			if(std::size(result) > 0) {
				showExistsDialog(result);
			} else if(cd.isInexact()) {
				// If this was an inexact match, we should also search by album/artist
				auto inexactResults = PgConn::queryArtistTitle(cd.artist(), cd.title());
				if(inexactResults.size() > 0) {
					showExistsDialog(inexactResults);
				}
#ifdef DEBUG
			} else {
				cout << "CD Does not exist in DB." << endl;
#endif
			}
		}
	} catch(const CddbError & e) {

		std::string message = std::string("Something went wrong querying CDDB: ") + e.what();
		QMessageBox::critical(this, "Error Querrying Music Database", QStr(message));
	}
}

void CdImport::onEditTracksClicked()
{
	// Downcast the data model as a referece.
	assert(_trackDataModel != nullptr);
	auto trackEditor = EditTrack(this, *_trackDataModel);

	// Connect up the signal to update the main UI when values have changed.
	QObject::connect(&trackEditor, &EditTrack::trackUpdated,
					 this, &CdImport::updateTrack);
	trackEditor.exec();

	// Update the table size, if necessary
	_ui.tracks->resizeColumnsToContents();
}

void CdImport::onSaveClicked()
{
	assert(_trackDataModel != nullptr);
	Cd::CdAlbumData cd = make_tuple(
		CD_MEDIUM_ID,
		_ui.type->currentIndex() + 1,
		_ui.category->currentIndex() + 1,
		_ui.compilation->isChecked(),
		_ui.result->text().toStdString(),
		_ui.discId->text().isEmpty() ? "NULL" : _ui.discId->text().toStdString(),	// yuck!
		_ui.title->text().toStdString(),
		_ui.artist->text().toStdString(),
		_ui.genre->text().toStdString(),
		_cdLength,
		_ui.extraInfo->text().toStdString(),
		std::stoi(_ui.year->text().toStdString()),
		std::stoi(_ui.numberOfTracks->text().toStdString())
	);
#ifdef DEBUG
	std::cout << "Inserting album '" << std::get<Cd::Title>(cd) << "'" << std::endl;
	for(int i=0;i<_trackDataModel->tracks().size();++i) {
		const Track::TrackRecord & tl2 = (*_trackDataModel)[i];
		std::cout << "Track " << i << ". " << std::get<Track::Title>(tl2) << " ("
				  << std::get<Track::ExtraInfo>(tl2) << ")" << std::endl;
	}
#endif
	if(not PgConn::insertCd(cd, _trackDataModel->tracks())) {
		QMessageBox::critical(this, "Error Inserting Record",
			"There was an error inserting the CD record into the database.");
	} else {
		// Success! Auto-eject the CD for the user
		onEjectClicked();
	}
}

void CdImport::updateTrack(int, const QString &, const QString &)
{
	// Update all the values, there aren't too many
	_ui.tracks->reset();
}

void CdImport::trackDoubleClicked(const QModelIndex & index)
{
#ifdef DEBUG
	using std::endl, std::cout;
	cout << "Tracks table double-clicked at (row, column) = ("
		 << index.row() << ", " << index.column() << ")" << endl;
#endif
	// Downcast the data model as a referece.
	TrackDataModel & tdm = *dynamic_cast<TrackDataModel*>(_ui.tracks->model());
	int track = index.row() + 1;				// UI is one-based
	bool nameHasFocus = index.column() != 2;	// Any column except extra info
	auto trackEditor = EditTrack(this, tdm, track, nameHasFocus);

	// Connect up the signal to update the main UI when values have changed.
	QObject::connect(&trackEditor, &EditTrack::trackUpdated,
					 this, &CdImport::updateTrack);
	trackEditor.exec();

	// Update the table size, if necessary
	_ui.tracks->resizeColumnsToContents();
}

// --------------------------  Other Methods  ----------------------------------

void CdImport::setCdTrayState(bool state)
{
	using std::system;
	if(state) {
		clear();
		_ui.eject->setText("R&eject");
		// Open the tray, ignore the return value.
		auto retVal = system("eject");
	} else {
		_ui.eject->setText("&Eject");
		// Close the tray, ignore the return value.
		auto retVal = system("eject -t");
	}
	_cdOpen = state;
}

void CdImport::clear()
{
	_ui.result->setText(nullptr);
	_ui.discId->setText(nullptr);
	_ui.title->setText(nullptr);
	_ui.artist->setText(nullptr);
	_ui.category->setCurrentIndex(0);
	_ui.genre->setText(nullptr);
	_ui.length->setText(nullptr);
	_cdLength = 0;	// Don't forget to reset the CD runtime length
	_ui.extraInfo->setText(nullptr);
	_ui.year->setText(nullptr);
	_ui.numberOfTracks->setText(nullptr);

	_ui.compilation->setChecked(false);
	_ui.type->setCurrentIndex(0);

	_ui.tracks->setModel(nullptr);

	_ui.save->setEnabled(false);
	_ui.editTracks->setEnabled(false);

	// Delete the data model for the UI, if necessary
	if(_trackDataModel != nullptr) {
		delete(_trackDataModel);
		_trackDataModel = nullptr;
	}
}

TrackDataModel * CdImport::createTrackDataModel(const Track::TrackList & tracks)
{
#ifdef DEBUG
	std::cout << "Creating tracks data model. There are "
			  << tracks.size() << " tracks." << std::endl;
#endif
	assert(_trackDataModel == nullptr);
	_trackDataModel = new TrackDataModel(tracks);
	return _trackDataModel;
}

void CdImport::setCategoryByName(const std::string & category)
{
	auto found = std::find(&Cddb::VALID_CATEGORIES[0],
							Cddb::VALID_CATEGORIES + Cddb::NUM_VALID_CATEGORIES,
							category);
	// get the index via pointer arithmetic
	int index = found - Cddb::VALID_CATEGORIES;
#ifdef DEBUG
	std::cout << "Found category index " << index << " for category \""
			  << category << "\"" << std::endl;
#endif
	_ui.category->setCurrentIndex(index);
}

void CdImport::showExistsDialog(const pqxx::result & result)
{
	std::stringstream existing;
	existing << "It looks like this CD already exists in the database.\n\n";
	if(result.size() == 1) {
		pqxx::row row = result[0];
		existing << row["category"] << ": " << row["artist"] << " / " << row["title"];
	} else if(result.size() > 1) {
		for(int i=0; i<result.size(); ++i) {
			auto row = result[i];
			existing << (i+1) << "." << row["category"] << ": "
					 << row["artist"] << " / " << row["title"] << std::endl;
		}
	}
	QMessageBox::information(this, "CD Exists", QStr(existing.str()));
}


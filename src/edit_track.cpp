
#include <QPushButton>

#include "edit_track.h"

#include "macros.h"

EditTrack::EditTrack(QWidget * parent, TrackDataModel & tracks, int which, bool field)
  : QDialog(parent), _tracks(tracks), _curr(which)
{
	_ui.setupUi(this);
	int idx = which - 1;

	// Populate the UI values
	_ui.name->setText(QStr(std::get<Track::Title>(_tracks[idx])));
	_ui.extInfo->setText(QStr(std::get<Track::ExtraInfo>(_tracks[idx])));
	_max = _tracks.tracks().size();
	_ui.trackNumber->setText(QString::number(_curr));

	// Set up the intial state of the dynamically changing UI
	if(_curr == _max) {
		_ui.nextTrack->setText("E&nd");
	}
	if(not field) {
		_ui.extInfo->setFocus();
	}
	if(_curr > 1) {
		_ui.prevTrack->setEnabled(true);
	}

	// Connect up the signals and slots
	QObject::connect(_ui.nextTrack, &QPushButton::clicked,
					 this, &EditTrack::onNextTrackClicked);
	QObject::connect(_ui.prevTrack, &QPushButton::clicked,
					 this, &EditTrack::onPrevTrackClicked);
}


void EditTrack::onNextTrackClicked()
{
	updateTrack();
	if(_curr == _max) {
		accept();
	} else {
		_ui.name->setText(QStr(std::get<Track::Title>(_tracks[_curr])));
		_ui.extInfo->setText(QStr(std::get<Track::ExtraInfo>(_tracks[_curr++])));
		_ui.name->setFocus();
		_ui.trackNumber->setText(QString::number(_curr));
		if(_curr == _max) {
			_ui.nextTrack->setText("E&nd");
		}
		if(_curr > 1) {
			_ui.prevTrack->setEnabled(true);
		}
	}
}

void EditTrack::onPrevTrackClicked()
{
	updateTrack();
	if(_curr == _max) {
		_ui.nextTrack->setText("&Next");
	}
	--_curr;
	_ui.trackNumber->setText(QString::number(_curr));
	_ui.name->setText(QStr(std::get<Track::Title>(_tracks[_curr-1])));
	_ui.extInfo->setText(QStr(std::get<Track::ExtraInfo>(_tracks[_curr-1])));
	_ui.name->setFocus();
	if(_curr == 1) {
		_ui.prevTrack->setEnabled(false);
	}
}

void EditTrack::updateTrack()
{
	emit trackUpdated(_curr, _ui.name->text(), _ui.extInfo->text());
	_tracks.updateTrack(_curr, _ui.name->text().toStdString(), _ui.extInfo->text().toStdString());
}


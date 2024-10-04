#pragma once

#include <string>
#include <vector>

#include <QRadioButton>
#include <QVBoxLayout>

#include "designer/ui_cd_chooser.h"

/// Dialog box with dynamic radio buttons to select which "found" CD with either
/// a multiple or an inexact match.
class CdChooser : public QDialog
{
	Q_OBJECT

  public slots:

	///< Update the selected status when a radio button has been clicked.
	void setSelected();

  public:

	/// Construct the dialog box given the parent, indicating whether multiple
	/// results were returned, or inexact matches were returned, by the CDDB
	/// query.
	/// @param parent The parent UI object.
	/// @param inexact This is true if the dialog is being created as the result
	///        of an inexact match from CDDB.
	CdChooser(QWidget * parent, bool inexact);
	
	/// Provide access to which CD has been selected.
	/// @return Returns the index of the radio button that is selected.
	inline int selected() const { return _selected; }

	/// Manually create radio button entries. Use this method to add the radio
	/// button options.
	void addRadioButtons(const std::vector<std::string> & buttonTexts);

  private:

	int _selected { 0 };						///< The selected radio button.
	Ui::CdChooser _ui;							///< The actual user interface generated via designer.
	std::vector<QRadioButton*> _radioButtons;	///< Dynamically-created radio buttons.
};


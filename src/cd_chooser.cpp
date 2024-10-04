
#include "cd_chooser.h"

#include "macros.h"

CdChooser::CdChooser(QWidget * parent, bool inexact)
  : QDialog(parent)
{
	_ui.setupUi(this);
	if(inexact) {
		_ui.choicesBox->setTitle("Inexact matches found. Please choose carefully.");
	}
	_selected = 0;
}

void CdChooser::setSelected()
{
	for(int i=0;i<_radioButtons.size();++i) {
		if(_radioButtons[i]->isChecked()) {
			_selected = i;
			break;
		}
	}
}

void CdChooser::addRadioButtons(const std::vector<std::string> & buttonTexts)
{
	auto vbox = new QVBoxLayout();
	for(int i=0;i<buttonTexts.size();++i) {
		auto text = buttonTexts[i];
		auto option = new QRadioButton(_ui.choicesBox);
		option->setText(QStr(text));
		if(i==0) { option->setChecked(true); }
		_radioButtons.push_back(option);
		vbox->addWidget(option);
		QObject::connect(option, &QRadioButton::clicked,
						 this, &CdChooser::setSelected);
		_ui.choicesBox->setLayout(vbox);
	}
}


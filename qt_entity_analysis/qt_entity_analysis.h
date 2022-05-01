#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_qt_entity_analysis.h"


class qt_entity_analysis : public QMainWindow
{
    Q_OBJECT

public:
    qt_entity_analysis(QWidget *parent = Q_NULLPTR);

private:
    Ui::qt_entity_analysisClass ui;
	
};

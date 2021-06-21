#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_ProjectView.h"

class QDragEnterEvent;
class QDropEvent;
class ProjectView : public QMainWindow
{
    Q_OBJECT

public:
    ProjectView(QWidget *parent = Q_NULLPTR);

	public Q_SLOTS:
	/// º”‘ÿœÓƒø
	void on_actionloadProject_triggered(void);
	void on_actionClear_triggered(void);

protected:
	void dragEnterEvent(QDragEnterEvent *event);
	void dropEvent(QDropEvent *event);

	void loadModels(const QStringList& list);

private:
    Ui::ProjectViewClass ui;
};

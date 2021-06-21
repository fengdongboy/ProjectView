#include "ProjectView.h"
#include "ModelManager.h"

#include <QFileDialog>
#include <QMap>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>

ProjectView::ProjectView(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
	setAcceptDrops(true);
}

void ProjectView::on_actionloadProject_triggered(void)
{
	static QString last;
	QStringList files = QFileDialog::getOpenFileNames(
		this,
		QStringLiteral("��ѡ��һ�����߶��ģ�ʹ�"),
		last,
		"model (*.xyz *.ply *.txt *.*)");
	if (files.isEmpty())
		return;

	last = files[0];
	loadModels(files);
}

void ProjectView::on_actionClear_triggered(void)
{
	ui.widgetScene->clear();
}

void ProjectView::dragEnterEvent(QDragEnterEvent *event)
{
	if (event->mimeData()->hasUrls())
		event->acceptProposedAction();
	else
		event->ignore();
}

void ProjectView::dropEvent(QDropEvent *event)
{
	const QMimeData *mimeData = event->mimeData();      // ��ȡMIME����
	if (mimeData->hasUrls()) 
	{
		// ��������а���URL
		QList<QUrl> urlList = mimeData->urls();         // ��ȡURL�б�
														// �����е�һ��URL��ʾΪ�����ļ�·��

		QStringList list;
		for (auto l:urlList)
		{
			list.push_back(l.toLocalFile());
		}
		loadModels(list);
	}
}

void ProjectView::loadModels(const QStringList& files)
{
	QMap<QString, PtCloud> ptlouds;
	QMap<QString, QMatrix4x4> matrixs;

	QString name;
	PtCloud c;
	QMatrix4x4 m;
	bool b = false;
	ModelManager* mm = ModelManager::getModelManager();
	for (const QString& s : files)
	{
		name = s.mid(s.lastIndexOf("/") + 1, s.lastIndexOf(".") - s.lastIndexOf("/") - 1);
		if (s.right(3) == "txt")
		{
			b = mm->loadMatrix(s, m);
			if (b)
				matrixs.insert(name, m);
		}
		else if (s.right(3) == "xyz")
		{
			b = mm->loadXyz(s, c);
			if (b)
				ptlouds.insert(name, c);
		}
		else if (s.right(3) == "ply")
		{
			b = mm->loadPly(s, c);
			if (b)
				ptlouds.insert(name, c);
		}
	}
	QMap<QString, PtCloud>::iterator it1 = ptlouds.begin();
	QMap<QString, PtCloud>::iterator it2 = ptlouds.end();
	for (; it1 != it2; ++it1)
	{
		if (!matrixs.isEmpty() && matrixs.contains(it1.key()))
		{
			mm->tranModel(matrixs[it1.key()], it1.value());
			ui.widgetScene->addCloud(&it1.value(), matrixs[it1.key()]);
		}
		else ui.widgetScene->addCloud(&it1.value(), QMatrix4x4());
	}
}

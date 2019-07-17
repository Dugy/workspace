#ifndef WORKSPACE_H
#define WORKSPACE_H

#include <functional>
#include <QFrame>
#include <memory>
#include "serialisable.hpp"

class QMenuBar;
class QMenu;
class QAction;
class QSplitter;
class Workspace;
class QVBoxLayout;

class WorkspaceContent : public QWidget, public Serialisable {
	Q_OBJECT
	void setParentWorkspace(Workspace* parent);
public:
	explicit WorkspaceContent(Workspace* parent = nullptr);
protected:
	QMenuBar* parentMenu();
	Workspace* parentWorkspace_;
	friend class Workspace;
};

class Workspace : public QFrame, public Serialisable
{
	Q_OBJECT
public:

	enum class SplitOrientation {
		HORIZONTAL,
		VERTICAL,
		UNSPECIFIED
	};

	explicit Workspace(SplitOrientation orientation = SplitOrientation::UNSPECIFIED, WorkspaceContent* containingWidget = nullptr, Workspace* parentWorkspace = nullptr, QWidget* parent = nullptr);

signals:

private slots:
	void addWidgetHorizontal();
	void addWidgetVertical();
	void removeSelf();

public slots:
	void replaceWidget(WorkspaceContent* former, const std::string* name, WorkspaceContent* newContent);

private:
	QMenuBar* menu_ = nullptr;
	QMenu* layoutMenu_;
	QAction* addWidgetRight_;
	QAction* addWidgetDown_;

	QMenu* changeContentMenu_;
	QAction* replaceContentAction_;
	QAction* removeContentAction_;

	SplitOrientation orientation_;
	QSplitter* splitterContents_ = nullptr;
	WorkspaceContent* singleContents_ = nullptr;
	QVBoxLayout* layout_ = nullptr;

	Workspace* parentWorkspace_;

	float size_;
	std::unique_ptr<std::string> contentsName_;

	void rebuildMenu();
	void enlarge();
	void clearContents();
	void initialiseWithOneWidget(WorkspaceContent* containingWidget = nullptr);
	void finishAddingWidget();

public:
	void serialisation() override;

	friend class WorkspaceContent;
};

class WorkspaceChooser : public WorkspaceContent {
	void serialisation() override;
	QVBoxLayout* layout_;
public:
	WorkspaceChooser(Workspace* parent = nullptr);
};

int registerWorkspaceConstructor(const std::string &name, const std::function<WorkspaceContent*(Workspace*)>& constructor);

template <typename C>
int registerWorkspaceConstructor(const std::string& name) {
  return registerWorkspaceConstructor(name, [] (Workspace* parent) {
	return new C(parent);
  });
}

#define REGISTER_WORKSPACE_TYPE(CLASS, NAME) namespace WorkspaceTools { const int WORKSPACE_REGISTERED_##CLASS_INDEX = registerWorkspaceConstructor<CLASS>(NAME); }

std::vector<std::string> getWorkspaces();
WorkspaceContent* constructWorkspace(const std::string& name, Workspace* parent = nullptr);

#endif // WORKSPACE_H

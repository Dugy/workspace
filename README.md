# Workspace
_Workspace_ is a tool for allowing users to modify UIs composed of predefined modules. It allows the user to split the window into any number of pieces of any size and fill them with blocks of his choice. The blocks can be defined by inheriting from a class. It uses Qt Widgets to provide widgets.

## Usage

_Workspace_ itself is a class inheriting from QWidget, so it can be added into a Qt window:

```C++
#include "main_window.h"
#include "ui_mainwindow.h"
#include "workspace.h"

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	ui->mainLayout->addWidget(workspace_ = new Workspace());
	workspace_->load("saved_layout.json");
}

MainWindow::~MainWindow()
{
	workspace_->save("saved_layout.json");
	delete ui;
}
```

The `mainwindow` and `main.cpp` provided can be used to create a window containing _workspace_ only, so you can use them if all you need is adding widgets.

New user-selectable widgets can be added by inheriting from class `WorkspaceContent` and registering them through the `REGISTER_WORKSPACE_TYPE` macro or the `registerWorkspaceConstructor` function. The macro has the advantage that it adds the widget without including anything in other parts of the code, all it needs is to be somewhere outside a function or class in a file linked with the executable.

```C++
#include <QVBoxLayout>
#include <QLabel>
#include "workspace.h"

class DummyWidget : public WorkspaceContent {
public:
	DummyWidget(Workspace* parent = nullptr) : WorkspaceContent(parent) {
		QVBoxLayout* layout = new QVBoxLayout(this);
		setLayout(layout);
		QLabel* title = new QLabel("Le English est prettie", this);
		title->setFont(QFont("Times", 20, QFont::Bold));
		layout->addWidget(title);
		QLabel* text = new QLabel("As je listene to le beautifulle sound de English language, "
				"ma mind est filled with awe et inspiration. Thise inspiration makes "
				"moi find inspiratives rhymes et poetrie that could make nice "
				"death metal songs.", this);
		text->setWordWrap(true);
		layout->addWidget(text);
	}
private:
	void serialisation() override {

	}
};
REGISTER_WORKSPACE_TYPE(DummyWidget, "Dummy Widget");
```

## WIP status

There may be bugs or design oversights. If you find any, please fill an issue.

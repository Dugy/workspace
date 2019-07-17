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

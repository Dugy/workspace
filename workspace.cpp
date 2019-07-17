#include "workspace.h"
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QSplitter>
#include <QVBoxLayout>
#include <QPushButton>

#include <iostream>

WorkspaceContent::WorkspaceContent(Workspace* parentWorkspace) : QWidget(parentWorkspace), parentWorkspace_(parentWorkspace) {
}

void WorkspaceContent::setParentWorkspace(Workspace* parentWorkspace) {
	parentWorkspace_ = parentWorkspace;
}

QMenuBar* WorkspaceContent::parentMenu() {
	return parentWorkspace_->menu_;
}

Workspace::Workspace(SplitOrientation orientation, WorkspaceContent* containingWidget, Workspace* parentWorkspace, QWidget* parent)
	: QFrame(parent), orientation_(orientation), parentWorkspace_(parentWorkspace)
{
	setFrameStyle( QFrame::Box | QFrame::Raised);
	setLineWidth(1);

	layout_ = new QVBoxLayout();
	layout_->setContentsMargins(0, 30, 0, 0);
	setLayout(layout_);

	initialiseWithOneWidget(containingWidget);

	rebuildMenu();
}

void Workspace::serialisation() {
	if (!saving()) {
		clearContents();
	}

	std::unique_ptr<std::vector<std::unique_ptr<Workspace>>> manyContentDescriptors;
	std::unique_ptr<bool> directionDescriptor;
	std::function<void()> cleanup = [&manyContentDescriptors] () {
		if (manyContentDescriptors)
			for (std::unique_ptr<Workspace>& it : *manyContentDescriptors)
				it.release();
	};

	try {
		synch("size", size_);

		if (saving() && orientation_ != SplitOrientation::UNSPECIFIED)
			directionDescriptor = std::make_unique<bool>((orientation_ == SplitOrientation::HORIZONTAL) ? true : false);
		synch("horizontal", directionDescriptor);
		if (!saving()) {
			if (!directionDescriptor) orientation_ = SplitOrientation::UNSPECIFIED;
			else orientation_ = (*directionDescriptor) ? SplitOrientation::HORIZONTAL : SplitOrientation::VERTICAL;
		}

		if (saving() && splitterContents_) {
			manyContentDescriptors = std::make_unique<std::vector<std::unique_ptr<Workspace>>>();
			for (int i = 0; i < splitterContents_->count(); i++) {
				manyContentDescriptors->emplace_back(dynamic_cast<Workspace*>(splitterContents_->widget(i)));
			}
			QList<int> sizes = splitterContents_->sizes();
			for (int i = 0; i < sizes.size(); i++) {
				(*manyContentDescriptors)[i]->size_ = sizes[i];
			}
		}
		synch("widgets", manyContentDescriptors);
		if (!saving() && manyContentDescriptors) {
			splitterContents_ = new QSplitter((orientation_ == SplitOrientation::VERTICAL) ? Qt::Orientation::Vertical : Qt::Orientation::Horizontal);
			layout_->addWidget(splitterContents_);
			int totalStretchFactor = 0;
			for (unsigned int i = 0; i < manyContentDescriptors->size(); i++)
				totalStretchFactor += (*manyContentDescriptors)[i]->size_;
			for (unsigned int i = 0; i < manyContentDescriptors->size(); i++) {
				int size = (*manyContentDescriptors)[i]->size_;
				(*manyContentDescriptors)[i]->setParentWorkspace(this);
				splitterContents_->addWidget((*manyContentDescriptors)[i].release());
				splitterContents_->setStretchFactor(i, size / totalStretchFactor);
			}
		}

		if (!manyContentDescriptors) {
			synch("type", contentsName_);
			if (!saving()) {
				if (contentsName_)
					initialiseWithOneWidget(constructWorkspace(*contentsName_, this));
				else
					initialiseWithOneWidget(new WorkspaceChooser(this));
			}
			if (singleContents_) {
				synch("widget", *singleContents_);
			}
		}
	} catch (std::exception& e) {
		cleanup();
		throw(e);
	}
	if (saving()) {
		cleanup();
	} else {
		rebuildMenu();
	}
}

void Workspace::clearContents() {
	contentsName_.reset();
	if (singleContents_) {
		delete singleContents_;
		singleContents_ = nullptr;
	}
	if (splitterContents_) {
		delete splitterContents_;
		splitterContents_ = nullptr;
	}
}

void  Workspace::rebuildMenu() {
	if (menu_) delete menu_;
	menu_ = new QMenuBar(this);
	menu_->setMinimumWidth(150);
	layoutMenu_ = menu_->addMenu("Layout");
	addWidgetRight_ = (orientation_ != SplitOrientation::VERTICAL) ? layoutMenu_->addAction("Add widget right") : nullptr;
	addWidgetDown_ = (orientation_ != SplitOrientation::HORIZONTAL) ? layoutMenu_->addAction("Add widget down") : nullptr;

	if (addWidgetRight_) connect(addWidgetRight_, &QAction::triggered, this, &Workspace::addWidgetHorizontal);
	if (addWidgetDown_) connect(addWidgetDown_, &QAction::triggered, this, &Workspace::addWidgetVertical);

	changeContentMenu_ = menu_->addMenu("Change");
	replaceContentAction_ = changeContentMenu_->addAction("Replace");
	connect(replaceContentAction_, &QAction::triggered, this, [this] () {
		clearContents();
		initialiseWithOneWidget();
	});

	if (parentWorkspace_) {
		removeContentAction_ = changeContentMenu_->addAction("Remove");
		connect(removeContentAction_, &QAction::triggered, this, &Workspace::removeSelf);
	}
}

void Workspace::setParentWorkspace(Workspace* parentWorkspace) {
	parentWorkspace_ = parentWorkspace;
	rebuildMenu();
}

void Workspace::initialiseWithOneWidget(WorkspaceContent* containingWidget) {
	singleContents_ = containingWidget ? containingWidget : new WorkspaceChooser(this);
	singleContents_->parentWorkspace_ = this;
	layout_->addWidget(singleContents_);
	splitterContents_ = nullptr;
}

void Workspace::addWidgetHorizontal() {
	if (!addWidgetRight_) return;
	else if (addWidgetDown_) addWidgetDown_->setEnabled(false);
	orientation_ = SplitOrientation::HORIZONTAL;
	finishAddingWidget();
}

void Workspace::addWidgetVertical() {
	if (!addWidgetDown_) return;
	else if (addWidgetRight_) addWidgetRight_->setEnabled(false);
	orientation_ = SplitOrientation::VERTICAL;
	finishAddingWidget();
}

void Workspace::finishAddingWidget() {
	if (!splitterContents_) enlarge();
	Workspace* subworkspace = new Workspace((orientation_ == SplitOrientation::VERTICAL) ? SplitOrientation::HORIZONTAL : SplitOrientation::VERTICAL, new WorkspaceChooser(this), this, this);
	splitterContents_->addWidget(subworkspace);
}

void Workspace::replaceWidget(WorkspaceContent* former, const std::string* name, WorkspaceContent* newContent) {
	if (singleContents_) {
		layout_->removeWidget(singleContents_);
		newContent->parentWorkspace_ = this;
		singleContents_ = newContent;
		layout_->addWidget(singleContents_);
		contentsName_ = std::make_unique<std::string>(*name);
	} else if (splitterContents_) {
		int index = -1;
		for (int i = 0; i < splitterContents_->count(); i++) {
			if (splitterContents_->widget(i) == former->parentWorkspace_) {
				index = i;
				break;
			}
		}
		if (index == -1) throw(std::logic_error("Replacing widget that is not a child"));
		newContent->parentWorkspace_ = this;
		splitterContents_->replaceWidget(index, newContent);
	} else throw(std::logic_error("Neither singleContents nor splitterContents?"));
}

void Workspace::removeSelf() {
	if (parentWorkspace_->singleContents_) parentWorkspace_->replaceWidget(nullptr, nullptr, new WorkspaceChooser(parentWorkspace_));
	parentWorkspace_->contentsName_.reset();
	deleteLater();
}

void Workspace::enlarge() {
	splitterContents_ = new QSplitter((orientation_ == SplitOrientation::VERTICAL) ? Qt::Orientation::Vertical : Qt::Orientation::Horizontal);
	Workspace* subworkspace = new Workspace((orientation_ == SplitOrientation::VERTICAL) ? SplitOrientation::HORIZONTAL : SplitOrientation::VERTICAL, singleContents_, this, this);
	subworkspace->contentsName_ = std::move(contentsName_);
	singleContents_->parentWorkspace_ = subworkspace;
	splitterContents_->addWidget(subworkspace);
	layout_->addWidget(splitterContents_);
	singleContents_ = nullptr;
}

WorkspaceChooser::WorkspaceChooser(Workspace* parent) : WorkspaceContent(parent) {
	setLayout(layout_ = new QVBoxLayout(this));
	std::vector<std::string> available = getWorkspaces();
	for (auto& name : available) {
		QPushButton* button = new QPushButton(QString::fromStdString(name), this);
		connect(button, &QPushButton::clicked, this, [name, this] () {
			parentWorkspace_->replaceWidget(this, &name, constructWorkspace(name, parentWorkspace_));
			deleteLater();
		});
		layout_->addWidget(button);
	}
	layout_->addStretch(1);
}

void WorkspaceChooser::serialisation() {

}

class WorkspaceWidgetRegister {
	std::unordered_map<std::string, std::function<WorkspaceContent*(Workspace*)>> constructors_;
	WorkspaceContent* make(const std::string& name, Workspace* parent = nullptr) {
		auto found = constructors_.find(name);
		if (found == constructors_.end()) throw(std::runtime_error("Invalid class " + name));
		return found->second(parent);
	}
	static WorkspaceWidgetRegister& get() {
		static WorkspaceWidgetRegister instance;
		return instance;
	}
	friend int registerWorkspaceConstructor(const std::string&, const std::function<WorkspaceContent*(Workspace*)>&);
	friend WorkspaceContent* constructWorkspace(const std::string&, Workspace*);
	friend std::vector<std::string> getWorkspaces();
};

int registerWorkspaceConstructor(const std::string &name, const std::function<WorkspaceContent*(Workspace*)>& constructor) {
	WorkspaceWidgetRegister::get().constructors_[name] = constructor;
	return WorkspaceWidgetRegister::get().constructors_.size();
}

std::vector<std::string> getWorkspaces() {
	std::vector<std::string> returned;
	for (auto& it : WorkspaceWidgetRegister::get().constructors_) {
		returned.push_back(it.first);
	}
	return returned;
}

WorkspaceContent* constructWorkspace(const std::string& name, Workspace* parent) {
	return WorkspaceWidgetRegister::get().make(name, parent);
}

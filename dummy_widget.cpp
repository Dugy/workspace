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
		QLabel* text = new QLabel("As je listene to le beautifulle sound de English language, ma mind est filled with awe et inspiration."
								  "Thise inspiration makes moi find inspiratives rhymes et poetrie that could make nice death metal songs.", this);
		text->setWordWrap(true);
		layout->addWidget(text);
	}
private:
	void serialisation() override {

	}
};
REGISTER_WORKSPACE_TYPE(DummyWidget, "Dummy Widget");

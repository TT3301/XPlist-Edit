#include "myapp.h"
#include "mainwindow.h"

extern QString fileName;
extern MainWindow* mw_one;

bool MyApplication::event(QEvent* event)
{
    if (event->type() == QEvent::FileOpen) {
        QFileOpenEvent* openEvent = static_cast<QFileOpenEvent*>(event);
        fileName = openEvent->file();

        if (mw_one) {
            mw_one->openPlist(fileName);
            mw_one->show();
            mw_one->raise();
            mw_one->activateWindow();
            fileName.clear();
        }
    }

    return QApplication::event(event);
}

void MyApplication::new_win()
{
    if (!fileName.isEmpty()) {
        mw_one = new MainWindow();
        mw_one->openPlist(fileName);
        mw_one->show();
    }
}

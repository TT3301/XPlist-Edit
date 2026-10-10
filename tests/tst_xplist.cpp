#include <QtTest>
#include <QFileDialog>
#include <QTimer>
#include <QTranslator>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "plistio.h"
#include "myapp.h"

MainWindow* mw_one = nullptr;
extern EditorTabsWidget* tabWidget;
extern QSettings Reg;

class XplistTests : public QObject {
    Q_OBJECT
    QString fixtures;
    MainWindow* window = nullptr;
    QByteArray bytes(const QString& path) {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly)) return {};
        return file.readAll();
    }
    QModelIndex findKey(DomModel* model, const QString& name) {
        const auto root = model->index(0, 0);
        for (int row = 0; row < model->rowCount(root); ++row) {
            const auto index = model->index(row, 0, root);
            if (index.data().toString() == name) return index;
        }
        return {};
    }
    bool save() {
        bool result = false;
        if (!QMetaObject::invokeMethod(window, "actionSave", Qt::DirectConnection,
                                       Q_RETURN_ARG(bool, result))) qFatal("Save slot unavailable");
        return result;
    }
    void closeTab(int index) {
        if (!QMetaObject::invokeMethod(window, "on_TabCloseRequest", Qt::DirectConnection,
                                      Q_ARG(int, index))) qFatal("Close slot unavailable");
    }
    void dismissWarning() {
        QTimer::singleShot(0, [] {
            auto box = qobject_cast<QMessageBox*>(QApplication::activeModalWidget());
            if (!box) qFatal("Expected a warning dialog");
            box->done(QMessageBox::Ok);
        });
    }
private slots:
    void initTestCase() {
        fixtures = qEnvironmentVariable("XPLIST_FIXTURES");
        QVERIFY(!fixtures.isEmpty());
        const QString home = qEnvironmentVariable("XPLIST_TEST_HOME");
        QSettings::setDefaultFormat(QSettings::IniFormat);
        QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, home + "/settings");
        window = new MainWindow;
        mw_one = window;
        window->show();
        QVERIFY(!window->ui->actionCheck_Update->isEnabled());
        QVERIFY(!window->ui->actionAutoUpdateCheck->isEnabled());
        QVERIFY(!window->ui->actionAutoUpdateCheck->isChecked());
        QVERIFY(QIcon(":/new/toolbar/res/save.svg").isNull() == false);
        QTranslator translator;
        QVERIFY(translator.load(":/new/toolbar/res/translation_cn.qm"));
        qApp->installTranslator(&translator);
        QVERIFY(QCoreApplication::translate("MainWindow", "Save") != "Save");
        qApp->removeTranslator(&translator);
    }
    void cleanup() {
        while (tabWidget->count()) {
            tabWidget->getCurentTab()->getUndoStack()->setClean();
            closeTab(tabWidget->count() - 1);
        }
    }
    void roundTrips_data() {
        QTest::addColumn<QString>("name");
        for (const QString& name : {QString::fromUtf8("类型样例 XML"), QString::fromUtf8("类型样例 BIN"),
                                   QString::fromUtf8("根数组 XML"), QString::fromUtf8("根数组 BIN")})
            QTest::newRow(name.toUtf8().constData()) << name;
    }
    void roundTrips() {
        QFETCH(QString, name);
        window->openPlist(fixtures + "/" + name + ".plist");
        QCOMPARE(tabWidget->count(), 1);
        auto tab = tabWidget->getCurentTab();
        if (name.startsWith(QString::fromUtf8("类型样例"))) {
            QVERIFY(findKey(tab->getModel(), "Item key").isValid());
            QVERIFY(findKey(tab->getModel(), "plist").isValid());
        }
        for (int format = 0; format < 2; ++format) {
            const QString target = fixtures + "/" + name + "-saved-" + (format ? "bin" : "xml") + ".plist";
            tab->setPath(target);
            window->ui->cboxFileType->setCurrentIndex(format);
            QVERIFY(save());
            QVERIFY(tab->getUndoStack()->isClean());
            const auto saved = bytes(target);
            QVERIFY(!saved.isEmpty());
            QCOMPARE(saved.startsWith("bplist00"), format == 1);
            const QByteArray expected = DomParser::toDom(tab->getModel()).toByteArray();
            window->openPlist(target);
            QCOMPARE(tabWidget->count(), 1);
            tab = tabWidget->getCurentTab();
            QCOMPARE(DomParser::toDom(tab->getModel()).toByteArray(), expected);
        }
    }
    void editsAndHistory() {
        window->openPlist(fixtures + QString::fromUtf8("/类型样例 XML.plist"));
        auto tab = tabWidget->getCurentTab();
        auto model = tab->getModel();
        const auto key = findKey(model, QString::fromUtf8("标题"));
        QVERIFY(key.isValid());
        const auto value = model->index(key.row(), 2, key.parent());
        const QString original = value.data().toString();
        tab->editorDataAboutToBeSet(value, QString::fromUtf8("已修改的中文"));
        QCOMPARE(value.data().toString(), QString::fromUtf8("已修改的中文"));
        QVERIFY(!tab->getUndoStack()->isClean());
        window->actionUndo->trigger();
        QCOMPARE(value.data().toString(), original);
        window->actionRedo->trigger();
        QCOMPARE(value.data().toString(), QString::fromUtf8("已修改的中文"));
        const auto root = model->index(0, 0);
        const int count = model->rowCount(root);
        tab->treeView->setCurrentIndex(root);
        window->actionAdd_activated();
        QCOMPARE(model->rowCount(root), count + 1);
        window->actionUndo->trigger();
        QCOMPARE(model->rowCount(root), count);
        window->actionRedo->trigger();
        QCOMPARE(model->rowCount(root), count + 1);
        tab->treeView->setCurrentIndex(model->index(count, 0, root));
        window->actionRemove_activated();
        QCOMPARE(model->rowCount(root), count);
        window->actionUndo->trigger();
        QCOMPARE(model->rowCount(root), count + 1);
        window->actionRedo->trigger();
        QCOMPARE(model->rowCount(root), count);
        tab->treeView->selectionModel()->clearSelection();
        tab->treeView->selectionModel()->select(key, QItemSelectionModel::Select | QItemSelectionModel::Rows);
        tab->treeView->setCurrentIndex(key);
        tab->on_copyAction();
        tab->on_pasteAction();
        QCOMPARE(model->rowCount(root), count + 1);
        window->actionUndo->trigger();
        QCOMPARE(model->rowCount(root), count);
        window->ui->editFind->setText(QString::fromUtf8("已修改"));
        window->on_Find();
        QVERIFY(window->ui->listFind->count() > 0);
        tab->setPath(fixtures + QString::fromUtf8("/类型样例 XML-edited.plist"));
        window->ui->cboxFileType->setCurrentIndex(0);
        QVERIFY(save());
        QVERIFY(tab->getUndoStack()->isClean());
        QVERIFY(tab->getUndoStack()->canUndo());
        window->actionUndo->trigger();
        QVERIFY(!tab->getUndoStack()->isClean());
        window->actionRedo->trigger();
        QVERIFY(tab->getUndoStack()->isClean());
        tab->treeView->expandAll();
        window->resize(1050, 740);
        QVERIFY(window->grab().save(fixtures + "/application.png"));
    }
    void atomicFailures() {
        const QString target = fixtures + "/protected.plist";
        QFile::remove(target);
        QVERIFY(QFile::copy(fixtures + QString::fromUtf8("/类型样例 XML.plist"), target));
        const auto original = bytes(target);
        QString error;
        QVERIFY(!PlistIO::save(target, "broken xml", true, &error));
        QVERIFY(!error.isEmpty());
        QCOMPARE(bytes(target), original);
        QVERIFY(!PlistIO::save(fixtures + "/missing/target.plist", original, false, &error));
        QVERIFY(!QFile::exists(fixtures + "/missing/target.plist"));
        QVERIFY(QFile::setPermissions(target, QFileDevice::ReadOwner));
        QVERIFY(!PlistIO::save(target, original, true, &error));
        QCOMPARE(bytes(target), original);
        window->openPlist(target);
        auto tab = tabWidget->getCurentTab();
        auto model = tab->getModel();
        auto key = findKey(model, QString::fromUtf8("标题"));
        tab->editorDataAboutToBeSet(model->index(key.row(), 2, key.parent()), "unsaved");
        dismissWarning();
        QVERIFY(!save());
        QVERIFY(!tab->getUndoStack()->isClean());
        QCOMPARE(bytes(target), original);
        int dialogs = 0;
        QTimer responder;
        connect(&responder, &QTimer::timeout, [&] {
            auto box = qobject_cast<QMessageBox*>(QApplication::activeModalWidget());
            if (!box) return;
            ++dialogs;
            box->done(box->standardButtons().testFlag(QMessageBox::Save) ? QMessageBox::Save : QMessageBox::Ok);
        });
        responder.start(10);
        closeTab(0);
        responder.stop();
        QCOMPARE(dialogs, 2);
        QCOMPARE(tabWidget->count(), 1);
        QCOMPARE(window->close_flag, 0);
        QCOMPARE(bytes(target), original);
        QVERIFY(QFile::setPermissions(target, QFileDevice::ReadOwner | QFileDevice::WriteOwner));
    }
    void invalidInputAndMultipleTabs() {
        window->openPlist(fixtures + QString::fromUtf8("/类型样例 XML.plist"));
        window->openPlist(fixtures + QString::fromUtf8("/根数组 BIN.plist"));
        QCOMPARE(tabWidget->count(), 2);
        const int current = tabWidget->currentIndex();
        const auto original = bytes(fixtures + QString::fromUtf8("/无效.plist"));
        dismissWarning();
        window->openPlist(fixtures + QString::fromUtf8("/无效.plist"));
        QCOMPARE(tabWidget->count(), 2);
        QCOMPARE(tabWidget->currentIndex(), current);
        QCOMPARE(bytes(fixtures + QString::fromUtf8("/无效.plist")), original);
        QFileOpenEvent event(fixtures + QString::fromUtf8("/类型样例 BIN.plist"));
        QCoreApplication::sendEvent(qApp, &event);
        QCOMPARE(tabWidget->count(), 3);
        QCOMPARE(tabWidget->getCurentTab()->getPath(), fixtures + QString::fromUtf8("/类型样例 BIN.plist"));
    }
    void unsavedCloseCancelAndDiscard() {
        window->openPlist(fixtures + QString::fromUtf8("/类型样例 XML.plist"));
        auto tab = tabWidget->getCurentTab();
        tab->treeView->setCurrentIndex(tab->getModel()->index(0, 0));
        window->actionAdd_activated();
        QTimer::singleShot(0, [] {
            auto box = qobject_cast<QMessageBox*>(QApplication::activeModalWidget());
            if (!box) qFatal("Expected unsaved changes prompt");
            box->done(QMessageBox::Cancel);
        });
        closeTab(0);
        QCOMPARE(tabWidget->count(), 1);
        QTimer::singleShot(0, [] {
            auto box = qobject_cast<QMessageBox*>(QApplication::activeModalWidget());
            if (!box) qFatal("Expected unsaved changes prompt");
            box->done(QMessageBox::Discard);
        });
        closeTab(0);
        QCOMPARE(tabWidget->count(), 0);
    }
    void fileDialogResources() {
        QFileDialog dialog(window);
        dialog.setDirectory(fixtures);
        dialog.setNameFilter("Property list (*.plist)");
        QCOMPARE(dialog.directory().absolutePath(), QFileInfo(fixtures).absoluteFilePath());
        QVERIFY(QFileInfo(dialog.directory().filePath(QString::fromUtf8("类型样例 XML.plist"))).exists());
    }
    void sessionRestoration() {
        const QString path = fixtures + QString::fromUtf8("/类型样例 XML.plist");
        Reg.setValue("restore", true);
        Reg.setValue("count", 1);
        Reg.setValue("0/file", path);
        Reg.setValue("index", 0);
        delete window;
        mw_one = nullptr;
        window = new MainWindow;
        mw_one = window;
        window->show();
        QCOMPARE(tabWidget->count(), 0);
        // Changes after construction must not alter the queued restoration snapshot.
        Reg.setValue("count", 0);
        QTRY_COMPARE(tabWidget->count(), 1);
        QCOMPARE(tabWidget->getCurentTab()->getPath(), path);
    }
    void cleanupTestCase() {
        delete window;
        mw_one = nullptr;
    }
};
int main(int argc, char** argv) {
    MyApplication app(argc, argv);
    XplistTests tests;
    return QTest::qExec(&tests, argc, argv);
}
#include "tst_xplist.moc"

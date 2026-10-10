#ifndef QTCOMPAT_H
#define QTCOMPAT_H
#include <QTextStream>
#include <QDir>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QStringConverter>
#endif
inline void setUtf8(QTextStream& stream) {
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    stream.setEncoding(QStringConverter::Utf8);
#else
    stream.setCodec("UTF-8");
#endif
}
inline QString xplistHomePath() {
#ifdef XPLIST_TESTS
    const QString path = qEnvironmentVariable("XPLIST_TEST_HOME");
    if (path.isEmpty()) qFatal("XPLIST_TEST_HOME must be set for tests");
    return path;
#else
    return QDir::homePath();
#endif
}
#endif

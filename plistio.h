#ifndef PLISTIO_H
#define PLISTIO_H
#include <QByteArray>
#include <QString>
namespace PlistIO {
// macOS system plutil validates and normalizes the document before use.
bool readXml(const QString& path, QByteArray* xml, QString* error);
bool save(const QString& path, const QByteArray& xml, bool binary, QString* error);
}
#endif

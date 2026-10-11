#include "plistio.h"
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QSaveFile>
#include <QTemporaryDir>

namespace {
bool convert(const QString& source, const QString& target,
             const QString& format, QString* error) {
    QProcess process;
    process.start("/usr/bin/plutil", {"-convert", format, "-o", target, "--", source});
    if (!process.waitForStarted(5000) || !process.waitForFinished(30000)) {
        *error = process.errorString();
        process.kill();
        process.waitForFinished(5000);
        return false;
    }
    if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
        *error = QString::fromUtf8(process.readAllStandardError() +
                                  process.readAllStandardOutput()).trimmed();
        if (error->isEmpty()) *error = QStringLiteral("plutil conversion failed");
        return false;
    }
    return true;
}
}

bool PlistIO::readXml(const QString& path, QByteArray* xml, QString* error) {
    QTemporaryDir temp;
    if (!temp.isValid()) { *error = temp.errorString(); return false; }
    const QString target = temp.filePath("document.plist");
    if (!convert(QFileInfo(path).absoluteFilePath(), target, "xml1", error)) return false;
    QFile file(target);
    if (!file.open(QIODevice::ReadOnly)) { *error = file.errorString(); return false; }
    *xml = file.readAll();
    if (file.error() != QFileDevice::NoError) { *error = file.errorString(); return false; }
    return true;
}

bool PlistIO::save(const QString& path, const QByteArray& xml, bool binary, QString* error) {
    const QFileInfo target(path);
    const auto writeBits = QFileDevice::WriteOwner | QFileDevice::WriteGroup | QFileDevice::WriteOther;
    if (target.exists() && (!(target.permissions() & writeBits) || !target.isWritable())) {
        *error = QStringLiteral("The file is read-only.");
        return false;
    }
    QTemporaryDir temp;
    if (!temp.isValid()) { *error = temp.errorString(); return false; }
    QFile source(temp.filePath("source.plist"));
    if (!source.open(QIODevice::WriteOnly) || source.write(xml) != xml.size() || !source.flush()) {
        *error = source.errorString();
        return false;
    }
    source.close();
    const QString converted = temp.filePath("converted.plist");
    if (!convert(source.fileName(), converted, binary ? "binary1" : "xml1", error)) return false;
    QFile data(converted);
    if (!data.open(QIODevice::ReadOnly)) { *error = data.errorString(); return false; }
    const QByteArray bytes = data.readAll();
    if (data.error() != QFileDevice::NoError) { *error = data.errorString(); return false; }
    QSaveFile output(target.absoluteFilePath());
    // Never fall back to truncating the original file if atomic replacement fails.
    output.setDirectWriteFallback(false);
    if (!output.open(QIODevice::WriteOnly) || output.write(bytes) != bytes.size() || !output.commit()) {
        *error = output.errorString();
        return false;
    }
    return true;
}

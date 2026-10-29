#pragma once
#include <QString>
#include <QByteArray>
namespace DocumentIO {
constexpr qint64 MaxBytes = 20 * 1024 * 1024;
bool decode(const QByteArray &bytes, QString &text, QString &error);
bool read(const QString &path, QString &text, QString &error);
bool write(const QString &path, const QString &text, QString &error);
}

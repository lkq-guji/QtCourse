#include "documentio.h"
#include <QFile>
#include <QFileInfo>
#include <QSaveFile>
#include <QStringDecoder>
bool DocumentIO::decode(const QByteArray &bytes, QString &text, QString &error)
{
    error.clear();
    auto encoding = QStringConverter::Utf8;
    int skip = 0;
    if (bytes.startsWith(QByteArray::fromHex("0000feff")) || bytes.startsWith(QByteArray::fromHex("fffe0000"))) {
        error = "暂不支持 UTF-32，请先转换为 UTF-8。";return false;
    }
    if (bytes.startsWith(QByteArray::fromHex("efbbbf"))) skip = 3;
    else if (bytes.startsWith(QByteArray::fromHex("fffe"))) { encoding = QStringConverter::Utf16LE;skip = 2; }
    else if (bytes.startsWith(QByteArray::fromHex("feff"))) { encoding = QStringConverter::Utf16BE;skip = 2; }
    if (encoding != QStringConverter::Utf8 && (bytes.size() - skip) % 2 != 0) {
        error = "UTF-16文件不完整：字符数据必须为偶数字节。";return false;
    }
    QStringDecoder decoder(encoding, QStringConverter::Flag::Stateless);
    const QString decoded = decoder(QByteArrayView(bytes).sliced(skip));
    if (decoder.hasError()) { error = "文本编码无效。请使用 UTF-8，或带 BOM 的 UTF-16 文件。";return false; }
    for (const QChar ch : decoded) {
        if (ch.unicode() < 32 && ch != '\n' && ch != '\r' && ch != '\t') {
            error = "文件包含二进制或不支持的控制字符。";return false;
        }
    }
    text = decoded;text.replace("\r\n", "\n");text.replace('\r', '\n');return true;
}
bool DocumentIO::read(const QString &path, QString &text, QString &error)
{
    error.clear();QFile file(path);
    if (!QFileInfo(path).isFile() || !file.open(QIODevice::ReadOnly)) {
        error = "无法打开文件：" + path + "\n" + file.errorString();return false;
    }
    if (file.size() > MaxBytes) { error = "文件超过20 MiB，请使用适合大文件的编辑器。";return false; }
    const QByteArray bytes = file.read(MaxBytes + 1);
    if (file.error() != QFileDevice::NoError || bytes.size() > MaxBytes) { error = "读取文件失败或文件超过20 MiB。";return false; }
    return decode(bytes, text, error);
}
bool DocumentIO::write(const QString &path, const QString &text, QString &error)
{
    error.clear();
    if (path.isEmpty()) { error = "保存路径为空。";return false; }
    const QByteArray bytes = text.toUtf8();
    if (bytes.size() > MaxBytes) { error = "文本超过20 MiB，无法保存。";return false; }
    QSaveFile file(path);
    // 原子提交，写入失败时不破坏已有文件；不启用直接覆盖回退。
    if (!file.open(QIODevice::WriteOnly) || file.write(bytes) != bytes.size() || !file.commit()) {
        error = "保存失败：" + file.errorString();return false;
    }
    return true;
}

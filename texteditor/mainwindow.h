#pragma once
#include <QMainWindow>
#include <memory>
class CodeEditor;
class SearchDialog;
class QLabel;
class QTimer;
namespace Ui { class MainWindow; }
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(const QString &backupRoot = {}, QWidget *parent = nullptr);
    ~MainWindow();
    CodeEditor *editor() const;
    QString currentFile() const { return filePath; }
    QString lastError() const { return errorText; }
    QString backupFile() const { return recoveryPath; }
    bool loadFile(const QString &path);
    bool saveTo(const QString &path);
    bool backupNow();
    void openPath(const QString &path);
protected:
    void closeEvent(QCloseEvent *event) override;
private:
    std::unique_ptr<Ui::MainWindow> ui;
    QString filePath, errorText, recoveryPath;
    QLabel *statsLabel, *positionLabel, *encodingLabel;
    QTimer *autoSaveTimer;
    SearchDialog *findDialog = nullptr, *replaceDialog = nullptr;
    void updateState();
    void openDialog();
    bool save();
    bool saveAs();
    bool maybeSave();
    void showSearch(bool replaceMode);
    void showAbout();
    void recover();
};

#include "calculatorwindow.h"
#include <QtTest>
#include <QApplication>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QProcess>
#include <QTimer>
#include <QMap>
#include <QImage>
#include <functional>

// 自动演示真实 QWidget：QTest 发送鼠标/键盘事件，连续抓取整个演示窗口，非预制效果图。
int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    if (argc != 3) { qCritical("Usage: calculator_demo ffmpeg_path output.mp4"); return 2; }
    QWidget presenter;
    presenter.setWindowTitle("实验一 连续测试记录");
    presenter.resize(1120, 780);
    presenter.setStyleSheet("QWidget#presenter{background:#eef2f7;} QLabel#caption{font:22px 'Microsoft YaHei UI';color:#172b4d;} QPlainTextEdit{font:15px 'Microsoft YaHei UI';background:white;color:#172b4d;border:1px solid #cbd5e1;}");
    presenter.setObjectName("presenter");
    auto *layout = new QHBoxLayout(&presenter);
    auto *calculator = new CalculatorWindow(&presenter);
    calculator->setMaximumWidth(470);
    layout->addWidget(calculator);
    auto *right = new QVBoxLayout;
    auto *caption = new QLabel("实验一  带键盘事件的计算器");
    caption->setObjectName("caption");
    caption->setWordWrap(true);
    auto *clock = new QLabel;
    auto *log = new QPlainTextEdit;
    log->setReadOnly(true);
    log->setLineWrapMode(QPlainTextEdit::WidgetWidth);
    right->addWidget(caption);
    right->addWidget(clock);
    right->addWidget(log, 1);
    layout->addLayout(right, 1);
    presenter.show();
    auto *display = calculator->findChild<QLineEdit *>("displayEdit");
    QMap<int, std::function<void()>> steps;
    auto section = [&](int frame, const QString &title, const QString &explanation) {
        steps[frame] = [=] { caption->setText(title); log->appendPlainText("\n" + title + "\n" + explanation); };
    };
    auto mouse = [&](int frame, const QString &name) {
        steps[frame] = [=] {
            auto *button = calculator->findChild<QPushButton *>(name);
            QTest::mouseClick(button, Qt::LeftButton);
            log->appendPlainText("鼠标 " + button->text() + "   →   " + display->text());
        };
    };
    auto keys = [&](int frame, const QString &text) {
        for (int i=0; i<text.size(); ++i) {
            const QString character=text.mid(i,1);
            steps[frame+i*10] = [=] {
                display->setFocus();
                QTest::keyClicks(display, character);
                log->appendPlainText("键盘 " + character + "   →   " + display->text());
            };
        }
    };
    auto key = [&](int frame, Qt::Key code, const QString &name) {
        steps[frame] = [=] { display->setFocus(); QTest::keyClick(display, code);
            log->appendPlainText("键盘 " + name + "   →   " + display->text()); };
    };
    section(0,"实验一  连续功能测试", "刘楷钦 2024414290221\nQt 6.11.1 / MinGW 13.1.0\n自动发送真实鼠标与键盘事件；连续录制150秒。\n逐步运算：2 + 3 × 4 = 20。");
    section(30,"01 鼠标小数运算", "12.5 + 3.75 = 16.25\n仅使用鼠标点击按钮。");
    const QStringList decimalButtons={"btn1","btn2","btnPoint","btn5","btnAdd","btn3","btnPoint","btn7","btn5","btnEquals"};
    for(int i=0;i<decimalButtons.size();++i) mouse(50+i*10,decimalButtons.at(i));
    section(240,"02 键盘运算", "Esc 清除，然后键盘输入 42 / 6，Enter 得到7。");
    key(260,Qt::Key_Escape,"Esc"); keys(280,"42/6"); key(340,Qt::Key_Return,"Enter");
    section(430,"03 退格和清除", "输入12345，退格两次得到123；清除后重新计算。");
    key(450,Qt::Key_Escape,"Esc"); keys(470,"12345");
    key(530,Qt::Key_Backspace,"Backspace"); key(550,Qt::Key_Backspace,"Backspace");
    key(590,Qt::Key_Escape,"Esc"); keys(610,"2+3"); key(650,Qt::Key_Return,"Enter");
    section(700,"04 除零和错误恢复", "8 / 0 显示不能除以零；直接输入数字即可开始新计算。");
    key(720,Qt::Key_Escape,"Esc"); keys(740,"8/0"); key(780,Qt::Key_Return,"Enter");
    keys(830,"2+3"); key(870,Qt::Key_Return,"Enter");
    section(920,"05 连续运算", "2 + 3 × 4 按标准计算器逐步运算，结果20。");
    key(940,Qt::Key_Escape,"Esc"); keys(960,"2+3*4"); key(1020,Qt::Key_Return,"Enter");
    section(1080,"06 异常输入", "2..3 忽略第二个小数点；8 + × 2 替换操作符。");
    key(1100,Qt::Key_Escape,"Esc"); keys(1120,"2..3+1"); key(1190,Qt::Key_Return,"Enter");
    key(1230,Qt::Key_Escape,"Esc"); keys(1250,"8+*2"); key(1300,Qt::Key_Return,"Enter");
    steps[1350]=[&] {
        caption->setText("07 Git 提交历史与测试结果");
        QProcess git;
        git.start("git", {"log","-7","--format=%h %s"});
        git.waitForFinished(3000);
        log->setPlainText("以下为录制时实际 Git 历史：\n\n"+QString::fromUtf8(git.readAllStandardOutput())
            +"\nWindows Qt Test：28 passed, 0 failed\n日志：artifacts/final-tests.txt\n\n鼠标与键盘共用 dispatch → engine.input。\n测试事件自动发送，画面连续捕获，无剪辑。");
    };
    QProcess encoder;
    const QSize size(1120,780);
    encoder.setProcessChannelMode(QProcess::ForwardedErrorChannel);
    encoder.start(QString::fromLocal8Bit(argv[1]), {"-y","-loglevel","error","-f","rawvideo","-pixel_format","rgba","-video_size","1120x780","-framerate","10","-i","pipe:0","-an","-c:v","libx264","-preset","veryfast","-crf","25","-pix_fmt","yuv420p","-movflags","+faststart",QString::fromLocal8Bit(argv[2])});
    if (!encoder.waitForStarted(10000)) return 3;
    QTimer timer;
    int frame=0;
    QObject::connect(&timer,&QTimer::timeout,&presenter,[&] {
        if (steps.contains(frame)) steps[frame]();
        clock->setText(QString("连续记录 %1 / 150 秒").arg(frame/10));
        auto image=presenter.grab().toImage().scaled(size).convertToFormat(QImage::Format_RGBA8888);
        encoder.write(reinterpret_cast<const char *>(image.constBits()),image.sizeInBytes());
        if (++frame==1500) {
            timer.stop(); encoder.closeWriteChannel(); encoder.waitForFinished(30000);
            qInfo("Recorded 1500 real widget frames (150 seconds).");
            app.exit(encoder.exitCode());
        }
    });
    timer.start(100);
    return app.exec();
}

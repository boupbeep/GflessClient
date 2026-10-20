#include "mainwindow.h"
#include "processchecker.h"
#include "settingsdialog.h"

#include <QApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QMessageBox>
#include <QSettings>
#include <QSharedMemory>
#include <QTextStream>

void checkGameforgeClient();

static QFile logFile;

static void messageHandler(QtMsgType type, const QMessageLogContext &, const QString &msg)
{
    if (!logFile.isOpen())
        return;

    QString level;
    switch (type) {
        case QtDebugMsg:    level = "DEBUG"; break;
        case QtInfoMsg:     level = "INFO"; break;
        case QtWarningMsg:  level = "WARN"; break;
        case QtCriticalMsg: level = "ERROR"; break;
        case QtFatalMsg:    level = "FATAL"; break;
    }

    QTextStream out(&logFile);
    out << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz")
        << " [" << level << "] " << msg << "\n";
    out.flush();
}

int main(int argc, char *argv[])
{
    QString logPath = QDir::currentPath() + "/gfless.log";
    logFile.setFileName(logPath);

    // The install folder (e.g. Program Files) may not be writable without admin rights
    if (!logFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        logPath = QDir::tempPath() + "/gfless.log";
        logFile.setFileName(logPath);
        logFile.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text);
    }

    qInstallMessageHandler(messageHandler);
    qInfo() << "=== GflessClient v" APP_VERSION " started ===";
    qInfo() << "Log file:" << logPath;

    QApplication a(argc, argv);
    a.setProperty("logPath", logPath);

    // Check if another instance of the app is already running
    QSharedMemory sharedMemory("Gfless Client");
    if (!sharedMemory.create(1))
    {
        QMessageBox::critical(nullptr, "Error", "Gfless Client is already running.");
        return 0;
    }

    checkGameforgeClient();

    QCoreApplication::setOrganizationName("Hatz Nostale");
    QCoreApplication::setApplicationName("Gfless Client");

    QSettings settings;
    settings.beginGroup("MainWindow");
    int theme = settings.value("theme", ThemeDefault).toInt();
    settings.endGroup();
    SettingsDialog::applyTheme(theme);

    MainWindow w;
    w.show();

    return a.exec();
}

void checkGameforgeClient()
{
    const wchar_t *gameforgeClientName = L"gfclient.exe";

    if (isProcessRunning(gameforgeClientName))
    {
        int res = QMessageBox::critical(nullptr, "Error", "Seems like Gameforge Client is running.\nDo you want to kill the process?", QMessageBox::Yes | QMessageBox::No);

        if (res == QMessageBox::Yes)
            killProcess(gameforgeClientName);

        else
            exit(0);
    }
}

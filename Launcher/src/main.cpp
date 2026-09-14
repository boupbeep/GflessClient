#include "mainwindow.h"
#include "processchecker.h"
#include "settingsdialog.h"

#include <QApplication>
#include <QMessageBox>
#include <QSettings>
#include <QSharedMemory>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QMutex>
#include <QDateTime>
#include <QDebug>
#include <QStatusBar>

void checkGameforgeClient();

static QFile logFile;
static QTextStream logStream;
static QMutex logMutex;
static QtMessageHandler previousMessageHandler = nullptr;

static QString logFilePath()
{
    QString dir = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    QDir().mkpath(dir);
    return dir + "/gfless.log";
}

static void ensureLogFileOpen()
{
    if (logFile.isOpen()) {
        return;
    }

    logFile.setFileName(logFilePath());

    const qint64 maxSize = 5 * 1024 * 1024;

    if (logFile.size() > maxSize) {
        logFile.open(QIODevice::WriteOnly | QIODevice::Truncate);
        logFile.close();
    }

    logFile.open(QIODevice::Append | QIODevice::Text);
    logStream.setDevice(&logFile);
}

static void gflessMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    {
        QMutexLocker locker(&logMutex);

        ensureLogFileOpen();

        QString level;

        switch (type) {
            case QtDebugMsg:
                level = "DEBUG";
                break;
            case QtInfoMsg:
                level = "INFO";
                break;
            case QtWarningMsg:
                level = "WARN";
                break;
            case QtCriticalMsg:
            default:
                level = "CRIT";
                break;
        }

        logStream << "[" << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss.zzz") << "] "
                   << "[" << level << "] " << msg << "\n";
        logStream.flush();
    }

    if (previousMessageHandler) {
        previousMessageHandler(type, context, msg);
    }
}

int main(int argc, char *argv[])
{
    previousMessageHandler = qInstallMessageHandler(gflessMessageHandler);

    QApplication a(argc, argv);

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

    QString logPath = logFilePath();
    qDebug() << "log file:" << logPath;

    QSettings settings;
    settings.beginGroup("MainWindow");
    int theme = settings.value("theme", ThemeDefault).toInt();
    settings.endGroup();
    SettingsDialog::applyTheme(theme);

    MainWindow w;
    w.show();
    w.statusBar()->showMessage("Log file: " + logPath, 10000);

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

#include "gflessclient.h"
#include <QMessageBox>
#include <TlHelp32.h>

// The thread id returned by QProcess can't be trusted after startDetached (Qt frees its PROCESS_INFORMATION),
// so look up the main thread of the suspended process instead.
static DWORD findMainThreadId(DWORD pid)
{
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);

    if (snapshot == INVALID_HANDLE_VALUE)
        return 0;

    THREADENTRY32 entry;
    entry.dwSize = sizeof(entry);
    DWORD found = 0;

    if (Thread32First(snapshot, &entry)) {
        do {
            if (entry.th32OwnerProcessID == pid) {
                found = entry.th32ThreadID;
                break;
            }
        } while (Thread32Next(snapshot, &entry));
    }

    CloseHandle(snapshot);

    return found;
}

// Don't leave a suspended game process behind when the launch fails
static void killSuspendedProcess(DWORD pid)
{
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);

    if (hProcess == NULL)
        return;

    TerminateProcess(hProcess, 1);
    CloseHandle(hProcess);
}

GflessClient::GflessClient(QObject *parent) : QObject(parent)
{
    pipe = nullptr;
    gfServer = new QLocalServer(this);
    gfServer->listen("GameforgeClientJSONRPC");
    token = "";
    displayName = "";

    connect(gfServer, &QLocalServer::newConnection, this, &GflessClient::handleNewConnection);
}

bool GflessClient::openClient(const QString& displayName, const QString& token, const QString &gameClientPath, const int &gameLanguage, DWORD& createdPID)
{
    this->displayName = displayName;
    this->token = token;

    int index = gameClientPath.lastIndexOf("/");

    if (index < 0)
        return false;

    QString directory = gameClientPath.left(index);
    QProcess process;
    QProcessEnvironment env = process.processEnvironment();
    qint64 pid;

    env.insert("_TNT_CLIENT_APPLICATION_ID", "d3b2a0c1-f0d0-4888-ae0b-1c5e1febdafb");
    env.insert("_TNT_SESSION_ID", "12345678-1234-1234-1234-123456789012");

    process.setProcessEnvironment(env);
    process.setWorkingDirectory(directory);
    process.setProgram(gameClientPath);
    process.setArguments({"gf", QString::number(gameLanguage)});

    process.setCreateProcessArgumentsModifier([&](QProcess::CreateProcessArguments *args) {
        args->flags |= CREATE_SUSPENDED;
    });

    qInfo() << "[CLIENT] Creating process:" << gameClientPath;
    qInfo() << "[CLIENT] Session ID env:" << env.value("_TNT_SESSION_ID");

    if (!process.startDetached(&pid))
    {
        qWarning() << "[CLIENT] Error creating process:" << process.errorString();
        return false;
    }

    qInfo() << "[CLIENT] Process created, PID:" << pid;

    DWORD threadId = findMainThreadId(pid);

    qInfo() << "[CLIENT] Main thread ID:" << threadId;

    QString dllPath = QDir::currentPath() + "/GflessDLL.dll";

    qInfo() << "[CLIENT] Injecting DLL:" << dllPath;

    if (!injectDll(pid, dllPath))
        qDebug() << "Dll injection failed";
    else
        qDebug() << "Dll injected successfully";

    // Resume the game
    HANDLE hThread = threadId ? OpenThread(THREAD_SUSPEND_RESUME, FALSE, threadId) : NULL;

    if (hThread == NULL)
    {
        qWarning() << "[CLIENT] Error on OpenThread:" << GetLastError() << "(thread" << threadId << ")";
        killSuspendedProcess(pid);
        return false;
    }

    if (ResumeThread(hThread) == (DWORD)-1) {
        qWarning() << "[CLIENT] ResumeThread failed, error:" << GetLastError();
        CloseHandle(hThread);
        killSuspendedProcess(pid);
        return false;
    }

    CloseHandle(hThread);
    createdPID = pid;

    qInfo() << "[CLIENT] Game launched successfully";

    return true;
}

bool GflessClient::openClientSettings(const QString &gameClientPath)
{
    int index = gameClientPath.lastIndexOf("/NostaleClientX.exe");

    if (index < 0)
        return false;

    QString directory = gameClientPath.left(index);
    QString settingsPath = directory + "/NtConfig.exe";

    return QProcess::startDetached(settingsPath, {}, directory);
}

void GflessClient::handleNewConnection()
{
    pipe = gfServer->nextPendingConnection();
    connect(pipe, &QLocalSocket::readyRead, this, &GflessClient::handlePipe);
}

void GflessClient::handlePipe()
{
    QByteArray output;
    QJsonObject json = QJsonDocument::fromJson(pipe->readAll()).object();
    QString method = json["method"].toString();

    qDebug() << QJsonDocument(json).toJson();
    qInfo() << "[PIPE] Received:" << method;

    if (method == "ClientLibrary.isClientRunning")
        output = prepareResponse(json, "true");

    else if  (method == "ClientLibrary.initSession")
        output = prepareResponse(json, json["params"].toObject()["sessionId"].toString().toUtf8());

    else if (method == "ClientLibrary.queryAuthorizationCode") {
        qInfo() << "[PIPE] Responding to queryAuthorizationCode for" << displayName;
        output = prepareResponse(json, token.toUtf8());
    }

    else if (method == "ClientLibrary.queryGameAccountName")
        output = prepareResponse(json, displayName.toUtf8());

    else
        return;

    pipe->write(output);
}

QByteArray GflessClient::prepareResponse(const QJsonObject &request, const QString &response)
{
    QJsonObject resp;

    resp["id"] = request["id"];
    resp["jsonrpc"] = request["jsonrpc"];
    resp["result"] = response;

    return QJsonDocument(resp).toJson();
}

bool GflessClient::injectDll(DWORD pid, const QString &dllPath)
{
    QString injectorPath = QDir::currentPath() + "/Injector.exe";
    QStringList arguments;
    arguments << QString::number(pid) << dllPath;

    if (!QFile::exists(injectorPath)) {
        qWarning() << "[CLIENT] Injector.exe not found at" << injectorPath;
        QMessageBox::critical((QWidget*)this->parent(), "Error", "Injector.exe not found in the current directory.\nMake sure your antivirus has not deleted it");
        return false;
    }

    QProcess process;
    process.start(injectorPath, arguments);

    process.waitForFinished();

    qDebug() << process.readAllStandardOutput();

    if (process.exitCode())
        qWarning() << "[CLIENT] DLL injection failed, exit code:" << process.exitCode();

    return !process.exitCode();
}

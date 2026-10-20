#include "gflessclient.h"
#include <QMessageBox>

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

    PROCESS_INFORMATION* procInfo;
    DWORD threadId;

    process.setCreateProcessArgumentsModifier([&](QProcess::CreateProcessArguments *args) {
        args->flags |= CREATE_SUSPENDED;
        procInfo = args->processInformation;
    });

    qInfo() << "[CLIENT] Creating process:" << gameClientPath;
    qInfo() << "[CLIENT] Session ID env:" << env.value("_TNT_SESSION_ID");

    if (!process.startDetached(&pid))
    {
        qWarning() << "[CLIENT] Error creating process:" << process.errorString();
        return false;
    }

    qInfo() << "[CLIENT] Process created, PID:" << pid;

    threadId = procInfo->dwThreadId;

    QString dllPath = QDir::currentPath() + "/GflessDLL.dll";

    qInfo() << "[CLIENT] Injecting DLL:" << dllPath;

    if (!injectDll(pid, dllPath))
        qDebug() << "Dll injection failed";
    else
        qDebug() << "Dll injected successfully";

    // Resume the game
    HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, threadId);

    if (hThread == NULL)
    {
        qWarning() << "[CLIENT] Error on OpenThread:" << GetLastError();
        return false;
    }

    if (ResumeThread(hThread) == -1) {
        qWarning() << "[CLIENT] ResumeThread failed, error:" << GetLastError();
        CloseHandle(hThread);
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

#include "gameforgeaccount.h"

GameforgeAccount::GameforgeAccount(const QString &gfEmail,
    const QString &gfPassword,
    const QString& identPath, const QString &installationId,
    const QString& customGamePath,
    bool proxy,
    const QString &proxyHost,
    const QString &proxyPort,
    const QString &proxyUsername,
    const QString &proxyPassword, QObject *parent
)
    : QObject{parent}
    , email(gfEmail)
    , password(gfPassword)
    , identityPath(identPath)
    , customClientPath(customGamePath)
{
    auth = new NostaleAuth(
        identityPath,
        installationId,
        proxy,
        proxyHost,
        proxyPort,
        proxyUsername,
        proxyPassword,
        this
    );
}

bool GameforgeAccount::authenticate(bool &captcha, QString &gfChallengeId, bool &wrongCredentials)
{
    bool res = auth->authenticate(email, password, captcha, gfChallengeId, wrongCredentials);

    if (res) {
        updateGameAccounts();
    }

    return res;
}

bool GameforgeAccount::createGameAccount(const QString &name, const QString &gfLang, QJsonObject& response)
{
    return auth->createGameAccount(this->email, name, gfLang, response);
}

void GameforgeAccount::setToken(const QString &token)
{
    auth->setToken(token);
}

void GameforgeAccount::setEmail(const QString &email)
{
    this->email = email;
}

void GameforgeAccount::setPassword(const QString &password)
{
    this->password = password;
}

void GameforgeAccount::setIdentityPath(const QString &path)
{
    identityPath = path;
}

void GameforgeAccount::setCustomClientPath(const QString &path)
{
    customClientPath = path;
}

const QMap<QString, QString> &GameforgeAccount::getGameAccounts() const
{
    return gameAccounts;
}

void GameforgeAccount::updateGameAccounts()
{
    gameAccounts = auth->getAccounts();
}

QString GameforgeAccount::getToken(const QString &accountId) const
{
    return auth->getToken(accountId);
}

QString GameforgeAccount::getEmail() const
{
    return email;
}

QString GameforgeAccount::getPassword() const
{
    return password;
}

QString GameforgeAccount::getIdentityPath() const
{
    return identityPath;
}

NostaleAuth *GameforgeAccount::getAuth() const
{
    return auth;
}

void GameforgeAccount::setOtpCode(const QString &otpCode)
{
    auth->setOtpCode(otpCode);
}

bool GameforgeAccount::getOtpRequired() const
{
    return auth->getOtpRequired();
}

void GameforgeAccount::refreshIdentity()
{
    auth->refreshIdentity();
}

QString GameforgeAccount::getcustomClientPath() const
{
    return customClientPath;
}

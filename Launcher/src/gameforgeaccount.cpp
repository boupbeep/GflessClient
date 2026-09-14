#include "gameforgeaccount.h"
#include <QDebug>

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

const QMap<QString, QString> &GameforgeAccount::getGameAccounts() const
{
    return gameAccounts;
}

void GameforgeAccount::updateGameAccounts()
{
    gameAccounts = auth->getAccounts();
}

QString GameforgeAccount::getToken(const QString &accountId, TokenFailureReason *reason) const
{
    if (reason) {
        *reason = TokenFailureReason::None;
    }

    QString cur = auth->getEgressIp();
    QString mintIp = auth->getTokenIp();

    bool needReauth = auth->getToken().isEmpty()
        || mintIp.isEmpty()
        || (!cur.isEmpty() && cur != mintIp);

    if (!needReauth) {
        QString token = auth->getToken(accountId);

        if (token.isEmpty() && reason) {
            *reason = TokenFailureReason::Generic;
        }

        return token;
    }

    if (!auth->canReauthNow()) {
        qDebug() << "GameforgeAccount::getToken reauth suppressed (anti-flap) for account" << accountId;

        if (reason) {
            *reason = TokenFailureReason::AntiFlap;
        }

        return {};
    }

    if (email.isEmpty() || password.isEmpty()) {
        if (reason) {
            *reason = TokenFailureReason::ReauthFailed;
        }

        return {};
    }

    qDebug() << "GameforgeAccount::getToken egress IP changed (" << mintIp << "->" << cur
              << "), re-authenticating BEFORE play for account" << accountId;

    bool captcha = false;
    QString gfChallengeId;
    bool wrongCredentials = false;

    if (!auth->authenticate(email, password, captcha, gfChallengeId, wrongCredentials)) {
        qDebug() << "GameforgeAccount::getToken re-authentication failed for account" << accountId;

        if (reason) {
            *reason = TokenFailureReason::ReauthFailed;
        }

        return {};
    }

    QString token = auth->getToken(accountId);

    if (token.isEmpty() && reason) {
        *reason = TokenFailureReason::Generic;
    }

    return token;
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

const NostaleAuth *GameforgeAccount::getAuth() const
{
    return auth;
}

void GameforgeAccount::setOtpCode(const QString &otpCode)
{
    auth->setOtpCode(otpCode);
}

void GameforgeAccount::setTokenIp(const QString &ip)
{
    auth->setTokenIp(ip);
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

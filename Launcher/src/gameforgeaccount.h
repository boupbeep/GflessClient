#ifndef GAMEFORGEACCOUNT_H
#define GAMEFORGEACCOUNT_H

#include <QObject>
#include <QMap>
#include "nostaleauth.h"

class GameforgeAccount : public QObject
{
    Q_OBJECT
public:
    enum class TokenFailureReason {
        None,          // token obtained, or not applicable
        AntiFlap,      // re-auth needed but suppressed, IP changed too recently
        ReauthFailed,  // re-auth needed and attempted, but failed (captcha/OTP/wrong credentials, or missing credentials)
        Generic        // token empty for any other reason (e.g. iovation failed with a matching IP)
    };

    explicit GameforgeAccount(
        const QString& gfEmail,
        const QString& gfPassword,
        const QString& identPath,
        const QString& installationId,
        const QString& customGamePath,
        bool proxy,
        const QString& proxyHost,
        const QString& proxyPort,
        const QString& proxyUsername,
        const QString& proxyPassword,
        QObject *parent = nullptr
    );

    bool authenticate(bool& captcha, QString& gfChallengeId, bool& wrongCredentials);

    bool createGameAccount(const QString& name, const QString& gfLang, QJsonObject& response);

    void setToken(const QString& token);

    const QMap<QString, QString>& getGameAccounts() const;

    void updateGameAccounts();

    QString getToken(const QString& accountId, TokenFailureReason* reason = nullptr) const;
    QString getEmail() const;

    QString getPassword() const;

    QString getIdentityPath() const;

    const NostaleAuth *getAuth() const;

    QString getcustomClientPath() const;

    void setOtpCode(const QString& otpCode);

    void setTokenIp(const QString& ip);

    bool getOtpRequired() const;

    void refreshIdentity();

private:
    QString email;
    QString password;
    QString identityPath;
    QString customClientPath;
    NostaleAuth* auth;
    QMap<QString /* id */, QString /* name */> gameAccounts;
};

#endif // GAMEFORGEACCOUNT_H

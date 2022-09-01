//
// Created by g29624 on 2022/8/29.
//

#include <drogon/HttpAppFramework.h>
#include <format>
#include <plugins/EmailManager.h>
#include <structures/Exceptions.h>

using namespace drogon;
using namespace std;
using namespace trantor;
using namespace techmino::plugins;
using namespace techmino::structures;
using namespace techmino::types;

Email::Email(
        std::string account,
        std::string password,
        std::string senderEmail,
        std::string senderName,
        string receiverEmail,
        string subject,
        string content,
        bool isHTML,
        std::shared_ptr<trantor::TcpClient> socket
) : _account(std::move(account)),
    _password(std::move(password)),
    _senderEmail(std::move(senderEmail)),
    _senderName(std::move(senderName)),
    _receiverEmail(std::move(receiverEmail)),
    _subject(std::move(subject)),
    _content(std::move(content)),
    _isHTML(isHTML),
    _socket(std::move(socket)) {}

void messageHandler(
        const TcpConnectionPtr &connPtr,
        MsgBuffer *msgBuffer,
        const shared_ptr<Email> &email,
        const function<void(bool result, const string &)> &callback
) {
    std::string receivedMsg;
    while (msgBuffer->readableBytes() > 0) {
        string buf(msgBuffer->peek(), msgBuffer->readableBytes());
        receivedMsg.append(buf);
        msgBuffer->retrieveAll();
    }
    LOG_TRACE << "receive: " << receivedMsg;
    uint32_t responseCode = stoul(receivedMsg.substr(0, 3));

    if (email->_state == EmailState::Close) {
        /*Callback here for succeed delivery is probable*/
        callback(true, "EMail sent.");
        return;
    }
    try {
        switch (responseCode) {
            case 220:
                switch (email->_state) {
                    case EmailState::Init: {
                        string outMsg("EHLO smtpclient.qw\r\n");
                        MsgBuffer out;
                        out.append(outMsg.data(), outMsg.size());

                        connPtr->send(std::move(out));

                        email->_state = EmailState::HandShake;
                        break;
                    }
                    case EmailState::HandShake: {
                        string outMsg("EHLO smtpclient.qw\r\n");
                        MsgBuffer out;
                        out.append(outMsg.data(), outMsg.size());

                        connPtr->startClientEncryption([connPtr, out]() {
                            LOG_TRACE << "SSL established";
                            connPtr->send(out);
                        }, false, false);

                        email->_state = EmailState::Auth;
                        break;
                    }
                    default:
                        throw EmailException("Unsupported state");
                }
                break;
            case 235:
                switch (email->_state) {
                    case EmailState::Mail: {
                        string outMsg("MAIL FROM:<" + email->_senderEmail + ">\r\n");
                        MsgBuffer out;
                        out.append(outMsg.data(), outMsg.size());

                        connPtr->send(std::move(out));

                        email->_state = EmailState::Recipient;
                        break;
                    }
                    default:
                        throw EmailException("Unsupported state");
                }
                break;
            case 250:
                switch (email->_state) {
                    case EmailState::HandShake: {
                        string outMsg("STARTTLS\r\n");
                        MsgBuffer out;
                        out.append(outMsg.data(), outMsg.size());

                        connPtr->send(std::move(out));

                        email->_state = EmailState::HandShake;
                        break;
                    }
                    case EmailState::Auth: {
                        string outMsg("AUTH LOGIN\r\n");
                        MsgBuffer out;
                        out.append(outMsg.data(), outMsg.size());

                        connPtr->send(std::move(out));

                        email->_state = EmailState::User;
                        break;
                    }
                    case EmailState::Recipient: {
                        string outMsg("RCPT TO:<" + email->_receiverEmail + ">\r\n");
                        MsgBuffer out;
                        out.append(outMsg.data(), outMsg.size());

                        connPtr->send(std::move(out));

                        email->_state = EmailState::Data;
                        break;
                    }
                    case EmailState::Data: {
                        string outMsg("DATA\r\n");
                        MsgBuffer out;
                        out.append(outMsg.data(), outMsg.size());

                        connPtr->send(std::move(out));

                        email->_state = EmailState::Body;
                        break;
                    }
                    case EmailState::Quit: {
                        string outMsg("QUIT\r\n");
                        MsgBuffer out;
                        out.append(outMsg.data(), outMsg.size());

                        connPtr->send(std::move(out));

                        email->_state = EmailState::Close;
                        break;
                    }
                    default:
                        throw EmailException("Unsupported state");
                }
                break;
            case 334:
                switch (email->_state) {
                    case EmailState::User: {
                        string outMsg(drogon::utils::base64Encode(
                                reinterpret_cast<const unsigned char *>(email->_account.c_str()),
                                email->_account.length()
                        ) + "\r\n");
                        MsgBuffer out;
                        out.append(outMsg.data(), outMsg.size());

                        connPtr->send(std::move(out));

                        email->_state = EmailState::Pass;
                        break;
                    }
                    case EmailState::Pass: {
                        string outMsg(drogon::utils::base64Encode(
                                reinterpret_cast<const unsigned char *>(email->_password.c_str()),
                                email->_password.length()
                        ) + "\r\n");
                        MsgBuffer out;
                        out.append(outMsg.data(), outMsg.size());

                        connPtr->send(std::move(out));

                        email->_state = EmailState::Mail;
                        break;
                    }
                    default:
                        throw EmailException("Unsupported state");
                }
                break;
            case 354:
                switch (email->_state) {
                    case EmailState::Body: {
                        string outMsg(
                                "To: " + email->_receiverEmail + "\r\n" +
                                "From: " + email->_senderEmail + "\r\n"
                        );
                        if (email->_isHTML) {
                            outMsg.append("Content-Type: text/html;\r\n");
                        }
                        outMsg.append(
                                "Subject: " + email->_subject + "\r\n\r\n" +
                                email->_content + "\r\n.\r\n"
                        );
                        MsgBuffer out;
                        out.append(outMsg.data(), outMsg.size());

                        connPtr->send(std::move(out));

                        email->_state = EmailState::Quit;
                        break;
                    }
                    default:
                        throw EmailException("Unsupported state");
                }
                break;
                break;
            default:
                throw EmailException("Unsupported state");
        }
    } catch (const EmailException &e) {
        email->_state = EmailState::Close;
        callback(false, receivedMsg);
    }
}

void EmailManager::initAndStart(const Json::Value &config) {
    if (!(
            config["server"].isString() &&
            config["port"].isUInt() &&
            config["account"].isString() &&
            config["password"].isString() &&
            config["senderEmail"].isString() &&
            config["senderName"].isString()
    )) {
        LOG_ERROR << R"(Invalid config)";
        abort();
    }
    _server = config["server"].asString();
    _port = config["port"].asUInt();
    _account = config["account"].asString();
    _password = config["password"].asString();
    _senderEmail = config["senderEmail"].asString();
    _senderName = config["senderName"].asString();

    LOG_INFO << "EmailManager loaded.";
}

void EmailManager::shutdown() {
    LOG_INFO << "EmailManager shutdown.";
}

void EmailManager::smtp(
        const string &receiverEmail,
        const string &subject,
        const string &content,
        bool isHTML,
        const function<void(const string &)> &callback
) {
    const auto resolver = app().getResolver();
    resolver->resolve(_server, [=, this](const InetAddress &resolvedAddr) {
        const auto threadNum = app().getThreadNum();
        EventLoop *ioLoop;
        for (size_t threadIndex = 0; threadIndex < threadNum; ++threadIndex) {
            ioLoop = app().getIOLoop(threadIndex);
            if (ioLoop == nullptr || ioLoop->isInLoopThread()) {
                continue;
            } else {
                break;
            }
        }
        InetAddress smtpAddress(resolvedAddr.toIp(), _port, false);
        const auto emailUuid = drogon::utils::getUuid();
        const auto tcpSocket = make_shared<TcpClient>(ioLoop, smtpAddress, "SMTPMail");
        _emailMap.emplace(emailUuid, make_shared<Email>(
                _account,
                _password,
                _senderEmail,
                _senderName,
                receiverEmail,
                subject,
                content,
                isHTML,
                tcpSocket
        ));
        tcpSocket->setConnectionCallback([emailUuid](const TcpConnectionPtr &connPtr) {
            if (connPtr->connected()) {
                LOG_TRACE << "Connection established!";
            } else {
                LOG_TRACE << "Connection disconnect";
                _emailMap.erase(emailUuid);
            }
        });
        tcpSocket->setConnectionErrorCallback([emailUuid]() {
            LOG_ERROR << "Bad Server address";
            _emailMap.erase(emailUuid);
        });
        tcpSocket->setMessageCallback(
                [emailUuid, callback](const TcpConnectionPtr &connPtr, MsgBuffer *msg) {
                    messageHandler(connPtr, msg, _emailMap[emailUuid], callback);
                }
        );
        tcpSocket->connect();
    });
}

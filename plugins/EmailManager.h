//
// Created by g29624 on 2022/8/29.
//

#pragma once

#include <drogon/plugins/Plugin.h>
#include <trantor/net/TcpClient.h>

namespace techmino::plugins {
    class EmailManager : public drogon::Plugin<EmailManager> {
    private:
        enum class EmailState {
            Init,
            HandShake,
            Tls [[maybe_unused]],
            Auth,
            User,
            Pass,
            Mail,
            Recipient,
            Data,
            Body,
            Quit,
            Close
        };

        struct Email {
            const std::string _account,
                    _password,
                    _senderEmail,
                    _senderName,
                    _receiverEmail,
                    _subject,
                    _content;
            std::atomic<bool> _isHTML{false};
            std::atomic<EmailState> _state{EmailState::Init};
            std::shared_ptr<trantor::TcpClient> _socket;

            Email(
                    std::string account,
                    std::string password,
                    std::string senderEmail,
                    std::string senderName,
                    std::string receiverEmail,
                    std::string subject,
                    std::string content,
                    bool isHTML,
                    std::shared_ptr<trantor::TcpClient> socket
            );

            ~Email() = default;
        };
    public:
        void initAndStart(const Json::Value &config) override;

        void shutdown() override;

        void smtp(
                const std::string &receiverEmail,
                const std::string &subject,
                const std::string &content,
                bool isHTML,
                const std::function<void(bool, const std::string &)> &callback
        );

    private:
        std::string _server;
        uint32_t _port;
        std::string _account, _password, _senderEmail, _senderName;
        std::unordered_map<std::string, std::shared_ptr<Email>> _emailMap;

        static void messageHandler(
                const trantor::TcpConnectionPtr &connPtr,
                trantor::MsgBuffer *msgBuffer,
                const std::shared_ptr<Email> &email,
                const std::function<void(bool result, const std::string &)> &callback
        );
    };
}

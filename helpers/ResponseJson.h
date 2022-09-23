//
// Created by ParticleG on 2022/2/9.
//

#pragma once

#include <helpers/BasicJson.h>
#include <types/ResultCode.h>

namespace techmino::helpers {
    class ResponseJson : public BasicJson {
    public:
        typedef std::function<void(const drogon::HttpResponsePtr &)> HttpCallback;

        explicit ResponseJson(
                drogon::HttpStatusCode statusCode = drogon::k200OK,
                types::ResultCode resultCode = types::ResultCode::Completed
        );

        ResponseJson &setResultCode(types::ResultCode code);

        ResponseJson &setResultCode(uint32_t code);

        ResponseJson &setStatusCode(drogon::HttpStatusCode code);

        ResponseJson &setMessage(const std::string &message);

        ResponseJson &setData(Json::Value data);

        ResponseJson &setReason(const std::exception &e);

        [[maybe_unused]] ResponseJson &setReason(const drogon::orm::DrogonDbException &e);

        ResponseJson &setReason(const std::string &reason);

        void to(const HttpCallback &callback) const;

    private:
        drogon::HttpStatusCode _statusCode = drogon::k200OK;
    };
}




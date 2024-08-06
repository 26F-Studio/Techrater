//
// Created by particleg on 2022/9/1.
//

#pragma once

#include <drogon/HttpController.h>
#include <plugins/NoticeManager.h>
#include <structures/ExceptionHandlers.h>

namespace techmino::api::v1 {
    class Notice :
            public drogon::HttpController<Notice>,
            public structures::ResponseJsonHandler<Notice> {
    public:
        static constexpr char projectName[] = CMAKE_PROJECT_NAME;

    public:
        Notice();

        METHOD_LIST_BEGIN
            METHOD_ADD(
                    Notice::createNotice,
                    "",
                    drogon::Post,
                    "techmino::filters::CheckAccessToken",
                    "techmino::filters::NoticeCreate"
            );
            METHOD_ADD(
                    Notice::retrieveNotice,
                    "",
                    drogon::Get,
                    "techmino::filters::NoticeRetrieve"
            );
            METHOD_ADD(
                    Notice::updateNotice,
                    "",
                    drogon::Put,
                    "techmino::filters::CheckAccessToken",
                    "techmino::filters::NoticeUpdate"
            );
            METHOD_ADD(
                    Notice::deleteNotice,
                    "",
                    drogon::Delete,
                    "techmino::filters::CheckAccessToken",
                    "techmino::filters::NoticeDelete"
            );
        METHOD_LIST_END

        void createNotice(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);

        void retrieveNotice(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);

        void updateNotice(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);

        void deleteNotice(const drogon::HttpRequestPtr &req, std::function<void(const drogon::HttpResponsePtr &)> &&callback);

    private:
        techmino::plugins::NoticeManager *_noticeManager;
    };
}

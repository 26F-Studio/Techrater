//
// Created by particleg on 2022/8/29.
//

#pragma once

#include <drogon/plugins/Plugin.h>
#include <helpers/I18nHelper.h>
#include <models/Notice.h>

namespace techmino::plugins {
    class NoticeManager :
            public drogon::Plugin<NoticeManager>,
            public helpers::I18nHelper<NoticeManager> {
    public:
        static constexpr char projectName[] = CMAKE_PROJECT_NAME;

    public:
        NoticeManager();

        void initAndStart(const Json::Value &config) override;

        void shutdown() override;

        uint64_t createNotice(const Json::Value &contents);

        Json::Value retrieveNotice(uint64_t lastCount, const std::string &language);

        Json::Value retrieveNotice(uint64_t pageIndex, uint64_t pageSize, const std::string &language);

        void updateNotice(uint64_t noticeId, const Json::Value &contents);

        void deleteNotice(uint64_t noticeId);

    private:
        std::string _fallbackLanguage;
        drogon::orm::Mapper<drogon_model::techrater::Notice> _noticeMapper;
    };
}

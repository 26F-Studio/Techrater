//
// Created by particleg on 2022/8/29.
//

#include <drogon/drogon.h>
#include <plugins/NoticeManager.h>
#include <structures/Exceptions.h>

using namespace drogon;
using namespace drogon_model;
using namespace std;
using namespace techmino::helpers;
using namespace techmino::plugins;
using namespace techmino::structures;
using namespace techmino::types;

NoticeManager::NoticeManager() : _noticeMapper(app().getDbClient()) {}

void NoticeManager::initAndStart(const Json::Value &config) {
    if (!(
            config["fallbackLanguage"].isString()
    )) {
        LOG_ERROR << R"(Invalid fallbackLanguage config)";
        abort();
    }
    _fallbackLanguage = config["fallbackLanguage"].asString();
    LOG_INFO << "NoticeManager loaded.";
}

void NoticeManager::shutdown() {
    LOG_INFO << "NoticeManager shutdown.";
}

uint64_t NoticeManager::createNotice(const Json::Value &contents) {
    techrater::Notice notice;
    notice.updateByJson(contents);
    _noticeMapper.insert(notice);
    return notice.getValueOfId();
}

Json::Value NoticeManager::retrieveNotice(uint64_t lastCount, const string &language) {
    Json::Value result;

    bool hasLanguage = false;
    for (uint64_t index = 0; index < techrater::Notice::getColumnNumber(); index++) {
        if (techrater::Notice::getColumnName(index) == language) {
            hasLanguage = true;
            break;
        }
    }
    if (hasLanguage) {
        result["language"] = language;
    } else {
        result["language"] = _fallbackLanguage;
    }

    Json::Value contents(Json::arrayValue);
    for (const auto &notice: _noticeMapper.orderBy(
            techrater::Notice::Cols::_id,
            orm::SortOrder::DESC
    ).limit(lastCount).findAll()) {
        Json::Value tempContent, noticeJson = notice.toJson();
        tempContent["id"] = noticeJson["id"];
        if (noticeJson[language].isNull()) {
            tempContent["content"] = noticeJson[_fallbackLanguage];
        } else {
            tempContent["content"] = noticeJson[language];
        }
        contents.append(tempContent);
    }

    result["contents"] = std::move(contents);
    return result;
}

Json::Value NoticeManager::retrieveNotice(uint64_t pageIndex, uint64_t pageSize, const string &language) {
    Json::Value result;

    bool hasLanguage = false;
    for (uint64_t index = 0; index < techrater::Notice::getColumnNumber(); index++) {
        if (techrater::Notice::getColumnName(index) == language) {
            hasLanguage = true;
            break;
        }
    }
    if (hasLanguage) {
        result["language"] = language;
    } else {
        result["language"] = _fallbackLanguage;
    }

    Json::Value contents(Json::arrayValue);
    for (const auto &notice: _noticeMapper.orderBy(
            techrater::Notice::Cols::_id,
            orm::SortOrder::DESC
    ).paginate(pageIndex, pageSize).findAll()) {
        Json::Value tempContent, noticeJson = notice.toJson();
        tempContent["id"] = noticeJson["id"];
        tempContent["timestamp"] = noticeJson["timestamp"];
        if (hasLanguage) {
            tempContent["content"] = noticeJson[language];
        } else {
            tempContent["content"] = noticeJson[_fallbackLanguage];
        }
        contents.append(tempContent);
    }

    result["contents"] = std::move(contents);
    return result;
}

void NoticeManager::updateNotice(uint64_t noticeId, const Json::Value &contents) {
    try {
        auto notice = _noticeMapper.findOne(orm::Criteria(
                techrater::Notice::Cols::_id,
                orm::CompareOperator::EQ,
                noticeId
        ));
        notice.updateByJson(contents);
        _noticeMapper.update(notice);
    } catch (const orm::UnexpectedRows &e) {
        LOG_DEBUG << "Unexpected rows: " << e.what();
        throw ResponseException(
                i18n("noticeNotFound"),
                ResultCode::NotFound,
                k404NotFound
        );
    }
}

void NoticeManager::deleteNotice(uint64_t noticeId) {
    try {
        _noticeMapper.deleteBy(orm::Criteria(
                techrater::Notice::Cols::_id,
                orm::CompareOperator::EQ,
                noticeId
        ));
    } catch (const orm::UnexpectedRows &e) {
        LOG_DEBUG << "Unexpected rows: " << e.what();
        throw ResponseException(
                i18n("noticeNotFound"),
                ResultCode::NotFound,
                k404NotFound
        );
    }
}

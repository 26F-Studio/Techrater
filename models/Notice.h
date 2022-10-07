/**
 *
 *  Notice.h
 *  DO NOT EDIT. This file is generated by drogon_ctl
 *
 */

#pragma once
#include <drogon/orm/Result.h>
#include <drogon/orm/Row.h>
#include <drogon/orm/Field.h>
#include <drogon/orm/SqlBinder.h>
#include <drogon/orm/Mapper.h>
#ifdef __cpp_impl_coroutine
#include <drogon/orm/CoroMapper.h>
#endif
#include <trantor/utils/Date.h>
#include <trantor/utils/Logger.h>
#include <json/json.h>
#include <string>
#include <memory>
#include <vector>
#include <tuple>
#include <stdint.h>
#include <iostream>

namespace drogon
{
namespace orm
{
class DbClient;
using DbClientPtr = std::shared_ptr<DbClient>;
}
}
namespace drogon_model
{
namespace techrater
{

class Notice
{
  public:
    struct Cols
    {
        static const std::string _id;
        static const std::string _create_time;
        static const std::string _en_us;
        static const std::string _zh_cn;
        static const std::string _zh_tw;
        static const std::string _fr_fr;
        static const std::string _es_es;
        static const std::string _pt_pt;
        static const std::string _id_id;
        static const std::string _ja_jp;
    };

    const static int primaryKeyNumber;
    const static std::string tableName;
    const static bool hasPrimaryKey;
    const static std::string primaryKeyName;
    using PrimaryKeyType = void;
    int getPrimaryKey() const { assert(false); return 0; }

    /**
     * @brief constructor
     * @param r One row of records in the SQL query result.
     * @param indexOffset Set the offset to -1 to access all columns by column names,
     * otherwise access all columns by offsets.
     * @note If the SQL is not a style of 'select * from table_name ...' (select all
     * columns by an asterisk), please set the offset to -1.
     */
    explicit Notice(const drogon::orm::Row &r, const ssize_t indexOffset = 0) noexcept;

    /**
     * @brief constructor
     * @param pJson The json object to construct a new instance.
     */
    explicit Notice(const Json::Value &pJson) noexcept(false);

    /**
     * @brief constructor
     * @param pJson The json object to construct a new instance.
     * @param pMasqueradingVector The aliases of table columns.
     */
    Notice(const Json::Value &pJson, const std::vector<std::string> &pMasqueradingVector) noexcept(false);

    Notice() = default;

    void updateByJson(const Json::Value &pJson) noexcept(false);
    void updateByMasqueradedJson(const Json::Value &pJson,
                                 const std::vector<std::string> &pMasqueradingVector) noexcept(false);
    static bool validateJsonForCreation(const Json::Value &pJson, std::string &err);
    static bool validateMasqueradedJsonForCreation(const Json::Value &,
                                                const std::vector<std::string> &pMasqueradingVector,
                                                    std::string &err);
    static bool validateJsonForUpdate(const Json::Value &pJson, std::string &err);
    static bool validateMasqueradedJsonForUpdate(const Json::Value &,
                                          const std::vector<std::string> &pMasqueradingVector,
                                          std::string &err);
    static bool validJsonOfField(size_t index,
                          const std::string &fieldName,
                          const Json::Value &pJson,
                          std::string &err,
                          bool isForCreation);

    /**  For column id  */
    ///Get the value of the column id, returns the default value if the column is null
    const int64_t &getValueOfId() const noexcept;
    ///Return a shared_ptr object pointing to the column const value, or an empty shared_ptr object if the column is null
    const std::shared_ptr<int64_t> &getId() const noexcept;
    ///Set the value of the column id
    void setId(const int64_t &pId) noexcept;

    /**  For column create_time  */
    ///Get the value of the column create_time, returns the default value if the column is null
    const ::trantor::Date &getValueOfCreateTime() const noexcept;
    ///Return a shared_ptr object pointing to the column const value, or an empty shared_ptr object if the column is null
    const std::shared_ptr<::trantor::Date> &getCreateTime() const noexcept;
    ///Set the value of the column create_time
    void setCreateTime(const ::trantor::Date &pCreateTime) noexcept;
    void setCreateTimeToNull() noexcept;

    /**  For column en_us  */
    ///Get the value of the column en_us, returns the default value if the column is null
    const std::string &getValueOfEnUs() const noexcept;
    ///Return a shared_ptr object pointing to the column const value, or an empty shared_ptr object if the column is null
    const std::shared_ptr<std::string> &getEnUs() const noexcept;
    ///Set the value of the column en_us
    void setEnUs(const std::string &pEnUs) noexcept;
    void setEnUs(std::string &&pEnUs) noexcept;
    void setEnUsToNull() noexcept;

    /**  For column zh_cn  */
    ///Get the value of the column zh_cn, returns the default value if the column is null
    const std::string &getValueOfZhCn() const noexcept;
    ///Return a shared_ptr object pointing to the column const value, or an empty shared_ptr object if the column is null
    const std::shared_ptr<std::string> &getZhCn() const noexcept;
    ///Set the value of the column zh_cn
    void setZhCn(const std::string &pZhCn) noexcept;
    void setZhCn(std::string &&pZhCn) noexcept;
    void setZhCnToNull() noexcept;

    /**  For column zh_tw  */
    ///Get the value of the column zh_tw, returns the default value if the column is null
    const std::string &getValueOfZhTw() const noexcept;
    ///Return a shared_ptr object pointing to the column const value, or an empty shared_ptr object if the column is null
    const std::shared_ptr<std::string> &getZhTw() const noexcept;
    ///Set the value of the column zh_tw
    void setZhTw(const std::string &pZhTw) noexcept;
    void setZhTw(std::string &&pZhTw) noexcept;
    void setZhTwToNull() noexcept;

    /**  For column fr_fr  */
    ///Get the value of the column fr_fr, returns the default value if the column is null
    const std::string &getValueOfFrFr() const noexcept;
    ///Return a shared_ptr object pointing to the column const value, or an empty shared_ptr object if the column is null
    const std::shared_ptr<std::string> &getFrFr() const noexcept;
    ///Set the value of the column fr_fr
    void setFrFr(const std::string &pFrFr) noexcept;
    void setFrFr(std::string &&pFrFr) noexcept;
    void setFrFrToNull() noexcept;

    /**  For column es_es  */
    ///Get the value of the column es_es, returns the default value if the column is null
    const std::string &getValueOfEsEs() const noexcept;
    ///Return a shared_ptr object pointing to the column const value, or an empty shared_ptr object if the column is null
    const std::shared_ptr<std::string> &getEsEs() const noexcept;
    ///Set the value of the column es_es
    void setEsEs(const std::string &pEsEs) noexcept;
    void setEsEs(std::string &&pEsEs) noexcept;
    void setEsEsToNull() noexcept;

    /**  For column pt_pt  */
    ///Get the value of the column pt_pt, returns the default value if the column is null
    const std::string &getValueOfPtPt() const noexcept;
    ///Return a shared_ptr object pointing to the column const value, or an empty shared_ptr object if the column is null
    const std::shared_ptr<std::string> &getPtPt() const noexcept;
    ///Set the value of the column pt_pt
    void setPtPt(const std::string &pPtPt) noexcept;
    void setPtPt(std::string &&pPtPt) noexcept;
    void setPtPtToNull() noexcept;

    /**  For column id_id  */
    ///Get the value of the column id_id, returns the default value if the column is null
    const std::string &getValueOfIdId() const noexcept;
    ///Return a shared_ptr object pointing to the column const value, or an empty shared_ptr object if the column is null
    const std::shared_ptr<std::string> &getIdId() const noexcept;
    ///Set the value of the column id_id
    void setIdId(const std::string &pIdId) noexcept;
    void setIdId(std::string &&pIdId) noexcept;
    void setIdIdToNull() noexcept;

    /**  For column ja_jp  */
    ///Get the value of the column ja_jp, returns the default value if the column is null
    const std::string &getValueOfJaJp() const noexcept;
    ///Return a shared_ptr object pointing to the column const value, or an empty shared_ptr object if the column is null
    const std::shared_ptr<std::string> &getJaJp() const noexcept;
    ///Set the value of the column ja_jp
    void setJaJp(const std::string &pJaJp) noexcept;
    void setJaJp(std::string &&pJaJp) noexcept;
    void setJaJpToNull() noexcept;


    static size_t getColumnNumber() noexcept {  return 10;  }
    static const std::string &getColumnName(size_t index) noexcept(false);

    Json::Value toJson() const;
    Json::Value toMasqueradedJson(const std::vector<std::string> &pMasqueradingVector) const;
    /// Relationship interfaces
  private:
    friend drogon::orm::Mapper<Notice>;
#ifdef __cpp_impl_coroutine
    friend drogon::orm::CoroMapper<Notice>;
#endif
    static const std::vector<std::string> &insertColumns() noexcept;
    void outputArgs(drogon::orm::internal::SqlBinder &binder) const;
    const std::vector<std::string> updateColumns() const;
    void updateArgs(drogon::orm::internal::SqlBinder &binder) const;
    ///For mysql or sqlite3
    void updateId(const uint64_t id);
    std::shared_ptr<int64_t> id_;
    std::shared_ptr<::trantor::Date> createTime_;
    std::shared_ptr<std::string> enUs_;
    std::shared_ptr<std::string> zhCn_;
    std::shared_ptr<std::string> zhTw_;
    std::shared_ptr<std::string> frFr_;
    std::shared_ptr<std::string> esEs_;
    std::shared_ptr<std::string> ptPt_;
    std::shared_ptr<std::string> idId_;
    std::shared_ptr<std::string> jaJp_;
    struct MetaData
    {
        const std::string colName_;
        const std::string colType_;
        const std::string colDatabaseType_;
        const ssize_t colLength_;
        const bool isAutoVal_;
        const bool isPrimaryKey_;
        const bool notNull_;
    };
    static const std::vector<MetaData> metaData_;
    bool dirtyFlag_[10]={ false };
  public:
    static const std::string &sqlForFindingByPrimaryKey()
    {
        static const std::string sql="";
        return sql;
    }

    static const std::string &sqlForDeletingByPrimaryKey()
    {
        static const std::string sql="";
        return sql;
    }
    std::string sqlForInserting(bool &needSelection) const
    {
        std::string sql="insert into " + tableName + " (";
        size_t parametersCount = 0;
        needSelection = false;
            sql += "id,";
            ++parametersCount;
        sql += "create_time,";
        ++parametersCount;
        if(!dirtyFlag_[1])
        {
            needSelection=true;
        }
        if(dirtyFlag_[2])
        {
            sql += "en_us,";
            ++parametersCount;
        }
        if(dirtyFlag_[3])
        {
            sql += "zh_cn,";
            ++parametersCount;
        }
        if(dirtyFlag_[4])
        {
            sql += "zh_tw,";
            ++parametersCount;
        }
        if(dirtyFlag_[5])
        {
            sql += "fr_fr,";
            ++parametersCount;
        }
        if(dirtyFlag_[6])
        {
            sql += "es_es,";
            ++parametersCount;
        }
        if(dirtyFlag_[7])
        {
            sql += "pt_pt,";
            ++parametersCount;
        }
        if(dirtyFlag_[8])
        {
            sql += "id_id,";
            ++parametersCount;
        }
        if(dirtyFlag_[9])
        {
            sql += "ja_jp,";
            ++parametersCount;
        }
        if(parametersCount > 0)
        {
            sql[sql.length()-1]=')';
            sql += " values (";
        }
        else
            sql += ") values (";

        int placeholder=1;
        char placeholderStr[64];
        size_t n=0;
        sql +="default,";
        if(dirtyFlag_[1])
        {
            n = sprintf(placeholderStr,"$%d,",placeholder++);
            sql.append(placeholderStr, n);
        }
        else
        {
            sql +="default,";
        }
        if(dirtyFlag_[2])
        {
            n = sprintf(placeholderStr,"$%d,",placeholder++);
            sql.append(placeholderStr, n);
        }
        if(dirtyFlag_[3])
        {
            n = sprintf(placeholderStr,"$%d,",placeholder++);
            sql.append(placeholderStr, n);
        }
        if(dirtyFlag_[4])
        {
            n = sprintf(placeholderStr,"$%d,",placeholder++);
            sql.append(placeholderStr, n);
        }
        if(dirtyFlag_[5])
        {
            n = sprintf(placeholderStr,"$%d,",placeholder++);
            sql.append(placeholderStr, n);
        }
        if(dirtyFlag_[6])
        {
            n = sprintf(placeholderStr,"$%d,",placeholder++);
            sql.append(placeholderStr, n);
        }
        if(dirtyFlag_[7])
        {
            n = sprintf(placeholderStr,"$%d,",placeholder++);
            sql.append(placeholderStr, n);
        }
        if(dirtyFlag_[8])
        {
            n = sprintf(placeholderStr,"$%d,",placeholder++);
            sql.append(placeholderStr, n);
        }
        if(dirtyFlag_[9])
        {
            n = sprintf(placeholderStr,"$%d,",placeholder++);
            sql.append(placeholderStr, n);
        }
        if(parametersCount > 0)
        {
            sql.resize(sql.length() - 1);
        }
        if(needSelection)
        {
            sql.append(") returning *");
        }
        else
        {
            sql.append(1, ')');
        }
        LOG_TRACE << sql;
        return sql;
    }
};
} // namespace techrater
} // namespace drogon_model

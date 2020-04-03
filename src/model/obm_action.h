// Copyright 2015 Toggl Desktop developers.

#ifndef SRC_OBM_ACTION_H_
#define SRC_OBM_ACTION_H_

#include <string>

#include <json/json.h>  // NOLINT

#include <Poco/Types.h>

#include "base_model.h"

namespace toggl {

class TOGGL_INTERNAL_EXPORT ObmAction : public BaseModel {
    inline static const std::string modelName{ kModelObmAction };
    inline static const Query query{
        Query::Table{"obm_actions"},
        Query::Columns {
            { "local_id", true },
            { "guid", false },
            { "experiment_id", true },
            { "key", true },
            { "value", true }
        },
        Query::Join{},
        Query::OrderBy{},
        // doesn't actually use stuff from BaseModel
        nullptr
    };
    ObmAction(ProtectedBase *container, Poco::Data::RecordSet &rs)
        : BaseModel(container, rs)
    {
        size_t ptr{ query.Offset() };
        // hacky
        Poco::Int64 localId;
        load(rs, query.IsRequired(ptr), ptr, localId);
        SetLocalID(localId);
        ptr++;
        guid uuid;
        load(rs, query.IsRequired(ptr), ptr, uuid);
        SetGUID(uuid);
        ptr++;
        load(rs, query.IsRequired(ptr), ptr, experiment_id_);
        ptr++;
        load(rs, query.IsRequired(ptr), ptr, key_);
        ptr++;
        load(rs, query.IsRequired(ptr), ptr, value_);
        ptr++;
        ClearDirty();
    }
    ObmAction(ProtectedBase *container)
        : BaseModel(container)
    {}
 public:
    friend class ProtectedBase;

    const std::string &Key() const {
        return key_;
    }
    void SetKey(const std::string &value);

    const std::string &Value() const {
        return value_;
    }
    void SetValue(const std::string &value);

    const Poco::UInt64 &ExperimentID() const {
        return experiment_id_;
    }
    void SetExperimentID(const Poco::UInt64 value);

    // Override BaseModel
    std::string String() const override;
    std::string ModelName() const override;
    std::string ModelURL() const override;
    Json::Value SaveToJSON() const override;

 private:
    Poco::UInt64 experiment_id_ { 0 };
    std::string key_ { "" };
    std::string value_ { "" };
};

class TOGGL_INTERNAL_EXPORT ObmExperiment : public BaseModel {
    inline static const std::string modelName{ kModelObmExperiment };
    inline static const Query query{
        Query::Table{"obm_experiments"},
        Query::Columns {
            { "local_id", true },
            { "guid", false },
            { "nr", true },
            { "included", true },
            { "has_seen", true },
            { "actions", true }
        },
        Query::Join{},
        Query::OrderBy{},
        // doesn't actually use stuff from BaseModel
        nullptr
    };
    ObmExperiment(ProtectedBase *container, Poco::Data::RecordSet &rs)
        : BaseModel(container, rs)
    {
        size_t ptr{ query.Offset() };
        // hacky
        Poco::Int64 localId;
        load(rs, query.IsRequired(ptr), ptr, localId);
        SetLocalID(localId);
        ptr++;
        guid uuid;
        load(rs, query.IsRequired(ptr), ptr, uuid);
        SetGUID(uuid);
        ptr++;
        load(rs, query.IsRequired(ptr), ptr, nr_);
        ptr++;
        load(rs, query.IsRequired(ptr), ptr, included_);
        ptr++;
        load(rs, query.IsRequired(ptr), ptr, has_seen_);
        ptr++;
        load(rs, query.IsRequired(ptr), ptr, actions_);
        ptr++;
        ClearDirty();
    }
    ObmExperiment(ProtectedBase *container)
        : BaseModel(container)
    {}
 public:
   friend class ProtectedBase;

    const bool &Included() const {
        return included_;
    }
    void SetIncluded(const bool value);

    const bool &HasSeen() const {
        return has_seen_;
    }
    void SetHasSeen(const bool value);

    const Poco::UInt64 &Nr() const {
        return nr_;
    }
    void SetNr(const Poco::UInt64 value);

    const std::string &Actions() const {
        return actions_;
    }
    void SetActions(const std::string &value);

    // Override BaseModel
    std::string String() const override;
    std::string ModelName() const override;
    std::string ModelURL() const override;

 private:
    bool included_ { false };
    Poco::UInt64 nr_ { 0 };
    bool has_seen_ { false };
    std::string actions_ { "" };
};

}  // namespace toggl

#endif  // SRC_OBM_ACTION_H_

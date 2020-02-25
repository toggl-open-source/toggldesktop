// Copyright 2014 Toggl Desktop developers.

#include "gtest/gtest.h"

#include <iostream>  // NOLINT

#include "./../autotracker.h"
#include "./../client.h"
#include "./../const.h"
#include "./../database.h"
#include "./../formatter.h"
#include "./../obm_action.h"
#include "./../project.h"
#include "./../proxy.h"
#include "./../settings.h"
#include "./../tag.h"
#include "./../task.h"
#include "./../time_entry.h"
#include "./../timeline_event.h"
#include "./../timeline_uploader.h"
#include "./../user.h"
#include "./../workspace.h"

#include "./test_data.h"

#include "Poco/File.h"
#include "Poco/FileStream.h"
#include "Poco/Logger.h"
#include "Poco/LocalDateTime.h"

namespace toggl {

namespace testing {
class Database {
 public:
    Database() : db_(0) {
        Poco::File f("test->db");
        if (f.exists()) {
            f.remove(false);
        }
        db_ = new toggl::Database("test->db");
    }
    ~Database() {
        if (db_) {
            delete db_;
        }
    }
    toggl::Database *instance() {
        return db_;
    }

 private:
    toggl::Database *db_;
};

}  // namespace testing

TEST(TimeEntry, TimeEntryReturnsTags) {
    ProtectedModel<TimeEntry> te { nullptr, true };
    te->SetTags("alfa|beeta");
    ASSERT_EQ(std::string("alfa|beeta"), te->Tags());
}

TEST(TimeEntry, WillNotPushUnlessValidationErrorIsCleared) {
    ProtectedModel<TimeEntry> te { nullptr, true };
    ASSERT_TRUE(te->NeedsPush());

    // Simulate getting an error from backend
    te->SetValidationError("All you do is wrong");
    ASSERT_EQ("All you do is wrong", te->ValidationError());
    ASSERT_FALSE(te->NeedsPush());

    // Simulate user changing the data,
    // which should wipe the validation error.
    te->SetDurationUserInput("10 seconds");
    ASSERT_EQ("", te->ValidationError());
    ASSERT_TRUE(te->NeedsPush());
}

TEST(TimeEntry, SetDurationUserInput) {
    ProtectedModel<TimeEntry> te { nullptr, true };
    ASSERT_FALSE(te->Dirty());
    ASSERT_FALSE(te->UIModifiedAt());

    te->SetDurationUserInput("1 minute");
    ASSERT_TRUE(te->Dirty());
    ASSERT_TRUE(te->UIModifiedAt());
}

TEST(Project, ProjectsHaveColorCodes) {
    ProtectedModel<Project> p { nullptr, true };
    p->SetColor("1");
    ASSERT_EQ("#c56bff", p->ColorCode());
    p->SetColor("");
    ASSERT_EQ("", p->ColorCode());
    p->SetColor("0");
    ASSERT_EQ("#06aaf5", p->ColorCode());
}

TEST(Project, ResolveOnlyAdminsCanChangeProjectVisibility) {
    ProtectedModel<Project> p { nullptr, true };
    p->SetPrivate(false);
    error err = error("Only admins can change project visibility");
    ASSERT_TRUE(p->ResolveError(err));
    ASSERT_TRUE(p->IsPrivate());
}

#if 0
TEST(User, CreateCompressedTimelineBatchForUpload) {
    testing::Database db;

    RelatedData related;
    ProtectedModel<User> user { &related, true };
    ASSERT_EQ(noError, user->LoadUserAndRelatedDataFromJSONString(loadTestData(), true));
    Poco::UInt64 user_id = user->ID();

    std::vector<ModelChange> changes;

    Poco::UInt64 good_duration_seconds(30);

    // Event that happened at least 15 minutes ago,
    // can be uploaded to Toggl backend.
    TimelineEvent *good = new TimelineEvent();
    good->SetUID(user_id);
    // started yesterday, "16 minutes ago"
    good->SetStart(time(0) - 86400 - 60*16);
    good->SetEndTime(good->Start() + good_duration_seconds);
    good->SetFilename("Notepad.exe");
    good->SetTitle("untitled");
    related.TimelineEvents.push_back(good);

    Poco::UInt64 good2_duration_seconds(20);

    // Another event that happened at least 15 minutes ago,
    // can be uploaded to Toggl backend.
    TimelineEvent *good2 = new TimelineEvent();
    good2->SetUID(user_id);
    good2->SetStart(good->EndTime() + 1);  // started after first event
    good2->SetEndTime(good2->Start() + good2_duration_seconds);
    good2->SetFilename("Notepad.exe");
    good2->SetTitle("untitled");
    related.TimelineEvents.push_back(good2);

    // Another event that happened at least 15 minutes ago,
    // but has already been uploaded to Toggl backend.
    TimelineEvent *uploaded = new TimelineEvent();;
    uploaded->SetUID(user_id);
    uploaded->SetStart(good2->EndTime() + 1);  // started after second event
    uploaded->SetEndTime(uploaded->Start() + 10);
    uploaded->SetFilename("Notepad.exe");
    uploaded->SetTitle("untitled");
    uploaded->SetUploaded(true);
    related.TimelineEvents.push_back(uploaded);

    // This event happened less than 15 minutes ago,
    // so it must not be uploaded
    TimelineEvent *too_fresh = new TimelineEvent();
    too_fresh->SetUID(user_id);
    too_fresh->SetStart(time(0) - 60);  // started 1 minute ago
    too_fresh->SetEndTime(time(0));  // lasted until now
    too_fresh->SetFilename("Notepad.exe");
    too_fresh->SetTitle("notes");
    related.TimelineEvents.push_back(too_fresh);

    // This event happened more than 7 days ago,
    // so it must not be uploaded, just deleted
    TimelineEvent *too_old = new TimelineEvent();
    too_old->SetUID(user_id);
    too_old->SetStart(time(0) - kTimelineSecondsToKeep - 1);  // 7 days ago
    too_old->SetEndTime(too_old->EndTime() + 120);  // lasted 2 minutes
    too_old->SetFilename("Notepad.exe");
    too_old->SetTitle("diary");
    related.TimelineEvents.push_back(too_old);

    db.instance()->SaveUser(&user, true, &changes);

    user->CompressTimeline();
    std::vector<TimelineEvent> timeline_events = user->CompressedTimelineForUpload();

    if (timeline_events.size() != 1) {
        std::cerr << "related.TimelineEvents:" << std::endl;
        for (std::vector<TimelineEvent *>::const_iterator it =
            related.TimelineEvents.begin();
                it != related.TimelineEvents.end(); it++) {
            TimelineEvent *ev = *it;
            std::cerr << ev->String() << std::endl;
        }

        std::cerr << "user->CompressedTimelineForUpload:" << std::endl;
        for (std::vector<TimelineEvent>::const_iterator it =
            timeline_events.begin();
                it != timeline_events.end(); it++) {
            TimelineEvent ev = *it;
            std::cerr << ev.String() << std::endl;
        }
    }

    ASSERT_EQ(size_t(1), timeline_events.size());

    // Compress some more, for fun and profit
    for (int i = 0; i < 100; i++) {
        user->CompressTimeline();
        timeline_events = user->CompressedTimelineForUpload();
    }

    ASSERT_EQ(size_t(1), timeline_events.size());

    TimelineEvent ready_for_upload = timeline_events[0];
    ASSERT_TRUE(ready_for_upload.Chunked());
    ASSERT_EQ(good->UID(), ready_for_upload.UID());

    ASSERT_NE(good2->Start(), ready_for_upload.Start());
    ASSERT_NE(uploaded->Start(), ready_for_upload.Start());
    ASSERT_NE(too_old->Start(), ready_for_upload.Start());
    ASSERT_NE(too_fresh->Start(), ready_for_upload.Start());
    ASSERT_EQ(good->Start(), ready_for_upload.Start());

    ASSERT_EQ(static_cast<Poco::Int64>(
        good_duration_seconds + good2_duration_seconds),
              ready_for_upload.Duration());
    ASSERT_EQ(good->Filename(), ready_for_upload.Filename());
    ASSERT_EQ(good->Title(), ready_for_upload.Title());
    ASSERT_EQ(good->Idle(), ready_for_upload.Idle());
    ASSERT_FALSE(ready_for_upload.Uploaded());

    // Fake that we have uploaded the chunked timeline event now
    user->MarkTimelineBatchAsUploaded(timeline_events);

    // Now, no more events should exist for upload
    std::vector<TimelineEvent> left_for_upload = user->CompressedTimelineForUpload();
    ASSERT_EQ(std::size_t(0), left_for_upload.size());
}
#endif

TEST(Database, Trim) {
    testing::Database db;
    std::string text(" jäääär ");
    std::string result("");
    error err = db.instance()->Trim(text, &result);
    ASSERT_EQ(noError, err);
    ASSERT_EQ(std::string("jäääär"), result);
}

TEST(Database, SaveAndLoadCurrentAPIToken) {
    testing::Database db;
    std::string api_token("");
    Poco::UInt64 uid(0);
    ASSERT_EQ(noError, db.instance()->CurrentAPIToken(&api_token, &uid));
    ASSERT_EQ("", api_token);
    ASSERT_EQ(Poco::UInt64(0), uid);

    api_token = "abc123";
    uid = 123;
    ASSERT_EQ(noError, db.instance()->SetCurrentAPIToken(api_token, uid));
    ASSERT_EQ(noError, db.instance()->SetCurrentAPIToken(api_token, uid));

    Poco::UInt64 n(0);
    ASSERT_EQ(noError,
              db.instance()->UInt("select count(1) from sessions", &n));
    ASSERT_EQ(Poco::UInt64(1), n);

    std::string api_token_from_db("");
    Poco::UInt64 uid_from_db(0);
    ASSERT_EQ(noError,
              db.instance()->CurrentAPIToken(&api_token_from_db, &uid_from_db));
    ASSERT_EQ("abc123", api_token_from_db);
    ASSERT_EQ(Poco::UInt64(123), uid_from_db);

    ASSERT_EQ(noError, db.instance()->ClearCurrentAPIToken());
    ASSERT_EQ(noError,
              db.instance()->UInt("select count(1) from sessions", &n));
    ASSERT_EQ(Poco::UInt64(0), n);

    ASSERT_EQ(noError,
              db.instance()->CurrentAPIToken(&api_token_from_db, &uid_from_db));
    ASSERT_EQ("", api_token_from_db);
    ASSERT_EQ(Poco::UInt64(0), uid_from_db);
}

Json::Value jsonStringToValue(const std::string json_string) {
    Json::Value root;
    Json::Reader reader;
    reader.parse(json_string, root);
    return root;
}

TEST(User, Since) {
    ProtectedModel<User> u { nullptr, true };

    // no timestamp should be wrong
    ASSERT_FALSE(u->HasValidSinceDate());

    // 0 timestamp should be wrong
    u->SetSince(0);
    ASSERT_FALSE(u->HasValidSinceDate());

    // current time should be ok
    u->SetSince(time(0));
    ASSERT_TRUE(u->HasValidSinceDate());

    // 1 month ago should be fine
    u->SetSince(time(0) - 2.62974e6);
    ASSERT_TRUE(u->HasValidSinceDate());
}

TEST(User, UpdatesTimeEntryFromJSON) {
    RelatedData related;
    ProtectedModel<User> user { &related, true };
    ASSERT_EQ(noError,
              user->LoadUserAndRelatedDataFromJSONString(loadTestData(), true));

    auto te = related.TimeEntries.byID(89818605);
    ASSERT_TRUE(te);

    std::string json = "{\"id\":89818605,\"description\":\"Changed\"}";
    te->LoadFromJSON(jsonStringToValue(json));
    ASSERT_EQ("Changed", te->Description());
}

TEST(User, DeletesZombies) {
    RelatedData related;
    ProtectedModel<User> user { &related, true };
    ASSERT_EQ(noError,
              user->LoadUserAndRelatedDataFromJSONString(loadTestData(), true));

    locked<TimeEntry> te = related.TimeEntries.byID(89818605);
    ASSERT_TRUE(te);
    ASSERT_FALSE(te->IsMarkedAsDeletedOnServer());

    std::string json =
        loadTestDataFile(std::string("../testdata/me_without_time_entries.json"));

    ASSERT_EQ(noError,
              user->LoadUserAndRelatedDataFromJSONString(json, true));

    te = related.TimeEntries.byID(89818605);
    ASSERT_TRUE(te);
    ASSERT_TRUE(te->IsMarkedAsDeletedOnServer());
}

TEST(Database, LoadUserByEmail) {
    testing::Database db;

    RelatedData related;
    ProtectedModel<User> user { &related, true };
    ASSERT_EQ(noError, user->LoadUserAndRelatedDataFromJSONString(loadTestData(), true));

    std::vector<ModelChange> changes;
    auto lUser = *user;
    ASSERT_EQ(noError, db.instance()->SaveUser(lUser, true, &changes));

    RelatedData related2;
    ProtectedModel<User> user2 { &related2, true };
    auto lUser2 = *user2;
    ASSERT_EQ(noError, db.instance()->LoadUserByEmail("johnsmith@toggl.com", lUser2));

    ASSERT_EQ(user->ID(), user2->ID());
}

TEST(Database, LoadUserByEmailWithoutEmail) {
    testing::Database db;

    RelatedData related;
    ProtectedModel<User> user { &related, true };
    auto lUser = *user;
    ASSERT_NE(noError, db.instance()->LoadUserByEmail("", lUser));
}

TEST(Database, LoadUserByEmailWithNonexistantEmail) {
    testing::Database db;

    RelatedData related;
    ProtectedModel<User> user { &related, true };
    auto lUser = *user;
    ASSERT_EQ(noError, db.instance()->LoadUserByEmail("foo@bar.com", lUser));
}

TEST(Database, AllowsSameEmail) {
    testing::Database db;

    RelatedData related;
    ProtectedModel<User> user { &related, true };
    ASSERT_EQ(noError,
              user->LoadUserAndRelatedDataFromJSONString(loadTestData(), true));

    std::vector<ModelChange> changes;
    auto lUser = *user;
    ASSERT_EQ(noError, db.instance()->SaveUser(lUser, true, &changes));

    RelatedData related2;
    ProtectedModel<User> user2 { &related2, true };
    std::string json = loadTestDataFile(std::string("../testdata/same_email.json"));
    ASSERT_EQ(noError,
              user2->LoadUserAndRelatedDataFromJSONString(json, true));

    auto lUser2 = *user2;
    ASSERT_EQ(noError, db.instance()->SaveUser(lUser2, true, &changes));

    ASSERT_EQ(user->Email(), user2->Email());
    ASSERT_NE(user->ID(), user2->ID());
    ASSERT_NE(user->APIToken(), user2->APIToken());
}

TEST(Formatter, EscapeControlCharactersInJSONString) {
    std::string text("\x16");
    ASSERT_EQ(" ", Formatter::EscapeJSONString(text));
}

TEST(User, UpdatesTimeEntryFromFullUserJSON) {
    testing::Database db;

    std::string json = loadTestData();

    RelatedData related;
    ProtectedModel<User> user { &related, true };
    ASSERT_EQ(noError,
              user->LoadUserAndRelatedDataFromJSONString(loadTestData(), true));

    auto te = related.TimeEntries.byID(89818605);
    ASSERT_TRUE(te);

    size_t n = json.find("Important things");
    ASSERT_TRUE(n);

    te->SetDescription("Even more important!");

    ASSERT_EQ(noError,
              user->LoadUserAndRelatedDataFromJSONString(json, true));
    te = related.TimeEntries.byID(89818605);
    ASSERT_TRUE(te);
    ASSERT_EQ("Even more important!", te->Description());
}

TEST(Database, SavesAndLoadsUserFields) {
    testing::Database db;

    RelatedData related;
    ProtectedModel<User> user { &related, true };
    ASSERT_EQ(noError,
              user->LoadUserAndRelatedDataFromJSONString(loadTestData(), true));

    ASSERT_TRUE(user->StoreStartAndStopTime());
    // Change fields
    user->SetStoreStartAndStopTime(false);

    std::vector<ModelChange> changes;
    auto lUser = *user;
    ASSERT_EQ(noError, db.instance()->SaveUser(lUser, true, &changes));

    // Load user into another instance
    RelatedData related2;
    ProtectedModel<User> user2 { &related2, true };
    auto lUser2 = *user2;
    ASSERT_EQ(noError, db.instance()->LoadUserByID(user->ID(), lUser2));
    ASSERT_FALSE(user2->StoreStartAndStopTime());

    // Change fields, again
    user->SetStoreStartAndStopTime(true);
    ASSERT_EQ(noError, db.instance()->SaveUser(lUser, true, &changes));

    // Load user into another instance
    RelatedData related3;
    ProtectedModel<User> user3 { &related3, true };
    auto lUser3 = *user3;
    ASSERT_EQ(noError, db.instance()->LoadUserByID(user->ID(), lUser3));
    ASSERT_TRUE(user3->StoreStartAndStopTime());
}

TEST(Database, SavesAndLoadsObmExperiments) {
    testing::Database db;

    RelatedData related;
    ProtectedModel<User> user { &related, true };
    ASSERT_EQ(noError,
              user->LoadUserAndRelatedDataFromJSONString(loadTestData(), true));

    std::string json = loadTestDataFile(std::string("../testdata/obm_response.json"));
    Json::Value data = jsonStringToValue(json);
    user->LoadObmExperiments(data);

    ASSERT_EQ(1, related.ObmExperiments.size());

    auto obm = related.ObmExperiments[0];
    ASSERT_TRUE(obm->Included());
    ASSERT_EQ(74, obm->Nr());
    ASSERT_EQ("stringarray/hasopmitempty/canbemissing", obm->Actions());

    std::vector<ModelChange> changes;
    auto lUser = *user;
    ASSERT_EQ(noError, db.instance()->SaveUser(lUser, true, &changes));

    // Load user into another instance
    RelatedData related2;
    ProtectedModel<User> user2 { &related2, true };
    auto lUser2 = *user2;
    ASSERT_EQ(noError, db.instance()->LoadUserByID(user->ID(), lUser2));
    ASSERT_EQ(related.ObmExperiments.size(),
              related2.ObmExperiments.size());


    obm = related.ObmExperiments[0];
    ASSERT_TRUE(obm->LocalID());
}

TEST(Database, SavesAndLoadsObmExperimentsArray) {
    testing::Database db;

    RelatedData related;
    ProtectedModel<User> user { &related, true };
    ASSERT_EQ(noError,
              user->LoadUserAndRelatedDataFromJSONString(loadTestData(), true));

    std::string json = loadTestDataFile(std::string("../testdata/obm_response_array.json"));
    Json::Value data = jsonStringToValue(json);
    user->LoadObmExperiments(data);

    ASSERT_EQ(2, related.ObmExperiments.size());

    auto obm = related.ObmExperiments[0];
    ASSERT_TRUE(obm->Included());
    ASSERT_EQ(74, obm->Nr());
    ASSERT_EQ("stringarray/hasopmitempty/canbemissing", obm->Actions());

    obm = related.ObmExperiments[1];
    ASSERT_FALSE(obm->Included());
    ASSERT_EQ(73, obm->Nr());
    ASSERT_EQ("blah", obm->Actions());

    std::vector<ModelChange> changes;
    auto lUser = *user;
    ASSERT_EQ(noError, db.instance()->SaveUser(lUser, true, &changes));

    // Load user into another instance
    RelatedData related2;
    ProtectedModel<User> user2 { &related, true };
    auto lUser2 = *user2;
    ASSERT_EQ(noError, db.instance()->LoadUserByID(user->ID(), lUser2));
    ASSERT_EQ(related.ObmExperiments.size(),
              related2.ObmExperiments.size());
}

TEST(Database, SavesModelsAndKnowsToUpdateWithSameUserInstance) {
    testing::Database db;

    RelatedData related;
    ProtectedModel<User> user { &related, true };
    ASSERT_EQ(noError,
              user->LoadUserAndRelatedDataFromJSONString(loadTestData(), true));

    Poco::UInt64 n;
    ASSERT_EQ(noError, db.instance()->UInt("select count(1) from users", &n));
    ASSERT_EQ(Poco::UInt64(0), n);

    auto lUser = *user;
    for (int i = 0; i < 3; i++) {
        std::vector<ModelChange> changes;
        ASSERT_EQ(noError, db.instance()->SaveUser(lUser, true, &changes));

        ASSERT_EQ(noError,
                  db.instance()->UInt("select count(1) from users", &n));
        ASSERT_EQ(Poco::UInt64(1), n);

        ASSERT_EQ(noError,
                  db.instance()->UInt("select count(1) from workspaces", &n));
        ASSERT_EQ(uint(2), n);

        ASSERT_EQ(noError,
                  db.instance()->UInt("select count(1) from clients", &n));
        ASSERT_EQ(uint(1), n);  // 2 clients in json, but one is deleted

        ASSERT_EQ(noError,
                  db.instance()->UInt("select count(1) from projects", &n));
        ASSERT_EQ(uint(2), n);

        ASSERT_EQ(noError,
                  db.instance()->UInt("select count(1) from tasks", &n));
        ASSERT_EQ(uint(2), n);

        ASSERT_EQ(noError,
                  db.instance()->UInt("select count(1) from tags", &n));
        ASSERT_EQ(uint(2), n);

        ASSERT_EQ(noError,
                  db.instance()->UInt("select count(1) from time_entries",
                                      &n));
        ASSERT_EQ(uint(5), n);
    }
}

TEST(Database,
     SavesModelsAndKnowsToUpdateWithSeparateUserInstances) {
    testing::Database db;

    std::string json = loadTestData();

    RelatedData related1;
    ProtectedModel<User> user1 { &related1, true };
    ASSERT_EQ(noError,
              user1->LoadUserAndRelatedDataFromJSONString(json, true));

    std::vector<ModelChange> changes;
    auto lUser1 = *user1;
    ASSERT_EQ(noError, db.instance()->SaveUser(lUser1, true, &changes));

    Poco::UInt64 n;
    ASSERT_EQ(noError, db.instance()->UInt("select count(1) from users", &n));
    ASSERT_EQ(Poco::UInt64(1), n);

    ASSERT_EQ(noError,
              db.instance()->UInt("select count(1) from workspaces", &n));
    ASSERT_EQ(uint(2), n);

    ASSERT_EQ(noError,
              db.instance()->UInt("select count(1) from clients", &n));
    ASSERT_EQ(uint(1), n);  // 2 clients in JSON but one is deleted

    ASSERT_EQ(noError,
              db.instance()->UInt("select count(1) from projects", &n));
    ASSERT_EQ(uint(2), n);

    ASSERT_EQ(noError,
              db.instance()->UInt("select count(1) from tasks", &n));
    ASSERT_EQ(uint(2), n);

    ASSERT_EQ(noError,
              db.instance()->UInt("select count(1) from tags", &n));
    ASSERT_EQ(uint(2), n);

    ASSERT_EQ(noError,
              db.instance()->UInt("select count(1) from time_entries", &n));
    ASSERT_EQ(Poco::UInt64(5), n);

    RelatedData related2;
    ProtectedModel<User> user2 { &related2, true };
    auto lUser2 = *user2;
    ASSERT_EQ(noError, db.instance()->LoadUserByID(user1->ID(), lUser2));

    ASSERT_EQ(related1.Workspaces.size(),
              related2.Workspaces.size());
    ASSERT_EQ(related1.Clients.size(),
              related2.Clients.size());
    ASSERT_EQ(related1.Projects.size(),
              related2.Projects.size());
    ASSERT_EQ(related1.Tasks.size(),
              related2.Tasks.size());
    ASSERT_EQ(related1.Tags.size(),
              related2.Tags.size());
    ASSERT_EQ(related1.TimeEntries.size(),
              related2.TimeEntries.size());

    ASSERT_EQ(noError,
              user2->LoadUserAndRelatedDataFromJSONString(json, true));

    ASSERT_EQ(noError, db.instance()->SaveUser(lUser2, true, &changes));

    ASSERT_EQ(noError, db.instance()->UInt("select count(1) from users", &n));
    ASSERT_EQ(Poco::UInt64(1), n);

    ASSERT_EQ(noError,
              db.instance()->UInt("select count(1) from workspaces", &n));
    ASSERT_EQ(uint(2), n);

    ASSERT_EQ(noError,
              db.instance()->UInt("select count(1) from clients", &n));
    ASSERT_EQ(uint(1), n);  // 2 clients in JSON but 1 is deleted

    ASSERT_EQ(noError,
              db.instance()->UInt("select count(1) from projects", &n));
    ASSERT_EQ(uint(2), n);

    ASSERT_EQ(noError, db.instance()->UInt("select count(1) from tasks", &n));
    ASSERT_EQ(uint(2), n);

    ASSERT_EQ(noError, db.instance()->UInt("select count(1) from tags", &n));
    ASSERT_EQ(uint(2), n);

    ASSERT_EQ(noError,
              db.instance()->UInt("select count(1) from time_entries", &n));
    ASSERT_EQ(uint(5), n);
}

TEST(User, TestStartTimeEntryWithDuration) {
    testing::Database db;

    RelatedData related;
    ProtectedModel<User> user { &related, true };
    ASSERT_EQ(noError,
              user->LoadUserAndRelatedDataFromJSONString(loadTestData(), true));

    size_t count = related.TimeEntries.size();

    user->Start("Old work", "1 hour", 0, 0, "", "", 0, 0, true);

    ASSERT_EQ(count + 1, related.TimeEntries.size());

    auto te = related.TimeEntries[related.TimeEntries.size()-1];

    ASSERT_EQ(3600, te->DurationInSeconds());
}

TEST(User, TestStartTimeEntryWithoutDuration) {
    testing::Database db;

    RelatedData related;
    ProtectedModel<User> user { &related, true };
    ASSERT_EQ(noError,
              user->LoadUserAndRelatedDataFromJSONString(loadTestData(), true));

    user->Start("Old work", "", 0, 0, "", "", 0, 0, true);

    auto te = user->RunningTimeEntry();
    ASSERT_TRUE(te);
    ASSERT_GT(0, te->DurationInSeconds());
}

TEST(User, TestDeletionSteps) {
    testing::Database db;

    RelatedData related;
    ProtectedModel<User> user { &related, true };
    ASSERT_EQ(noError,
              user->LoadUserAndRelatedDataFromJSONString(loadTestData(), true));

    // first, mark time entry as deleted
    user->Start("My new time entry", "", 0, 0, "", "", 0, 0, true);
    auto te = user->RunningTimeEntry();
    ASSERT_TRUE(te);
    std::vector<ModelChange> changes;
    auto lUser = *user;
    ASSERT_EQ(noError, db.instance()->SaveUser(lUser, true, &changes));
    te->MarkAsDeletedOnServer();
    {
        Poco::UInt64 te_count(0);
        std::stringstream query;
        query << "select count(1) from time_entries where local_id = "
              << te->LocalID();
        ASSERT_EQ(noError, db.instance()->UInt(query.str(), &te_count));
        ASSERT_EQ(Poco::UInt64(1), te_count);
    }

    // now, really delete it
    te->MarkAsDeletedOnServer();
    ASSERT_EQ(noError, db.instance()->SaveUser(lUser, true, &changes));
    {
        Poco::UInt64 te_count(0);
        std::stringstream query;
        query << "select count(1) from time_entries where local_id = "
              << te->LocalID();
        ASSERT_EQ(noError, db.instance()->UInt(query.str(), &te_count));
        ASSERT_EQ(Poco::UInt64(0), te_count);
    }
}

TEST(Database, SavesModels) {
    RelatedData related;
    ProtectedModel<User> user { &related, true };
    ASSERT_EQ(noError,
              user->LoadUserAndRelatedDataFromJSONString(loadTestData(), true));

    testing::Database db;

    std::vector<ModelChange> changes;
    auto lUser = *user;
    ASSERT_EQ(noError, db.instance()->SaveUser(lUser, false, &changes));

    ASSERT_EQ(noError, db.instance()->SaveUser(lUser, false, &changes));
}

TEST(Database, AssignsGUID) {
    std::string json = loadTestData();
    ASSERT_FALSE(json.empty());

    RelatedData related;
    ProtectedModel<User> user { &related, true };
    ASSERT_EQ(noError,
              user->LoadUserAndRelatedDataFromJSONString(json, true));

    ASSERT_EQ(uint(5), related.TimeEntries.size());
    auto te = related.TimeEntries.byID(89837445);
    ASSERT_TRUE(te);

    ASSERT_NE("", te->GUID());
    ASSERT_TRUE(te->GUID().size());

    auto te2 = related.TimeEntries.byGUID(te->GUID());
    ASSERT_TRUE(te2);

    ASSERT_EQ(te->GUID(), te2->GUID());
    ASSERT_EQ(te->ID(), te2->ID());
}

TEST(User, ParsesAndSavesData) {
    std::string json = loadTestData();
    ASSERT_FALSE(json.empty());

    RelatedData related;
    ProtectedModel<User> user { &related, true };
    auto lUser = *user;
    ASSERT_EQ(noError,
              user->LoadUserAndRelatedDataFromJSONString(json, true));
    ASSERT_EQ(Poco::UInt64(1379068550), user->Since());
    ASSERT_EQ(Poco::UInt64(10471231), user->ID());
    ASSERT_EQ(Poco::UInt64(123456788), user->DefaultWID());
    ASSERT_EQ("30eb0ae954b536d2f6628f7fec47beb6", user->APIToken());
    ASSERT_EQ("John Smith", user->Fullname());

    // Projects
    ASSERT_EQ(uint(2), related.Projects.size());

    ASSERT_EQ(Poco::UInt64(2567324), related.Projects[0]->ID());
    //ASSERT_EQ("2f0b8f51-f898-d992-3e1a-6bc261fc41xf", related.Projects[0]->GUID());
    ASSERT_EQ(Poco::UInt64(123456789), related.Projects[0]->WID());
    ASSERT_EQ("Even more work", related.Projects[0]->Name());
    ASSERT_EQ("#999999", related.Projects[0]->Color());
    ASSERT_EQ(Poco::UInt64(1385144), related.Projects[0]->CID());
    ASSERT_EQ(user->ID(), related.Projects[0]->UID());
    ASSERT_TRUE(related.Projects[0]->Active());

    ASSERT_EQ(Poco::UInt64(2598305), related.Projects[1]->ID());
    ASSERT_EQ(Poco::UInt64(123456789), related.Projects[1]->WID());
    ASSERT_EQ("Testing stuff", related.Projects[1]->Name());
    ASSERT_EQ("#fb8b14", related.Projects[1]->Color());
    ASSERT_EQ(user->ID(), related.Projects[1]->UID());
    ASSERT_FALSE(related.Projects[1]->Active());

    // Time entries
    ASSERT_EQ(uint(5), related.TimeEntries.size());

    ASSERT_EQ(Poco::UInt64(89818605), related.TimeEntries[0]->ID());
    //ASSERT_EQ("07fba193-91c4-0ec8-2894-820df0548a8f", related.TimeEntries[0]->GUID());
    ASSERT_EQ(uint(2567324), related.TimeEntries[0]->PID());
    ASSERT_TRUE(related.TimeEntries[0]->Billable());
    ASSERT_EQ(uint(1378362830), related.TimeEntries[0]->Start());
    ASSERT_EQ(uint(1378369186), related.TimeEntries[0]->Stop());
    ASSERT_EQ(6356, related.TimeEntries[0]->DurationInSeconds());
    ASSERT_EQ("Important things",
              related.TimeEntries[0]->Description());
    ASSERT_EQ(uint(0), related.TimeEntries[0]->TagNames.size());
    ASSERT_FALSE(related.TimeEntries[0]->DurOnly());
    ASSERT_EQ(user->ID(), related.TimeEntries[0]->UID());

    ASSERT_EQ(uint(2), related.Tasks.size());

    ASSERT_EQ(uint(1894794), related.Tasks[0]->ID());
    ASSERT_EQ("blog (writing)", related.Tasks[0]->Name());
    ASSERT_EQ(uint(123456789), related.Tasks[0]->WID());
    ASSERT_EQ(uint(2598305), related.Tasks[0]->PID());
    ASSERT_EQ(user->ID(), related.Tasks[0]->UID());

    // Tags
    ASSERT_EQ(uint(2), related.Tags.size());

    ASSERT_EQ(uint(27457022), related.Tags[0]->ID());
    ASSERT_EQ("billed", related.Tags[0]->Name());
    ASSERT_EQ(user->ID(), related.Tags[0]->UID());
    ASSERT_EQ(uint(123456788), related.Tags[0]->WID());
    //ASSERT_EQ("", related.Tags[0]->GUID());

    ASSERT_EQ(uint(36253522), related.Tags[1]->ID());
    ASSERT_EQ("create new", related.Tags[1]->Name());
    ASSERT_EQ(user->ID(), related.Tags[1]->UID());
    ASSERT_EQ(uint(123456788), related.Tags[1]->WID());
    //ASSERT_EQ("041390ba-ed9c-b477-b949-1a4ebb60a9ce", related.Tags[1]->GUID());

    // Workspaces
    ASSERT_EQ(uint(2), related.Workspaces.size());

    ASSERT_EQ(uint(123456788), related.Workspaces[0]->ID());
    ASSERT_EQ("stuff", related.Workspaces[0]->Name());
    ASSERT_EQ(user->ID(), related.Workspaces[0]->UID());

    ASSERT_FALSE(related.Workspaces[0]->OnlyAdminsMayCreateProjects());
    ASSERT_TRUE(related.Workspaces[0]->Admin());

    ASSERT_TRUE(related.Workspaces[1]->OnlyAdminsMayCreateProjects());
    ASSERT_TRUE(related.Workspaces[1]->Admin());
    ASSERT_EQ(uint(123456789), related.Workspaces[1]->ID());

    // Clients (2 in JSON but 1 is deleted)
    ASSERT_EQ(uint(1), related.Clients.size());

    ASSERT_EQ(uint(878318), related.Clients[0]->ID());
    ASSERT_EQ(uint(123456788), related.Clients[0]->WID());
    ASSERT_EQ("Big Client", related.Clients[0]->Name());
    //ASSERT_EQ("59b464cd-0f8e-e601-ff44-f135225a6738", related.Clients[0]->GUID());
    ASSERT_EQ(user->ID(), related.Clients[0]->UID());

    testing::Database db;

    Poco::UInt64 n;
    ASSERT_EQ(noError, db.instance()->UInt("select count(1) from users", &n));
    ASSERT_EQ(Poco::UInt64(0), n);

    // Insert
    std::vector<ModelChange> changes;
    ASSERT_EQ(noError, db.instance()->SaveUser(lUser, true, &changes));
    ASSERT_GT(user->LocalID(), uint(0));
    ASSERT_GT(user->ID(), uint(0));
    ASSERT_FALSE(user->APIToken().empty());

    ASSERT_EQ(noError,
              db.instance()->UInt("select count(1) from users", &n));
    ASSERT_EQ(Poco::UInt64(1), n);

    ASSERT_EQ(noError,
              db.instance()->UInt("select count(1) from workspaces", &n));
    ASSERT_EQ(Poco::UInt64(related.Workspaces.size()), n);

    ASSERT_EQ(noError,
              db.instance()->UInt("select count(1) from clients", &n));
    ASSERT_EQ(Poco::UInt64(related.Clients.size()), n);

    ASSERT_EQ(noError,
              db.instance()->UInt("select count(1) from projects", &n));
    ASSERT_EQ(Poco::UInt64(related.Projects.size()), n);

    ASSERT_EQ(noError, db.instance()->UInt("select count(1) from tasks", &n));
    ASSERT_EQ(Poco::UInt64(related.Tasks.size()), n);

    ASSERT_EQ(noError, db.instance()->UInt("select count(1) from tags", &n));
    ASSERT_EQ(Poco::UInt64(related.Tags.size()), n);

    ASSERT_EQ(noError,
              db.instance()->UInt("select count(1) from time_entries", &n));
    ASSERT_EQ(Poco::UInt64(related.TimeEntries.size()), n);

    // Update
    ASSERT_EQ(noError, db.instance()->SaveUser(lUser, true, &changes));
    ASSERT_EQ(noError, db.instance()->UInt("select count(1) from users", &n));
    ASSERT_EQ(Poco::UInt64(1), n);

    // Select
    RelatedData related2;
    ProtectedModel<User> user2 { &related2, true };
    auto lUser2 = *user2;
    ASSERT_EQ(noError, db.instance()->LoadUserByID(user->ID(), lUser2));

    ASSERT_TRUE(user2->ID());
    ASSERT_EQ(user->ID(), user2->ID());
    ASSERT_TRUE(user2->Since());
    ASSERT_EQ(user->Since(), user2->Since());
    ASSERT_EQ(user->ID(), user2->ID());
    ASSERT_EQ(user->DefaultWID(), user2->DefaultWID());

    ASSERT_EQ(uint(2), related2.Projects.size());
    auto project_from_db = related2.Projects.byID(related.Projects[0]->ID());
    ASSERT_TRUE(project_from_db);
    ASSERT_EQ(related.Projects[0]->String(),
              project_from_db->String());
    project_from_db = related2.Projects.byID(related.Projects[1]->ID());
    ASSERT_EQ(related.Projects[1]->String(),
              project_from_db->String());

    ASSERT_EQ(uint(5), related2.TimeEntries.size());
    auto te_from_db = related2.TimeEntries.byID(related.TimeEntries[0]->ID());
    ASSERT_TRUE(te_from_db);
    ASSERT_EQ(related.TimeEntries[0]->String(), te_from_db->String());
    te_from_db = related2.TimeEntries.byID(related.TimeEntries[1]->ID());
    ASSERT_TRUE(te_from_db);
    ASSERT_EQ(related.TimeEntries[1]->String(), te_from_db->String());
    te_from_db = related2.TimeEntries.byID(related.TimeEntries[2]->ID());
    ASSERT_TRUE(te_from_db);
    ASSERT_EQ(related.TimeEntries[2]->String(), te_from_db->String());

    ASSERT_EQ(uint(2), related2.Workspaces.size());
    auto ws_from_db = related2.Workspaces.byID(related.Workspaces[0]->ID());
    ASSERT_TRUE(ws_from_db);
    ASSERT_EQ(related.Workspaces[0]->String(), ws_from_db->String());
    ws_from_db = related2.Workspaces.byID(related.Workspaces[1]->ID());
    ASSERT_TRUE(ws_from_db);
    ASSERT_EQ(related.Workspaces[1]->String(), ws_from_db->String());

    ASSERT_EQ(uint(2), related2.Tasks.size());
    auto task_from_db = related2.Tasks.byID(related2.Tasks[0]->ID());
    ASSERT_EQ(related.Tasks[0]->String(), task_from_db->String());
    ASSERT_EQ(related.Tasks[1]->String(),
              related2.Tasks[1]->String());

    ASSERT_EQ(uint(1), related2.Clients.size());
    ASSERT_EQ(related.Clients[0]->String(),
              related2.Clients[0]->String());

    // Delete
    ASSERT_EQ(noError, db.instance()->DeleteUser(lUser, true));

    ASSERT_EQ(noError, db.instance()->UInt("select count(1) from users", &n));
    ASSERT_EQ(Poco::UInt64(0), n);

    ASSERT_EQ(noError,
              db.instance()->UInt("select count(1) from projects", &n));
    ASSERT_EQ(Poco::UInt64(0), n);

    ASSERT_EQ(noError,
              db.instance()->UInt("select count(1) from workspaces", &n));
    ASSERT_EQ(Poco::UInt64(0), n);

    ASSERT_EQ(noError,
              db.instance()->UInt("select count(1) from tasks", &n));
    ASSERT_EQ(Poco::UInt64(0), n);

    ASSERT_EQ(noError,
              db.instance()->UInt("select count(1) from time_entries", &n));
    ASSERT_EQ(Poco::UInt64(0), n);

    ASSERT_EQ(noError, db.instance()->UInt("select count(1) from tags", &n));
    ASSERT_EQ(Poco::UInt64(0), n);

    ASSERT_EQ(noError, db.instance()->UInt("select count(1) from clients", &n));
    ASSERT_EQ(Poco::UInt64(0), n);
}

TEST(TimeEntry, ParsesDurationLikeOnTheWeb) {
    ProtectedModel<TimeEntry> te { nullptr, true };

    te->SetDurationUserInput("00:00:15");
    ASSERT_EQ("0:00:15",
              toggl::Formatter::FormatDuration(te->DurationInSeconds(),
                      toggl::Format::Improved));

    te->SetDurationInSeconds(0);
    te->SetDurationUserInput("00:23:15");
    ASSERT_EQ("0:23:15",
              toggl::Formatter::FormatDuration(te->DurationInSeconds(),
                      toggl::Format::Improved));

    te->SetDurationInSeconds(0);
    te->SetDurationUserInput("12:34:56");
    ASSERT_EQ("12:34:56",
              toggl::Formatter::FormatDuration(te->DurationInSeconds(),
                      toggl::Format::Improved));

    te->SetDurationInSeconds(0);
    te->SetDurationUserInput("0:1");
    ASSERT_EQ("0:01:00",
              toggl::Formatter::FormatDuration(te->DurationInSeconds(),
                      toggl::Format::Improved));

    te->SetDurationInSeconds(0);
    te->SetDurationUserInput("1:2");
    ASSERT_EQ("1:02:00",
              toggl::Formatter::FormatDuration(te->DurationInSeconds(),
                      toggl::Format::Improved));

    te->SetDurationInSeconds(0);
    te->SetDurationUserInput("1:0");
    ASSERT_EQ("1:00:00",
              toggl::Formatter::FormatDuration(te->DurationInSeconds(),
                      toggl::Format::Improved));

    te->SetDurationInSeconds(0);
    te->SetDurationUserInput("05:22 min");
    ASSERT_EQ("0:05:22",
              toggl::Formatter::FormatDuration(te->DurationInSeconds(),
                      toggl::Format::Improved));

    te->SetDurationInSeconds(0);
    te->SetDurationUserInput("00:22 min");
    ASSERT_EQ("0:00:22",
              toggl::Formatter::FormatDuration(te->DurationInSeconds(),
                      toggl::Format::Improved));

    te->SetDurationInSeconds(0);
    te->SetDurationUserInput("0 hours");
    ASSERT_EQ("0:00:00",
              toggl::Formatter::FormatDuration(te->DurationInSeconds(),
                      toggl::Format::Improved));

    te->SetDurationInSeconds(0);
    te->SetDurationUserInput("0.5 hours");
    ASSERT_EQ("0:30:00",
              toggl::Formatter::FormatDuration(te->DurationInSeconds(),
                      toggl::Format::Improved));

    te->SetDurationInSeconds(0);
    te->SetDurationUserInput("0,5 hours");
    ASSERT_EQ("0:30:00",
              toggl::Formatter::FormatDuration(te->DurationInSeconds(),
                      toggl::Format::Improved));

    te->SetDurationInSeconds(0);
    te->SetDurationUserInput("1 hour");
    ASSERT_EQ("1:00:00",
              toggl::Formatter::FormatDuration(te->DurationInSeconds(),
                      toggl::Format::Improved));

    te->SetDurationInSeconds(0);
    te->SetDurationUserInput("1 hr");
    ASSERT_EQ("1:00:00",
              toggl::Formatter::FormatDuration(te->DurationInSeconds(),
                      toggl::Format::Improved));

    te->SetDurationInSeconds(0);
    te->SetDurationUserInput("1,5 hours");
    ASSERT_EQ("1:30:00",
              toggl::Formatter::FormatDuration(te->DurationInSeconds(),
                      toggl::Format::Improved));

    te->SetDurationInSeconds(0);
    te->SetDurationUserInput("1.5 hours");
    ASSERT_EQ("1:30:00",
              toggl::Formatter::FormatDuration(te->DurationInSeconds(),
                      toggl::Format::Improved));

    te->SetDurationInSeconds(0);
    te->SetDurationUserInput("24 hours");
    ASSERT_EQ(86400, te->DurationInSeconds());

    te->SetDurationInSeconds(0);
    te->SetDurationUserInput("0 minutes");
    ASSERT_EQ("0:00:00",
              toggl::Formatter::FormatDuration(te->DurationInSeconds(),
                      toggl::Format::Improved));

    te->SetDurationInSeconds(0);
    te->SetDurationUserInput("0 min");
    ASSERT_EQ("0:00:00",
              toggl::Formatter::FormatDuration(te->DurationInSeconds(),
                      toggl::Format::Improved));

    te->SetDurationInSeconds(0);
    te->SetDurationUserInput("5 minutes");
    ASSERT_EQ("0:05:00",
              toggl::Formatter::FormatDuration(te->DurationInSeconds(),
                      toggl::Format::Improved));

    te->SetDurationInSeconds(0);
    te->SetDurationUserInput("5minutes");
    ASSERT_EQ("0:05:00",
              toggl::Formatter::FormatDuration(te->DurationInSeconds(),
                      toggl::Format::Improved));

    te->SetDurationInSeconds(0);
    te->SetDurationUserInput("0,5 minutes");
    ASSERT_EQ("0:00:30",
              toggl::Formatter::FormatDuration(te->DurationInSeconds(),
                      toggl::Format::Improved));

    te->SetDurationInSeconds(0);
    te->SetDurationUserInput("1 minute");
    ASSERT_EQ("0:01:00",
              toggl::Formatter::FormatDuration(te->DurationInSeconds(),
                      toggl::Format::Improved));

    te->SetDurationInSeconds(0);
    te->SetDurationUserInput("1,5 minutes");
    ASSERT_EQ("0:01:30",
              toggl::Formatter::FormatDuration(te->DurationInSeconds(),
                      toggl::Format::Improved));

    te->SetDurationInSeconds(0);
    te->SetDurationUserInput("1.5 minutes");
    ASSERT_EQ("0:01:30",
              toggl::Formatter::FormatDuration(te->DurationInSeconds(),
                      toggl::Format::Improved));

    te->SetDurationInSeconds(0);
    te->SetDurationUserInput("15");
    ASSERT_EQ("0:15:00",
              toggl::Formatter::FormatDuration(te->DurationInSeconds(),
                      toggl::Format::Improved));

    te->SetDurationInSeconds(0);
    te->SetDurationUserInput("0 seconds");
    ASSERT_EQ("0:00:00",
              toggl::Formatter::FormatDuration(te->DurationInSeconds(),
                      toggl::Format::Improved));

    te->SetDurationInSeconds(0);
    te->SetDurationUserInput("1 second");
    ASSERT_EQ("0:00:01",
              toggl::Formatter::FormatDuration(te->DurationInSeconds(),
                      toggl::Format::Improved));

    te->SetDurationInSeconds(0);
    te->SetDurationUserInput("1.5h");
    ASSERT_EQ("1:30:00",
              toggl::Formatter::FormatDuration(te->DurationInSeconds(),
                      toggl::Format::Improved));

    te->SetDurationInSeconds(0);
    te->SetDurationUserInput("1.5 h");
    ASSERT_EQ("1:30:00",
              toggl::Formatter::FormatDuration(te->DurationInSeconds(),
                      toggl::Format::Improved));

    te->SetDurationInSeconds(0);
    te->SetDurationUserInput("3h");
    ASSERT_EQ("3:00:00",
              toggl::Formatter::FormatDuration(te->DurationInSeconds(),
                      toggl::Format::Improved));

    te->SetDurationInSeconds(0);
    te->SetDurationUserInput("3 h");
    ASSERT_EQ("3:00:00",
              toggl::Formatter::FormatDuration(te->DurationInSeconds(),
                      toggl::Format::Improved));

    te->SetDurationInSeconds(0);
    te->SetDurationUserInput("15m");
    ASSERT_EQ("0:15:00",
              toggl::Formatter::FormatDuration(te->DurationInSeconds(),
                      toggl::Format::Improved));

    te->SetDurationInSeconds(0);
    te->SetDurationUserInput("15 m");
    ASSERT_EQ("0:15:00",
              toggl::Formatter::FormatDuration(te->DurationInSeconds(),
                      toggl::Format::Improved));

    te->SetDurationInSeconds(0);
    te->SetDurationUserInput("25s");
    ASSERT_EQ("0:00:25",
              toggl::Formatter::FormatDuration(te->DurationInSeconds(),
                      toggl::Format::Improved));

    te->SetDurationInSeconds(0);
    te->SetDurationUserInput("25 s");
    ASSERT_EQ("0:00:25",
              toggl::Formatter::FormatDuration(te->DurationInSeconds(),
                      toggl::Format::Improved));

    te->SetDurationInSeconds(0);
    te->SetDurationUserInput("1.5");
    ASSERT_EQ("1:30:00",
              toggl::Formatter::FormatDuration(te->DurationInSeconds(),
                      toggl::Format::Improved));

    te->SetDurationInSeconds(0);
    te->SetDurationUserInput("1,5");
    ASSERT_EQ("1:30:00",
              toggl::Formatter::FormatDuration(te->DurationInSeconds(),
                      toggl::Format::Improved));

    te->SetDurationInSeconds(0);
    te->SetDurationUserInput("0.25");
    ASSERT_EQ("0:15:00",
              toggl::Formatter::FormatDuration(te->DurationInSeconds(),
                      toggl::Format::Improved));

    te->SetDurationInSeconds(0);
    te->SetDurationUserInput("0.025");
    ASSERT_EQ("0:01:30",
              toggl::Formatter::FormatDuration(te->DurationInSeconds(),
                      toggl::Format::Improved));

    te->SetDurationInSeconds(0);
    te->SetDurationUserInput("2h45");
    ASSERT_EQ("2:45:00",
              toggl::Formatter::FormatDuration(te->DurationInSeconds(),
                      toggl::Format::Improved));

    te->SetDurationInSeconds(0);
    te->SetDurationUserInput("2h");
    ASSERT_EQ("2:00:00",
              toggl::Formatter::FormatDuration(te->DurationInSeconds(),
                      toggl::Format::Improved));

    te->SetDurationInSeconds(0);
    te->SetDurationUserInput("2h 18m");
    ASSERT_EQ("2:18:00",
              toggl::Formatter::FormatDuration(te->DurationInSeconds(),
                      toggl::Format::Improved));

    te->SetDurationInSeconds(0);
    te->SetDurationUserInput("2h 18m 50s");
    ASSERT_EQ("2:18:50",
              toggl::Formatter::FormatDuration(te->DurationInSeconds(),
                      toggl::Format::Improved));

    te->SetDurationInSeconds(0);
    te->SetDurationUserInput("1hr 25min 30sec");
    ASSERT_EQ("1:25:30",
              toggl::Formatter::FormatDuration(te->DurationInSeconds(),
                      toggl::Format::Improved));

    te->SetDurationInSeconds(0);
    te->SetDurationUserInput("1 hours 25 minutes 30 seconds");
    ASSERT_EQ("1:25:30",
              toggl::Formatter::FormatDuration(te->DurationInSeconds(),
                      toggl::Format::Improved));

    te->SetDurationInSeconds(0);
    te->SetDurationUserInput("1 hour 1 minute 1 second");
    ASSERT_EQ("1:01:01",
              toggl::Formatter::FormatDuration(te->DurationInSeconds(),
                      toggl::Format::Improved));
}

TEST(TimeEntry, ParseDurationLargerThan24Hours) {
    ProtectedModel<TimeEntry> te { nullptr, true };

    te->SetDurationInSeconds(0);
    te->SetDurationUserInput("90:10:00");
    ASSERT_EQ("90:10:00",
              toggl::Formatter::FormatDuration(te->DurationInSeconds(),
                      toggl::Format::Improved));
}

TEST(TimeEntry, InterpretsCrazyStartAndStopAsMissingValues) {
    ProtectedModel<TimeEntry> te { nullptr, true };

    ASSERT_EQ(Poco::UInt64(0), te->Start());
    te->SetStartString("0003-03-16T-7:-19:-24Z");
    ASSERT_EQ(Poco::UInt64(0), te->Start());

    ASSERT_EQ(Poco::UInt64(0), te->Stop());
    te->SetStopString("0003-03-16T-5:-52:-51Z");
    ASSERT_EQ(Poco::UInt64(0), te->Stop());
}

TEST(User, Continue) {
    testing::Database db;

    RelatedData related;
    ProtectedModel<User> user { &related, true };
    ASSERT_EQ(noError,
              user->LoadUserAndRelatedDataFromJSONString(loadTestData(), true));

    auto te = related.TimeEntries.byID(89818605);
    ASSERT_TRUE(te);
    size_t count = related.TimeEntries.size();
    ASSERT_TRUE(user->Continue(te->GUID(), false));
    ASSERT_EQ(count + 1, related.TimeEntries.size());
}

TEST(TimeEntry, SetDurationOnRunningTimeEntryWithDurOnlySetting) {
    testing::Database db;

    std::string json = loadTestDataFile(std::string("../testdata/user_with_duronly.json"));

    RelatedData related;
    ProtectedModel<User> user { &related, true };
    ASSERT_EQ(noError,
              user->LoadUserAndRelatedDataFromJSONString(json, true));

    auto te = related.TimeEntries.byID(164891639);
    ASSERT_TRUE(te);
    ASSERT_TRUE(te->IsTracking());
    ASSERT_LT(te->Start(), te->Stop());

    te->SetDurationUserInput("00:59:47");
    ASSERT_TRUE(te->IsTracking());
    ASSERT_LT(te->Start(), te->Stop());

    te->StopTracking();
    ASSERT_FALSE(te->IsTracking());
    ASSERT_LT(te->Start(), te->Stop());
}

TEST(Formatter, CollectErrors) {
    {
        std::vector<error> errors;
        errors.push_back(error("foo"));
        errors.push_back(error("bar"));
        errors.push_back(error("foo"));
        error err = Formatter::CollectErrors(&errors);
        ASSERT_EQ("Errors encountered while syncing data: foo bar", err);
    }

    {
        std::vector<error> errors;
        errors.push_back(error("foo\n"));
        errors.push_back(error("bar\n"));
        errors.push_back(error("foo\n"));
        error err = Formatter::CollectErrors(&errors);
        ASSERT_EQ("Errors encountered while syncing data: foo. bar.", err);
    }
}

TEST(Formatter, ParseTimeInput) {
    int hours = 0;
    int minutes = 0;

    ASSERT_FALSE(Formatter::ParseTimeInput("", &hours, &minutes));
    ASSERT_EQ(0, hours);
    ASSERT_EQ(0, minutes);

    ASSERT_TRUE(Formatter::ParseTimeInput("120a", &hours, &minutes));
    ASSERT_EQ(1, hours);
    ASSERT_EQ(20, minutes);

    ASSERT_TRUE(Formatter::ParseTimeInput("120a", &hours, &minutes));
    ASSERT_EQ(1, hours);
    ASSERT_EQ(20, minutes);

    ASSERT_TRUE(Formatter::ParseTimeInput("1P", &hours, &minutes));
    ASSERT_EQ(13, hours);
    ASSERT_EQ(0, minutes);

    ASSERT_FALSE(Formatter::ParseTimeInput("x", &hours, &minutes));

    ASSERT_TRUE(Formatter::ParseTimeInput("2", &hours, &minutes));
    ASSERT_EQ(2, hours);
    ASSERT_EQ(0, minutes);

    ASSERT_TRUE(Formatter::ParseTimeInput("12", &hours, &minutes));
    ASSERT_EQ(12, hours);
    ASSERT_EQ(0, minutes);

    ASSERT_TRUE(Formatter::ParseTimeInput("1230", &hours, &minutes));
    ASSERT_EQ(12, hours);
    ASSERT_EQ(30, minutes);

    ASSERT_FALSE(Formatter::ParseTimeInput("12x", &hours, &minutes));

    ASSERT_FALSE(Formatter::ParseTimeInput("12xx", &hours, &minutes));

    ASSERT_FALSE(Formatter::ParseTimeInput(":", &hours, &minutes));

    ASSERT_FALSE(Formatter::ParseTimeInput("11:", &hours, &minutes));

    ASSERT_FALSE(Formatter::ParseTimeInput(":20", &hours, &minutes));

    ASSERT_FALSE(Formatter::ParseTimeInput("11:xx", &hours, &minutes));

    ASSERT_TRUE(Formatter::ParseTimeInput("11:20", &hours, &minutes));
    ASSERT_EQ(11, hours);
    ASSERT_EQ(20, minutes);

    ASSERT_TRUE(Formatter::ParseTimeInput("5:30", &hours, &minutes));
    ASSERT_EQ(5, hours);
    ASSERT_EQ(30, minutes);

    ASSERT_TRUE(Formatter::ParseTimeInput("5:30 PM", &hours, &minutes));
    ASSERT_EQ(17, hours);
    ASSERT_EQ(30, minutes);

    ASSERT_TRUE(Formatter::ParseTimeInput("5:30 odp.", &hours, &minutes));
    ASSERT_EQ(17, hours);
    ASSERT_EQ(30, minutes);

    ASSERT_TRUE(Formatter::ParseTimeInput("17:10", &hours, &minutes));
    ASSERT_EQ(17, hours);
    ASSERT_EQ(10, minutes);

    ASSERT_TRUE(Formatter::ParseTimeInput("12:00 AM", &hours, &minutes));
    ASSERT_EQ(0, hours);
    ASSERT_EQ(0, minutes);

    ASSERT_TRUE(Formatter::ParseTimeInput("12:00 dop.", &hours, &minutes));
    ASSERT_EQ(0, hours);
    ASSERT_EQ(0, minutes);

    ASSERT_FALSE(Formatter::ParseTimeInput("NOT VALID", &hours, &minutes));
}

TEST(Formatter, FormatTimeForTimeEntryEditor) {
    ASSERT_EQ("", Formatter::FormatTimeForTimeEntryEditor(0));

    //  date -r 1412950844
    //  Fri Oct 10 16:20:44 CEST 2014
    time_t t(1412950844);

    std::string res = Formatter::FormatTimeForTimeEntryEditor(t);
    ASSERT_NE(std::string::npos, res.find(":20"));

    res = Formatter::FormatTimeForTimeEntryEditor(t);
    ASSERT_NE(std::string::npos, res.find(":20"));
}

TEST(Formatter, FormatDateHeader) {
    ASSERT_EQ("", Formatter::FormatDateHeader(0));

    //  date -r 1412120844
    //  Wed Oct  1 01:47:24 CEST 2014
    time_t t(1412120844);
    std::string res = Formatter::FormatDateHeader(t);
    ASSERT_TRUE("Wed, 1 Oct" == res || "Tue, 30 Sep" == res);

    t = time(0);
    ASSERT_EQ("Today", Formatter::FormatDateHeader(t));

    t = t - 86400;
    ASSERT_EQ("Yesterday", Formatter::FormatDateHeader(t));
}

TEST(Formatter, Format8601) {
    ASSERT_EQ("null", Formatter::Format8601(0));

    //  date -r 1412220844
    //  Thu Oct  2 05:34:04 CEST 2014
    time_t t(1412220844);

    ASSERT_EQ("2014-10-02T03:34:04Z", Formatter::Format8601(t));
}

TEST(Formatter, Parse8601) {
    ASSERT_EQ(0, Formatter::Parse8601("null"));

    //  date -r 1412220844
    //  Thu Oct  2 05:34:04 CEST 2014
    time_t t(1412220844);

    ASSERT_EQ(t, Formatter::Parse8601("2014-10-02T03:34:04Z"));

    ASSERT_EQ(0, Formatter::Parse8601("invalid value"));
}

TEST(Formatter, FormatDurationForDateHeader) {
    ASSERT_EQ("0 h 00 min", Formatter::FormatDurationForDateHeader(0));
    ASSERT_EQ("0 h 00 min", Formatter::FormatDurationForDateHeader(30));
    ASSERT_EQ("0 h 02 min", Formatter::FormatDurationForDateHeader(120));
    ASSERT_EQ("1 h 30 min", Formatter::FormatDurationForDateHeader(5400));
}

TEST(Formatter, FormatDurationClassic) {
    ASSERT_EQ("0 sec", Formatter::FormatDuration(0, Format::Classic));
    ASSERT_EQ("30 sec", Formatter::FormatDuration(30, Format::Classic));
    ASSERT_EQ("02:00 min", Formatter::FormatDuration(120, Format::Classic));
    ASSERT_EQ("01:30:00", Formatter::FormatDuration(5400, Format::Classic));
    ASSERT_EQ("01:00:00", Formatter::FormatDuration(3600, Format::Classic));
}

TEST(Formatter, FormatDurationImproved) {
    ASSERT_EQ("0:00:00", Formatter::FormatDuration(0, Format::Improved));
    ASSERT_EQ("0:00:30", Formatter::FormatDuration(30, Format::Improved));
    ASSERT_EQ("0:02:00", Formatter::FormatDuration(120, Format::Improved));
    ASSERT_EQ("1:30:00", Formatter::FormatDuration(5400, Format::Improved));
}

TEST(Formatter, FormatDurationDecimal) {
    ASSERT_EQ("0.00 h", Formatter::FormatDuration(0, Format::Decimal));
    ASSERT_EQ("0.01 h", Formatter::FormatDuration(30, Format::Decimal));
    ASSERT_EQ("0.03 h", Formatter::FormatDuration(120, Format::Decimal));
    ASSERT_EQ("1.50 h", Formatter::FormatDuration(5400, Format::Decimal));
}

TEST(Formatter, FormatDecimal) {
    const int kMinute = 60;
    ASSERT_EQ("0.00 h",
              Formatter::FormatDuration(0*kMinute, Format::Decimal));
    ASSERT_EQ("0.17 h",
              Formatter::FormatDuration(10*kMinute, Format::Decimal));
    ASSERT_EQ("0.25 h",
              Formatter::FormatDuration(15*kMinute, Format::Decimal));
    ASSERT_EQ("0.33 h",
              Formatter::FormatDuration(20*kMinute, Format::Decimal));
    ASSERT_EQ("0.50 h",
              Formatter::FormatDuration(30*kMinute, Format::Decimal));
    ASSERT_EQ("0.58 h",
              Formatter::FormatDuration(35*kMinute, Format::Decimal));
    ASSERT_EQ("0.67 h",
              Formatter::FormatDuration(40*kMinute, Format::Decimal));
    ASSERT_EQ("0.75 h",
              Formatter::FormatDuration(45*kMinute, Format::Decimal));
    ASSERT_EQ("0.83 h",
              Formatter::FormatDuration(50*kMinute, Format::Decimal));
    ASSERT_EQ("0.92 h",
              Formatter::FormatDuration(55*kMinute, Format::Decimal));
    ASSERT_EQ("1.00 h",
              Formatter::FormatDuration(60*kMinute, Format::Decimal));
}

TEST(Formatter, JoinTaskName) {
    locked<Task> nullTask; locked<Project> nullProject;
    std::string res = Formatter::JoinTaskName(nullTask, nullProject);
    ASSERT_EQ("", res);

    ProtectedModel<Task> t { nullptr, true };
    t->SetName("Task name");
    auto lt = *t;
    res = Formatter::JoinTaskName(lt, nullProject);
    ASSERT_EQ("Task name", res);

    ProtectedModel<Project> p { nullptr, true };
    p->SetName("Project name");
    auto lp = *p;
    res = Formatter::JoinTaskName(nullTask, lp);
    ASSERT_EQ(p->Name(), res);

    res = Formatter::JoinTaskName(lt, lp);
    ASSERT_EQ("Task name. Project name", res);

    p->SetCID(1);
    p->SetClientName("Client name");
    res = Formatter::JoinTaskName(lt, lp);
    ASSERT_EQ("Task name. Project name. Client name", res);

    res = Formatter::JoinTaskName(nullTask, lp);
    ASSERT_EQ("Project name. Client name", res);
}

TEST(JSON, EscapeJSONString) {
    std::string text("https://github.com/bartschuller");
    ASSERT_EQ(text, Formatter::EscapeJSONString(text));
    ASSERT_EQ("\"", Formatter::EscapeJSONString("\""));
    ASSERT_EQ(" ", Formatter::EscapeJSONString("\t"));
}

TEST(JSON, UserID) {
    Poco::UInt64 user_id(0);
    ASSERT_EQ(noError,
              User::UserID("{\"data\": {\"id\": 12345}}", &user_id));
    ASSERT_EQ(Poco::UInt64(12345), user_id);
}

TEST(JSON, LoginToken) {
    std::string token("");
    ASSERT_EQ(noError,
              User::LoginToken("{\"login_token\": \"foobar\"}", &token));
    ASSERT_EQ("foobar", token);
}

#if 0
TEST(JSON, ConvertTimelineToJSON) {
    const std::string desktop_id("12345");

    TimelineEvent event;
    event->SetStart(time(0) - 10);
    event->SetEndTime(time(0));
    event->SetFilename("Is this the real life?");
    event->SetTitle("Is this just fantasy?");
    event->SetIdle(true);
    {
        std::vector<TimelineEvent> list;
        list->push_back(event);

        std::string json = convertTimelineToJSON(list, desktop_id);
        Json::Value root = jsonStringToValue(json);
        ASSERT_EQ(std::size_t(1), root->size());

        const Json::Value v = root[0];
        ASSERT_EQ("timeline", v["created_with"].asString());
        ASSERT_EQ(desktop_id, v["desktop_id"].asString());
        ASSERT_EQ(event->Start(), v["start_time"].asUInt());
        ASSERT_EQ(event->EndTime(), v["end_time"].asUInt());
    }

    event->SetIdle(false);
    {
        std::vector<TimelineEvent> list;
        list->push_back(event);

        std::string json = convertTimelineToJSON(list, desktop_id);
        Json::Value root = jsonStringToValue(json);
        ASSERT_EQ(std::size_t(1), root->size());

        const Json::Value v = root[0];
        ASSERT_EQ("timeline", v["created_with"].asString());
        ASSERT_EQ(desktop_id, v["desktop_id"].asString());
        ASSERT_EQ(event->Start(), v["start_time"].asUInt());
        ASSERT_EQ(event->EndTime(), v["end_time"].asUInt());
        ASSERT_EQ(event->Filename(), v["filename"].asString());
        ASSERT_EQ(event->Title(), v["title"].asString());
    }

    event->SetTitle("Õhtu jõuab, päev veereb {\"\b\t");
    {
        std::vector<TimelineEvent> list;
        list->push_back(event);

        std::string json = convertTimelineToJSON(list, desktop_id);
        Json::Value root = jsonStringToValue(json);
        ASSERT_EQ(std::size_t(1), root->size());

        const Json::Value v = root[0];
        ASSERT_EQ(event->Title(), v["title"].asString());
    }
}
#endif

TEST(JSON, Tag) {
    std::string json("{\"id\":36253522,\"wid\":123456788,\"name\":\"create new\",\"at\":\"2013-10-15T08:51:46+00:00\",\"guid\":\"041390ba-ed9c-b477-b949-1a4ebb60a9ce\"}");  // NOLINT

    ProtectedModel<Tag> t { nullptr, true };
    t->LoadFromJSON(jsonStringToValue(json));
    ASSERT_EQ(Poco::UInt64(36253522), t->ID());
    ASSERT_EQ(Poco::UInt64(123456788), t->WID());
    ASSERT_EQ("create new", t->Name());
    //ASSERT_EQ("041390ba-ed9c-b477-b949-1a4ebb60a9ce", t->GUID());
}

TEST(JSON, Workspace) {
    std::string json("{\"id\":123456722,\"name\":\"A deleted workspace\",\"premium\":false,\"admin\":true,\"default_hourly_rate\":0,\"default_currency\":\"USD\",\"only_admins_may_create_projects\":false,\"only_admins_see_billable_rates\":false,\"rounding\":1,\"rounding_minutes\":0,\"at\":\"2013-07-02T13:45:36+00:00\",\"server_deleted_at\":\"2013-08-22T09:05:31+00:00\"}");  // NOLINT

    ProtectedModel<Workspace> w { nullptr, true };
    w->LoadFromJSON(jsonStringToValue(json));
    ASSERT_EQ(Poco::UInt64(123456722), w->ID());
    ASSERT_EQ("A deleted workspace", w->Name());
    ASSERT_FALSE(w->Premium());
    ASSERT_TRUE(w->Admin());
    ASSERT_FALSE(w->OnlyAdminsMayCreateProjects());
}

TEST(JSON, Task) {
    std::string json("{\"id\":1894712,\"name\":\"A deleted task\",\"wid\":123456789,\"pid\":2598305,\"active\":true,\"at\":\"2013-06-05T07:58:41+00:00\",\"estimated_seconds\":0,\"server_deleted_at\":\"2013-08-22T09:05:31+00:00\"}");  // NOLINT

    ProtectedModel<Task> t { nullptr, true };
    t->LoadFromJSON(jsonStringToValue(json));
    ASSERT_EQ(Poco::UInt64(1894712), t->ID());
    ASSERT_EQ("A deleted task", t->Name());
    ASSERT_EQ(Poco::UInt64(123456789), t->WID());
    ASSERT_EQ(Poco::UInt64(2598305), t->PID());
}

TEST(JSON, Project) {
    std::string json("{\"id\":2598323,\"guid\":\"2f0b8f11-f898-d992-3e1a-6bc261fc41ef\",\"wid\":123456789,\"name\":\"A deleted project\",\"billable\":true,\"is_private\":false,\"active\":false,\"template\":false,\"at\":\"2013-05-13T10:19:24+00:00\",\"color\":\"21\",\"auto_estimates\":true,\"server_deleted_at\":\"2013-08-22T09:05:31+00:00\"}");  // NOLINT

    ProtectedModel<Project> p { nullptr, true };
    p->LoadFromJSON(jsonStringToValue(json));
    ASSERT_EQ(Poco::UInt64(2598323), p->ID());
    ASSERT_EQ("A deleted project", p->Name());
    ASSERT_EQ(Poco::UInt64(123456789), p->WID());
    //ASSERT_EQ("2f0b8f11-f898-d992-3e1a-6bc261fc41ef", p.GUID());

    ProtectedModel<Project> p2 { nullptr, true };
    p2->LoadFromJSON(p->SaveToJSON());
    ASSERT_EQ(p->ID(), p2->ID());
    ASSERT_EQ(p->Name(), p2->Name());
    ASSERT_EQ(p->WID(), p2->WID());
    //ASSERT_EQ(p->GUID(), p2->GUID());
    ASSERT_EQ(p->CID(), p2->CID());
    //ASSERT_EQ(p->Billable(), p2->Billable());
    ASSERT_EQ(p->IsPrivate(), p2->IsPrivate());
    ASSERT_EQ(p->UIModifiedAt(), p2->UIModifiedAt());
}

TEST(JSON, Client) {
    std::string json("{\"id\":878318,\"guid\":\"59b464cd-0f8e-e601-ff44-f135225a6738\",\"wid\":123456789,\"name\":\"Big Client\",\"at\":\"2013-03-27T13:17:18+00:00\"}");  // NOLINT

    ProtectedModel<Client> c { nullptr, true };
    c->LoadFromJSON(jsonStringToValue(json));
    ASSERT_EQ(Poco::UInt64(878318), c->ID());
    ASSERT_EQ("Big Client", c->Name());
    ASSERT_EQ(Poco::UInt64(123456789), c->WID());
    //ASSERT_EQ("59b464cd-0f8e-e601-ff44-f135225a6738", c.GUID());

    ProtectedModel<Client> c2 { nullptr, true };
    c2->LoadFromJSON(c->SaveToJSON());
    ASSERT_EQ(c->ID(), c2->ID());
    ASSERT_EQ(c->Name(), c2->Name());
    ASSERT_EQ(c->WID(), c2->WID());
    //ASSERT_EQ(c.GUID(), c2.GUID());
}

TEST(JSON, TimeEntry) {
    std::string json("{\"id\":89818612,\"guid\":\"07fba193-91c4-0ec8-2345-820df0548123\",\"wid\":123456789,\"pid\":2567324,\"billable\":true,\"start\":\"2013-09-05T06:33:50+00:00\",\"stop\":\"2013-09-05T08:19:46+00:00\",\"duration\":6356,\"description\":\"A deleted time entry\",\"tags\":[\"ahaa\"],\"duronly\":false,\"at\":\"2013-09-05T08:19:45+00:00\",\"server_deleted_at\":\"2013-08-22T09:05:31+00:00\"}");  // NOLINT

    ProtectedModel<TimeEntry> t { nullptr, true };
    t->LoadFromJSON(jsonStringToValue(json));
    ASSERT_EQ(Poco::UInt64(0), t->ID()); // ID can only be updated from User class
    ASSERT_EQ(Poco::UInt64(2567324), t->PID());
    ASSERT_EQ(Poco::UInt64(123456789), t->WID());
    //ASSERT_EQ("07fba193-91c4-0ec8-2345-820df0548123", t->GUID());
    ASSERT_TRUE(t->Billable());
    ASSERT_EQ(Poco::UInt64(1378362830000 / 1000), t->Start());
    ASSERT_EQ(Poco::UInt64(1378369186000 / 1000), t->Stop());
    ASSERT_EQ(Poco::Int64(6356), t->DurationInSeconds());
    ASSERT_EQ("A deleted time entry", t->Description());
    ASSERT_EQ("ahaa", t->Tags());
    ASSERT_FALSE(t->DurOnly());

    ProtectedModel<TimeEntry> t2 { nullptr, true };
    t2->LoadFromJSON(t->SaveToJSON());
    ASSERT_EQ(t->ID(), t2->ID());
    ASSERT_EQ(t->PID(), t2->PID());
    ASSERT_EQ(t->WID(), t2->WID());
    //ASSERT_EQ(t->GUID(), t2.GUID());
    ASSERT_EQ(t->Billable(), t2->Billable());
    ASSERT_EQ(t->Start(), t2->Start());
    ASSERT_EQ(t->Stop(), t2->Stop());
    ASSERT_EQ(t->DurationInSeconds(), t2->DurationInSeconds());
    ASSERT_EQ(t->Description(), t2->Description());
    ASSERT_EQ(t->Tags(), t2->Tags());
    ASSERT_EQ(t->DurOnly(), t2->DurOnly());
}

TEST(User, TimeOfDayFormat) {
    ProtectedModel<User> u { nullptr, true };
    u->SetTimeOfDayFormat("H:mm");
    ASSERT_EQ("H:mm", u->TimeOfDayFormat());
    ASSERT_EQ("H:mm", Formatter::TimeOfDayFormat);

    u->SetTimeOfDayFormat("h:mm A");
    ASSERT_EQ("h:mm A", u->TimeOfDayFormat());
    ASSERT_EQ("h:mm A", Formatter::TimeOfDayFormat);
}

TEST(User, DurationFormat) {
    ProtectedModel<User> u { nullptr, true };
    u->SetDurationFormat("classic");
    ASSERT_EQ("classic", u->DurationFormat());
    ASSERT_EQ("classic", Formatter::DurationFormat);

    u->SetDurationFormat("decimal");
    ASSERT_EQ("decimal", u->DurationFormat());
    ASSERT_EQ("decimal", Formatter::DurationFormat);
}

TEST(BaseModel, LoadFromDataStringWithInvalidJSON) {
    ProtectedModel<User> u { nullptr, true };
    error err = u->LoadFromDataString("foobar");
    ASSERT_NE(noError, err);
}

TEST(BaseModel, LoadFromDataString) {
    RelatedData related;
    ProtectedModel<User> u { &related, true };
    error err = u->LoadFromDataString(loadTestData());
    ASSERT_EQ(noError, err);
}

TEST(BaseModel, LoadFromJSONStringWithEmptyString) {
    ProtectedModel<User> u { nullptr, true };
    u->LoadFromJSON(jsonStringToValue(""));
    ASSERT_TRUE(true);
}

TEST(BaseModel, LoadFromJSONStringWithInvalidString) {
    ProtectedModel<User> u { nullptr, true };
    u->LoadFromJSON(jsonStringToValue("foobar"));
    ASSERT_TRUE(true);
}

TEST(BaseModel, BatchUpdateJSONWithoutGUID) {
    ProtectedModel<TimeEntry> t { nullptr, true };
    Json::Value v;
    error err = t->BatchUpdateJSON(&v);
    ASSERT_NE(noError, err);
}

TEST(BaseModel, BatchUpdateJSON) {
    ProtectedModel<TimeEntry> t { nullptr, true };
    t->EnsureGUID();
    Json::Value v;
    error err = t->BatchUpdateJSON(&v);
    ASSERT_EQ(noError, err);
}

TEST(BaseModel, BatchUpdateJSONForDelete) {
    ProtectedModel<TimeEntry> t { nullptr, true };
    t->EnsureGUID();
    t->SetID(123);
    t->SetDeletedAt(time(0));
    Json::Value v;
    error err = t->BatchUpdateJSON(&v);
    ASSERT_EQ(noError, err);
}

TEST(BaseModel, BatchUpdateJSONForPut) {
    ProtectedModel<TimeEntry> t { nullptr, true };
    t->EnsureGUID();
    t->SetID(123);
    t->SetDescription("test");
    Json::Value v;
    error err = t->BatchUpdateJSON(&v);
    ASSERT_EQ(noError, err);
}

TEST(BatchUpdateResult, Error) {
    BatchUpdateResult b;
    ASSERT_EQ(noError, b.Error());
}

TEST(BatchUpdateResult, ErrorWithStatusCode200) {
    BatchUpdateResult b;
    b.StatusCode = 200;
    ASSERT_EQ(noError, b.Error());
}

TEST(BatchUpdateResult, ErrorWithStatusCode400) {
    BatchUpdateResult b;
    b.StatusCode = 400;
    b.Body = "null";
    ASSERT_NE(noError, b.Error());
}

TEST(BatchUpdateResult, String) {
    BatchUpdateResult b;
    ASSERT_NE("", b.String());
}

TEST(BatchUpdateResult, ResourceIsGone) {
    BatchUpdateResult b;
    ASSERT_FALSE(b.ResourceIsGone());
}

TEST(BatchUpdateResult, ResourceIsGoneBecauseOfDeleteMethod) {
    BatchUpdateResult b;
    b.Method = "DELETE";
    ASSERT_TRUE(b.ResourceIsGone());
}

TEST(BatchUpdateResult, ResourceIsGoneBecauseOf404) {
    BatchUpdateResult b;
    b.StatusCode = 404;
    ASSERT_TRUE(b.ResourceIsGone());
}

TEST(Proxy, IsConfigured) {
    Proxy p;
    ASSERT_FALSE(p.IsConfigured());

    p.SetHost("localhost");
    p.SetPort(123);
    ASSERT_TRUE(p.IsConfigured());
}

TEST(Proxy, HasCredentials) {
    Proxy p;
    ASSERT_FALSE(p.HasCredentials());

    p.SetUsername("foo");
    p.SetPassword("bar");
    ASSERT_TRUE(p.HasCredentials());
}

TEST(Proxy, String) {
    Proxy p;
    ASSERT_NE("", p.String());
}

#if 0
TEST(AutotrackerRule, Matches) {
    AutotrackerRule a;
    a.SetTerm("work");
    a.SetPID(123);

    TimelineEvent ev;
    ASSERT_FALSE(a.Matches(ev));

    ev.SetTitle("WORKING");
    ASSERT_TRUE(a.Matches(ev));

    ev.SetTitle("I was working late");
    ASSERT_TRUE(a.Matches(ev));

    ev.SetTitle("dork");
    ASSERT_FALSE(a.Matches(ev));
}
#endif

TEST(Settings, IsSame) {
    ProtectedModel<Settings> s1 { nullptr, true };
    ProtectedModel<Settings> s2 { nullptr, true };
    locked<Settings> ls1 = *s1;
    locked<Settings> ls2 = *s2;

    ASSERT_TRUE(s1->IsSame(ls2));
    ASSERT_TRUE(s2->IsSame(ls1));

    s1->use_idle_detection = true;
    ASSERT_FALSE(s1->IsSame(ls2));
    ASSERT_FALSE(s2->IsSame(ls1));

    // This is a use case I'd like to stop supporting eventually
    // Passing BaseModel instances around as values shouldn't be valid
    // If this test fails, it's possible it's been already changed
    Settings s3 = **ls1;
    locked<Settings> ls3 = s1.make_locked<Settings>(&s3);
    ASSERT_TRUE(s3.IsSame(ls1));
    ASSERT_TRUE(s1->IsSame(ls3));
    ASSERT_FALSE(s3.IsSame(ls2));
    ASSERT_FALSE(s2->IsSame(ls3));
}

}  // namespace toggl

int main(int argc, char **argv) {
    Poco::Logger &logger = Poco::Logger::get("");
    logger.setLevel(Poco::Message::PRIO_DEBUG);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

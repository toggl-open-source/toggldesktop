// Copyright 2014 Toggl Desktop developers.

#include "gtest/gtest.h"

#include <iostream>  // NOLINT

#include "model/autotracker.h"
#include "model/client.h"
#include "const.h"
#include "database/database.h"
#include "util/formatter.h"
#include "model/project.h"
#include "proxy.h"
#include "model/settings.h"
#include "model/tag.h"
#include "model/task.h"
#include "model/time_entry.h"
#include "model/timeline_event.h"
#include "timeline_uploader.h"
#include "model/user.h"
#include "model/workspace.h"
#include "color_convert.h"

#include "test_data.h"

#include "Poco/File.h"
#include "Poco/FileStream.h"
#include "Poco/Logger.h"
#include "Poco/LocalDateTime.h"
#include <Poco/SimpleFileChannel.h>
#include <Poco/FormattingChannel.h>
#include <Poco/PatternFormatter.h>
#include <Poco/ConsoleChannel.h>

namespace toggl {

namespace testing {
class Database {
 public:
    Database() : db_(0) {
        Poco::File f("test.db");
        if (f.exists()) {
            f.remove(false);
        }
        db_ = new toggl::Database("test.db");
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
    TimeEntry te;
    te.SetTags("alfa|beeta", false);
    ASSERT_EQ(std::string("alfa|beeta"), te.Tags());
}

TEST(TimeEntry, WillNotPushUnlessValidationErrorIsCleared) {
    TimeEntry te;
    ASSERT_TRUE(te.NeedsPush());

    // Simulate getting an error from backend
    te.SetValidationError("All you do is wrong");
    ASSERT_EQ("All you do is wrong", te.ValidationError());
    ASSERT_FALSE(te.NeedsPush());

    // Simulate user changing the data,
    // which should wipe the validation error.
    te.SetDurationUserInput("10 seconds");
    ASSERT_EQ("", te.ValidationError());
    ASSERT_TRUE(te.NeedsPush());
}

TEST(TimeEntry, SetDurationUserInput) {
    TimeEntry te;
    ASSERT_FALSE(te.Dirty());
    ASSERT_FALSE(te.UIModifiedAt());

    te.SetDurationUserInput("1 minute");
    ASSERT_TRUE(te.Dirty());
    ASSERT_TRUE(te.UIModifiedAt());
}

TEST(TimeEntry, TagSplitter) {
    std::vector<std::string> expectedSplit {
        "a",
        "b",
        "c"
    };
    std::string expectedJoined {
        "a" "\t"
        "b" "\t"
        "c"
    };
    auto split = toggl::TimeEntry::TagsStringToVector(expectedJoined);
    auto joined = toggl::TimeEntry::TagsVectorToString(expectedSplit);
    auto joinedsplit = toggl::TimeEntry::TagsVectorToString(split);
    auto splitjoined = toggl::TimeEntry::TagsStringToVector(joined);

    ASSERT_EQ(expectedSplit, split);
    ASSERT_EQ(expectedSplit, splitjoined);
    ASSERT_EQ(expectedJoined, joined);
    ASSERT_EQ(expectedJoined, joinedsplit);
}

TEST(Project, ProjectsHaveColorCodes) {
    Project p;
    p.SetColor("1");
    ASSERT_EQ("#9e5bd9", p.ColorCode());
    p.SetColor("");
    ASSERT_EQ("", p.ColorCode());
    p.SetColor("0");
    ASSERT_EQ("#0b83d9", p.ColorCode());
}

TEST(Project, ResolveOnlyAdminsCanChangeProjectVisibility) {
    Project p;
    p.SetPrivate(false);
    error err = error("Only admins can change project visibility");
    ASSERT_TRUE(p.ResolveError(err));
    ASSERT_TRUE(p.Private());
}

TEST(User, CreateCompressedTimelineBatchForUpload) {
    testing::Database db;

    User user;
    ASSERT_EQ(noError,
              user.LoadUserAndRelatedDataFromJSONString(loadTestData(), true, false));
    Poco::UInt64 user_id = user.ID();

    std::vector<ModelChange> changes;

    Poco::UInt64 good_duration_seconds(30);

    // Event that happened at least 15 minutes ago,
    // can be uploaded to Toggl backend.
    TimelineEvent *good = new TimelineEvent();
    good->SetUID(user_id);
    // started yesterday, "16 minutes ago"
    good->SetStartTime(time(0) - 86400 - 60*16);
    good->SetEndTime(good->Start() + good_duration_seconds);
    good->SetFilename("Notepad.exe");
    good->SetTitle("untitled");
    user.related.TimelineEvents.push_back(good);

    Poco::UInt64 good2_duration_seconds(20);

    // Another event that happened at least 15 minutes ago,
    // can be uploaded to Toggl backend.
    TimelineEvent *good2 = new TimelineEvent();
    good2->SetUID(user_id);
    good2->SetStartTime(good->EndTime() + 1);  // started after first event
    good2->SetEndTime(good2->Start() + good2_duration_seconds);
    good2->SetFilename("Notepad.exe");
    good2->SetTitle("untitled");
    user.related.TimelineEvents.push_back(good2);

    // Another event that happened at least 15 minutes ago,
    // but has already been uploaded to Toggl backend.
    TimelineEvent *uploaded = new TimelineEvent();;
    uploaded->SetUID(user_id);
    uploaded->SetStartTime(good2->EndTime() + 1);  // started after second event
    uploaded->SetEndTime(uploaded->Start() + 10);
    uploaded->SetFilename("Notepad.exe");
    uploaded->SetTitle("untitled");
    uploaded->SetUploaded(true);
    user.related.TimelineEvents.push_back(uploaded);

    // This event happened less than 15 minutes ago,
    // so it must not be uploaded
    TimelineEvent *too_fresh = new TimelineEvent();
    too_fresh->SetUID(user_id);
    too_fresh->SetStartTime(time(0) - 60);  // started 1 minute ago
    too_fresh->SetEndTime(time(0));  // lasted until now
    too_fresh->SetFilename("Notepad.exe");
    too_fresh->SetTitle("notes");
    user.related.TimelineEvents.push_back(too_fresh);

    // This event happened more than 7 days ago,
    // so it must not be uploaded, just deleted
    TimelineEvent *too_old = new TimelineEvent();
    too_old->SetUID(user_id);
    too_old->SetStartTime(time(0) - kTimelineSecondsToKeep - 1);  // 7 days ago
    too_old->SetEndTime(too_old->EndTime() + 120);  // lasted 2 minutes
    too_old->SetFilename("Notepad.exe");
    too_old->SetTitle("diary");
    user.related.TimelineEvents.push_back(too_old);

    db.instance()->SaveUser(&user, true, &changes);

    user.CompressTimeline();
    std::vector<const TimelineEvent*> timeline_events = user.CompressedTimelineForUpload();

    if (timeline_events.size() != 1) {
        std::cerr << "user.related.TimelineEvents:" << std::endl;
        for (std::vector<TimelineEvent *>::const_iterator it =
            user.related.TimelineEvents.begin();
                it != user.related.TimelineEvents.end(); it++) {
            TimelineEvent *ev = *it;
            std::cerr << ev->String() << std::endl;
        }

        std::cerr << "user.CompressedTimelineForUpload:" << std::endl;
        for (std::vector<const TimelineEvent*>::const_iterator it =
            timeline_events.begin();
                it != timeline_events.end(); it++) {
            const TimelineEvent *ev = *it;
            std::cerr << ev->String() << std::endl;
        }
    }

    ASSERT_EQ(size_t(1), timeline_events.size());

    // Compress some more, for fun and profit
    for (int i = 0; i < 100; i++) {
        user.CompressTimeline();
        timeline_events = user.CompressedTimelineForUpload();
    }

    ASSERT_EQ(size_t(1), timeline_events.size());

    const TimelineEvent *ready_for_upload = timeline_events[0];
    ASSERT_TRUE(ready_for_upload->Chunked());
    ASSERT_EQ(good->UID(), ready_for_upload->UID());

    ASSERT_NE(good2->Start(), ready_for_upload->Start());
    ASSERT_NE(uploaded->Start(), ready_for_upload->Start());
    ASSERT_NE(too_old->Start(), ready_for_upload->Start());
    ASSERT_NE(too_fresh->Start(), ready_for_upload->Start());
    ASSERT_EQ(good->Start(), ready_for_upload->Start());

    ASSERT_EQ(static_cast<Poco::Int64>(
        good_duration_seconds + good2_duration_seconds),
              ready_for_upload->Duration());
    ASSERT_EQ(good->Filename(), ready_for_upload->Filename());
    ASSERT_EQ(good->Title(), ready_for_upload->Title());
    ASSERT_EQ(good->Idle(), ready_for_upload->Idle());
    ASSERT_FALSE(ready_for_upload->Uploaded());

    // Fake that we have uploaded the chunked timeline event now
    user.MarkTimelineBatchAsUploaded(timeline_events);

    // Now, no more events should exist for upload
    std::vector<const TimelineEvent*> left_for_upload = user.CompressedTimelineForUpload();
    ASSERT_EQ(std::size_t(0), left_for_upload.size());
}

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

Json::Value &jsonStringToValue(const std::string json_string) {
    static thread_local Json::Value root;
    Json::Reader reader;
    reader.parse(json_string, root);
    return root;
}

TEST(User, Since) {
    User u;

    // no timestamp should be wrong
    ASSERT_FALSE(u.HasValidSinceDate());

    // 0 timestamp should be wrong
    u.SetSince(0);
    ASSERT_FALSE(u.HasValidSinceDate());

    // current time should be ok
    u.SetSince(time(0));
    ASSERT_TRUE(u.HasValidSinceDate());

    // 1 month ago should be fine
    u.SetSince(time(0) - 2.62974e6);
    ASSERT_TRUE(u.HasValidSinceDate());
}

TEST(User, UpdatesTimeEntryFromJSON) {
    User user;
    ASSERT_EQ(noError,
              user.LoadUserAndRelatedDataFromJSONString(loadTestData(), true, false));

    TimeEntry *te = user.related.TimeEntryByID(89818605);
    ASSERT_TRUE(te);

    std::string json = "{\"id\":89818605,\"description\":\"Changed\"}";
    te->LoadFromJSON(jsonStringToValue(json), false);
    ASSERT_EQ("Changed", te->Description());
}

TEST(User, DeletesZombies) {
    User user;
    ASSERT_EQ(noError,
              user.LoadUserAndRelatedDataFromJSONString(loadTestData(), true, false));

    TimeEntry *te = user.related.TimeEntryByID(89818605);
    ASSERT_TRUE(te);
    ASSERT_FALSE(te->IsMarkedAsDeletedOnServer());

    std::string json =
        loadTestDataFile(std::string("../testdata/me_without_time_entries.json"));

    ASSERT_EQ(noError,
              user.LoadUserAndRelatedDataFromJSONString(json, true, false));

    te = user.related.TimeEntryByID(89818605);
    ASSERT_TRUE(te);
    ASSERT_TRUE(te->IsMarkedAsDeletedOnServer());
}

TEST(Database, LoadUserByEmail) {
    testing::Database db;

    User user;
    ASSERT_EQ(noError,
              user.LoadUserAndRelatedDataFromJSONString(loadTestData(), true, false));

    std::vector<ModelChange> changes;
    ASSERT_EQ(noError, db.instance()->SaveUser(&user, true, &changes));

    User user2;
    ASSERT_EQ(noError,
              db.instance()->LoadUserByEmail("johnsmith@toggl.com", &user2));

    ASSERT_EQ(user.ID(), user2.ID());
}

TEST(Database, LoadUserByEmailWithoutEmail) {
    testing::Database db;

    User user;
    ASSERT_NE(noError, db.instance()->LoadUserByEmail("", &user));
}

TEST(Database, LoadUserByEmailWithNonexistantEmail) {
    testing::Database db;

    User user;
    ASSERT_EQ(noError, db.instance()->LoadUserByEmail("foo@bar.com", &user));
}

TEST(Database, AllowsSameEmail) {
    testing::Database db;

    User user;
    ASSERT_EQ(noError,
              user.LoadUserAndRelatedDataFromJSONString(loadTestData(), true, false));

    std::vector<ModelChange> changes;
    ASSERT_EQ(noError, db.instance()->SaveUser(&user, true, &changes));

    User user2;
    std::string json = loadTestDataFile(std::string("../testdata/same_email.json"));
    ASSERT_EQ(noError,
              user2.LoadUserAndRelatedDataFromJSONString(json, true, false));

    ASSERT_EQ(noError, db.instance()->SaveUser(&user2, true, &changes));

    ASSERT_EQ(user.Email(), user2.Email());
    ASSERT_NE(user.ID(), user2.ID());
    ASSERT_NE(user.APIToken(), user2.APIToken());
}

TEST(Formatter, EscapeControlCharactersInJSONString) {
    std::string text("\x16");
    ASSERT_EQ(" ", Formatter::EscapeJSONString(text));
}

TEST(User, UpdatesTimeEntryFromFullUserJSON) {
    testing::Database db;

    std::string json = loadTestData();

    User user;
    ASSERT_EQ(noError,
              user.LoadUserAndRelatedDataFromJSONString(loadTestData(), true, false));

    TimeEntry *te = user.related.TimeEntryByID(89818605);
    ASSERT_TRUE(te);

    size_t n = json.find("Important things");
    ASSERT_TRUE(n);

    te->SetDescription("Even more important!", false);

    ASSERT_EQ(noError,
              user.LoadUserAndRelatedDataFromJSONString(json, true, false));
    te = user.related.TimeEntryByID(89818605);
    ASSERT_TRUE(te);
    ASSERT_EQ("Even more important!", te->Description());
}

TEST(Database, SavesAndLoadsUserFields) {
    testing::Database db;

    User user;
    ASSERT_EQ(noError,
              user.LoadUserAndRelatedDataFromJSONString(loadTestData(), true, false));

    std::vector<ModelChange> changes;
    ASSERT_EQ(noError, db.instance()->SaveUser(&user, true, &changes));

    // Load user into another instance
    User user2;
    ASSERT_EQ(noError, db.instance()->LoadUserByID(user.ID(), &user2));
}

TEST(Database, SavesModelsAndKnowsToUpdateWithSameUserInstance) {
    testing::Database db;

    User user;
    ASSERT_EQ(noError,
              user.LoadUserAndRelatedDataFromJSONString(loadTestData(), true, false));

    Poco::UInt64 n;
    ASSERT_EQ(noError, db.instance()->UInt("select count(1) from users", &n));
    ASSERT_EQ(Poco::UInt64(0), n);

    for (int i = 0; i < 3; i++) {
        std::vector<ModelChange> changes;
        ASSERT_EQ(noError, db.instance()->SaveUser(&user, true, &changes));

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

    User user1;
    ASSERT_EQ(noError,
              user1.LoadUserAndRelatedDataFromJSONString(json, true, false));

    std::vector<ModelChange> changes;
    ASSERT_EQ(noError, db.instance()->SaveUser(&user1, true, &changes));

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

    User user2;
    ASSERT_EQ(noError, db.instance()->LoadUserByID(user1.ID(), &user2));

    ASSERT_EQ(user1.related.Workspaces.size(),
              user2.related.Workspaces.size());
    ASSERT_EQ(user1.related.Clients.size(),
              user2.related.Clients.size());
    ASSERT_EQ(user1.related.Projects.size(),
              user2.related.Projects.size());
    ASSERT_EQ(user1.related.Tasks.size(),
              user2.related.Tasks.size());
    ASSERT_EQ(user1.related.Tags.size(),
              user2.related.Tags.size());
    ASSERT_EQ(user1.related.TimeEntries.size(),
              user2.related.TimeEntries.size());

    ASSERT_EQ(noError,
              user2.LoadUserAndRelatedDataFromJSONString(json, true, false));

    ASSERT_EQ(noError, db.instance()->SaveUser(&user2, true, &changes));

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

    User user;
    ASSERT_EQ(noError,
              user.LoadUserAndRelatedDataFromJSONString(loadTestData(), true, false));

    size_t count = user.related.TimeEntries.size();

    user.Start("Old work", "1 hour", 0, 0, "", "", 0, 0, true);

    ASSERT_EQ(count + 1, user.related.TimeEntries.size());

    TimeEntry *te = user.related.TimeEntries[user.related.TimeEntries.size()-1];

    ASSERT_EQ(3600, te->DurationInSeconds());
}

TEST(User, TestStartTimeEntryWithoutDuration) {
    testing::Database db;

    User user;
    ASSERT_EQ(noError,
              user.LoadUserAndRelatedDataFromJSONString(loadTestData(), true, false));

    user.Start("Old work", "", 0, 0, "", "", 0, 0, true);

    TimeEntry *te = user.RunningTimeEntry();
    ASSERT_TRUE(te);
    ASSERT_GT(0, te->DurationInSeconds());
}

TEST(User, TestDeletionSteps) {
    testing::Database db;

    User user;
    ASSERT_EQ(noError,
              user.LoadUserAndRelatedDataFromJSONString(loadTestData(), true, false));

    // first, mark time entry as deleted
    user.Start("My new time entry", "", 0, 0, "", "", 0, 0, true);
    TimeEntry *te = user.RunningTimeEntry();
    ASSERT_TRUE(te);
    std::vector<ModelChange> changes;
    ASSERT_EQ(noError, db.instance()->SaveUser(&user, true, &changes));
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
    ASSERT_EQ(noError, db.instance()->SaveUser(&user, true, &changes));
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
    User user;
    ASSERT_EQ(noError,
              user.LoadUserAndRelatedDataFromJSONString(loadTestData(), true, false));

    testing::Database db;

    std::vector<ModelChange> changes;
    ASSERT_EQ(noError, db.instance()->SaveUser(&user, false, &changes));

    ASSERT_EQ(noError, db.instance()->SaveUser(&user, false, &changes));
}

TEST(Database, AssignsGUID) {
    std::string json = loadTestData();
    ASSERT_FALSE(json.empty());

    User user;
    ASSERT_EQ(noError,
              user.LoadUserAndRelatedDataFromJSONString(json, true, false));

    ASSERT_EQ(uint(5), user.related.TimeEntries.size());
    TimeEntry *te = user.related.TimeEntryByID(89837445);
    ASSERT_TRUE(te);

    ASSERT_NE("", te->GUID());
    ASSERT_TRUE(te->GUID().size());

    TimeEntry *te2 = user.related.TimeEntryByGUID(te->GUID());
    ASSERT_TRUE(te2);

    ASSERT_EQ(te->GUID(), te2->GUID());
    ASSERT_EQ(te->ID(), te2->ID());
}

TEST(User, ParsesAndSavesData) {
    std::string json = loadTestData();
    ASSERT_FALSE(json.empty());

    User user;
    ASSERT_EQ(noError,
              user.LoadUserAndRelatedDataFromJSONString(json, true, false));
    ASSERT_EQ(Poco::UInt64(1379068550), user.Since());
    ASSERT_EQ(Poco::UInt64(10471231), user.ID());
    ASSERT_EQ(Poco::UInt64(123456788), user.DefaultWID());
    ASSERT_EQ("30eb0ae954b536d2f6628f7fec47beb6", user.APIToken());
    ASSERT_EQ("John Smith", user.Fullname());

    // Projects
    ASSERT_EQ(uint(2), user.related.Projects.size());

    ASSERT_EQ(Poco::UInt64(2567324), user.related.Projects[0]->ID());
    //ASSERT_EQ("2f0b8f51-f898-d992-3e1a-6bc261fc41xf", user.related.Projects[0]->GUID());
    ASSERT_EQ(Poco::UInt64(123456789), user.related.Projects[0]->WID());
    ASSERT_EQ("Even more work", user.related.Projects[0]->Name());
    ASSERT_EQ("#999999", user.related.Projects[0]->Color());
    ASSERT_EQ(Poco::UInt64(1385144), user.related.Projects[0]->CID());
    ASSERT_EQ(user.ID(), user.related.Projects[0]->UID());
    ASSERT_TRUE(user.related.Projects[0]->Active());

    ASSERT_EQ(Poco::UInt64(2598305), user.related.Projects[1]->ID());
    ASSERT_EQ(Poco::UInt64(123456789), user.related.Projects[1]->WID());
    ASSERT_EQ("Testing stuff", user.related.Projects[1]->Name());
    ASSERT_EQ("#fb8b14", user.related.Projects[1]->Color());
    ASSERT_EQ(user.ID(), user.related.Projects[1]->UID());
    ASSERT_FALSE(user.related.Projects[1]->Active());

    // Time entries
    ASSERT_EQ(uint(5), user.related.TimeEntries.size());

    ASSERT_EQ(Poco::UInt64(89818605), user.related.TimeEntries[0]->ID());
    //ASSERT_EQ("07fba193-91c4-0ec8-2894-820df0548a8f", user.related.TimeEntries[0]->GUID());
    ASSERT_EQ(uint(2567324), user.related.TimeEntries[0]->PID());
    ASSERT_TRUE(user.related.TimeEntries[0]->Billable());
    ASSERT_EQ(uint(1378362830), user.related.TimeEntries[0]->StartTime());
    ASSERT_EQ(uint(1378369186), user.related.TimeEntries[0]->StopTime());
    ASSERT_EQ(6356, user.related.TimeEntries[0]->DurationInSeconds());
    ASSERT_EQ("Important things",
              user.related.TimeEntries[0]->Description());
    ASSERT_EQ(uint(0), user.related.TimeEntries[0]->TagNames->size());
    ASSERT_FALSE(user.related.TimeEntries[0]->DurOnly());
    ASSERT_EQ(user.ID(), user.related.TimeEntries[0]->UID());

    ASSERT_EQ(uint(2), user.related.Tasks.size());

    ASSERT_EQ(uint(1894794), user.related.Tasks[0]->ID());
    ASSERT_EQ("blog (writing)", user.related.Tasks[0]->Name());
    ASSERT_EQ(uint(123456789), user.related.Tasks[0]->WID());
    ASSERT_EQ(uint(2598305), user.related.Tasks[0]->PID());
    ASSERT_EQ(user.ID(), user.related.Tasks[0]->UID());

    // Tags
    ASSERT_EQ(uint(2), user.related.Tags.size());

    ASSERT_EQ(uint(27457022), user.related.Tags[0]->ID());
    ASSERT_EQ("billed", user.related.Tags[0]->Name());
    ASSERT_EQ(user.ID(), user.related.Tags[0]->UID());
    ASSERT_EQ(uint(123456788), user.related.Tags[0]->WID());
    //ASSERT_EQ("", user.related.Tags[0]->GUID());

    ASSERT_EQ(uint(36253522), user.related.Tags[1]->ID());
    ASSERT_EQ("create new", user.related.Tags[1]->Name());
    ASSERT_EQ(user.ID(), user.related.Tags[1]->UID());
    ASSERT_EQ(uint(123456788), user.related.Tags[1]->WID());
    //ASSERT_EQ("041390ba-ed9c-b477-b949-1a4ebb60a9ce", user.related.Tags[1]->GUID());

    // Workspaces
    ASSERT_EQ(uint(2), user.related.Workspaces.size());

    ASSERT_EQ(uint(123456788), user.related.Workspaces[0]->ID());
    ASSERT_EQ("stuff", user.related.Workspaces[0]->Name());
    ASSERT_EQ(user.ID(), user.related.Workspaces[0]->UID());

    ASSERT_FALSE(user.related.Workspaces[0]->OnlyAdminsMayCreateProjects());
    ASSERT_TRUE(user.related.Workspaces[0]->Admin());

    ASSERT_TRUE(user.related.Workspaces[1]->OnlyAdminsMayCreateProjects());
    ASSERT_TRUE(user.related.Workspaces[1]->Admin());
    ASSERT_EQ(uint(123456789), user.related.Workspaces[1]->ID());

    // Clients (2 in JSON but 1 is deleted)
    ASSERT_EQ(uint(1), user.related.Clients.size());

    ASSERT_EQ(uint(878318), user.related.Clients[0]->ID());
    ASSERT_EQ(uint(123456788), user.related.Clients[0]->WID());
    ASSERT_EQ("Big Client", user.related.Clients[0]->Name());
    //ASSERT_EQ("59b464cd-0f8e-e601-ff44-f135225a6738", user.related.Clients[0]->GUID());
    ASSERT_EQ(user.ID(), user.related.Clients[0]->UID());

    testing::Database db;

    Poco::UInt64 n;
    ASSERT_EQ(noError, db.instance()->UInt("select count(1) from users", &n));
    ASSERT_EQ(Poco::UInt64(0), n);

    // Insert
    std::vector<ModelChange> changes;
    ASSERT_EQ(noError, db.instance()->SaveUser(&user, true, &changes));
    ASSERT_GT(user.LocalID(), uint(0));
    ASSERT_GT(user.ID(), uint(0));
    ASSERT_FALSE(user.APIToken().empty());

    ASSERT_EQ(noError,
              db.instance()->UInt("select count(1) from users", &n));
    ASSERT_EQ(Poco::UInt64(1), n);

    ASSERT_EQ(noError,
              db.instance()->UInt("select count(1) from workspaces", &n));
    ASSERT_EQ(Poco::UInt64(user.related.Workspaces.size()), n);

    ASSERT_EQ(noError,
              db.instance()->UInt("select count(1) from clients", &n));
    ASSERT_EQ(Poco::UInt64(user.related.Clients.size()), n);

    ASSERT_EQ(noError,
              db.instance()->UInt("select count(1) from projects", &n));
    ASSERT_EQ(Poco::UInt64(user.related.Projects.size()), n);

    ASSERT_EQ(noError, db.instance()->UInt("select count(1) from tasks", &n));
    ASSERT_EQ(Poco::UInt64(user.related.Tasks.size()), n);

    ASSERT_EQ(noError, db.instance()->UInt("select count(1) from tags", &n));
    ASSERT_EQ(Poco::UInt64(user.related.Tags.size()), n);

    ASSERT_EQ(noError,
              db.instance()->UInt("select count(1) from time_entries", &n));
    ASSERT_EQ(Poco::UInt64(user.related.TimeEntries.size()), n);

    // Update
    ASSERT_EQ(noError, db.instance()->SaveUser(&user, true, &changes));
    ASSERT_EQ(noError, db.instance()->UInt("select count(1) from users", &n));
    ASSERT_EQ(Poco::UInt64(1), n);

    // Select
    User user2;
    ASSERT_EQ(noError, db.instance()->LoadUserByID(user.ID(), &user2));

    ASSERT_TRUE(user2.ID());
    ASSERT_EQ(user.ID(), user2.ID());
    ASSERT_TRUE(user2.Since());
    ASSERT_EQ(user.Since(), user2.Since());
    ASSERT_EQ(user.ID(), user2.ID());
    ASSERT_EQ(user.DefaultWID(), user2.DefaultWID());

    ASSERT_EQ(uint(2), user2.related.Projects.size());
    Project *project_from_db =
        user2.related.ProjectByID(user.related.Projects[0]->ID());
    ASSERT_TRUE(project_from_db);
    ASSERT_EQ(user.related.Projects[0]->String(),
              project_from_db->String());
    project_from_db = user2.related.ProjectByID(user.related.Projects[1]->ID());
    ASSERT_EQ(user.related.Projects[1]->String(),
              project_from_db->String());

    ASSERT_EQ(uint(5), user2.related.TimeEntries.size());
    TimeEntry *te_from_db =
        user2.related.TimeEntryByID(user.related.TimeEntries[0]->ID());
    ASSERT_TRUE(te_from_db);
    ASSERT_EQ(user.related.TimeEntries[0]->String(), te_from_db->String());
    te_from_db = user2.related.TimeEntryByID(user.related.TimeEntries[1]->ID());
    ASSERT_TRUE(te_from_db);
    ASSERT_EQ(user.related.TimeEntries[1]->String(), te_from_db->String());
    te_from_db = user2.related.TimeEntryByID(user.related.TimeEntries[2]->ID());
    ASSERT_TRUE(te_from_db);
    ASSERT_EQ(user.related.TimeEntries[2]->String(), te_from_db->String());

    ASSERT_EQ(uint(2), user2.related.Workspaces.size());
    Workspace *ws_from_db =
        user2.related.WorkspaceByID(user.related.Workspaces[0]->ID());
    ASSERT_TRUE(ws_from_db);
    ASSERT_EQ(user.related.Workspaces[0]->String(), ws_from_db->String());
    ws_from_db = user2.related.WorkspaceByID(user.related.Workspaces[1]->ID());
    ASSERT_TRUE(ws_from_db);
    ASSERT_EQ(user.related.Workspaces[1]->String(), ws_from_db->String());

    ASSERT_EQ(uint(2), user2.related.Tasks.size());
    Task *task_from_db = user2.related.TaskByID(user2.related.Tasks[0]->ID());
    ASSERT_EQ(user.related.Tasks[0]->String(), task_from_db->String());
    ASSERT_EQ(user.related.Tasks[1]->String(),
              user2.related.Tasks[1]->String());

    ASSERT_EQ(uint(1), user2.related.Clients.size());
    ASSERT_EQ(user.related.Clients[0]->String(),
              user2.related.Clients[0]->String());

    // Delete
    ASSERT_EQ(noError, db.instance()->DeleteUser(&user, true));

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
    TimeEntry te;

    te.SetDurationUserInput("00:00:15");
    ASSERT_EQ("0:00:15",
              toggl::Formatter::FormatDuration(te.DurationInSeconds(),
                      toggl::Format::Improved));

    te.SetDurationInSeconds(0, false);
    te.SetDurationUserInput("00:23:15");
    ASSERT_EQ("0:23:15",
              toggl::Formatter::FormatDuration(te.DurationInSeconds(),
                      toggl::Format::Improved));

    te.SetDurationInSeconds(0, false);
    te.SetDurationUserInput("12:34:56");
    ASSERT_EQ("12:34:56",
              toggl::Formatter::FormatDuration(te.DurationInSeconds(),
                      toggl::Format::Improved));

    te.SetDurationInSeconds(0, false);
    te.SetDurationUserInput("0:1");
    ASSERT_EQ("0:01:00",
              toggl::Formatter::FormatDuration(te.DurationInSeconds(),
                      toggl::Format::Improved));

    te.SetDurationInSeconds(0, false);
    te.SetDurationUserInput("1:2");
    ASSERT_EQ("1:02:00",
              toggl::Formatter::FormatDuration(te.DurationInSeconds(),
                      toggl::Format::Improved));

    te.SetDurationInSeconds(0, false);
    te.SetDurationUserInput("1:0");
    ASSERT_EQ("1:00:00",
              toggl::Formatter::FormatDuration(te.DurationInSeconds(),
                      toggl::Format::Improved));

    te.SetDurationInSeconds(0, false);
    te.SetDurationUserInput("05:22 min");
    ASSERT_EQ("0:05:22",
              toggl::Formatter::FormatDuration(te.DurationInSeconds(),
                      toggl::Format::Improved));

    te.SetDurationInSeconds(0, false);
    te.SetDurationUserInput("00:22 min");
    ASSERT_EQ("0:00:22",
              toggl::Formatter::FormatDuration(te.DurationInSeconds(),
                      toggl::Format::Improved));

    te.SetDurationInSeconds(0, false);
    te.SetDurationUserInput("0 hours");
    ASSERT_EQ("0:00:00",
              toggl::Formatter::FormatDuration(te.DurationInSeconds(),
                      toggl::Format::Improved));

    te.SetDurationInSeconds(0, false);
    te.SetDurationUserInput("0.5 hours");
    ASSERT_EQ("0:30:00",
              toggl::Formatter::FormatDuration(te.DurationInSeconds(),
                      toggl::Format::Improved));

    te.SetDurationInSeconds(0, false);
    te.SetDurationUserInput("0,5 hours");
    ASSERT_EQ("0:30:00",
              toggl::Formatter::FormatDuration(te.DurationInSeconds(),
                      toggl::Format::Improved));

    te.SetDurationInSeconds(0, false);
    te.SetDurationUserInput("1 hour");
    ASSERT_EQ("1:00:00",
              toggl::Formatter::FormatDuration(te.DurationInSeconds(),
                      toggl::Format::Improved));

    te.SetDurationInSeconds(0, false);
    te.SetDurationUserInput("1 hr");
    ASSERT_EQ("1:00:00",
              toggl::Formatter::FormatDuration(te.DurationInSeconds(),
                      toggl::Format::Improved));

    te.SetDurationInSeconds(0, false);
    te.SetDurationUserInput("1,5 hours");
    ASSERT_EQ("1:30:00",
              toggl::Formatter::FormatDuration(te.DurationInSeconds(),
                      toggl::Format::Improved));

    te.SetDurationInSeconds(0, false);
    te.SetDurationUserInput("1.5 hours");
    ASSERT_EQ("1:30:00",
              toggl::Formatter::FormatDuration(te.DurationInSeconds(),
                      toggl::Format::Improved));

    te.SetDurationInSeconds(0, false);
    te.SetDurationUserInput("24 hours");
    ASSERT_EQ(86400, te.DurationInSeconds());

    te.SetDurationInSeconds(0, false);
    te.SetDurationUserInput("0 minutes");
    ASSERT_EQ("0:00:00",
              toggl::Formatter::FormatDuration(te.DurationInSeconds(),
                      toggl::Format::Improved));

    te.SetDurationInSeconds(0, false);
    te.SetDurationUserInput("0 min");
    ASSERT_EQ("0:00:00",
              toggl::Formatter::FormatDuration(te.DurationInSeconds(),
                      toggl::Format::Improved));

    te.SetDurationInSeconds(0, false);
    te.SetDurationUserInput("5 minutes");
    ASSERT_EQ("0:05:00",
              toggl::Formatter::FormatDuration(te.DurationInSeconds(),
                      toggl::Format::Improved));

    te.SetDurationInSeconds(0, false);
    te.SetDurationUserInput("5minutes");
    ASSERT_EQ("0:05:00",
              toggl::Formatter::FormatDuration(te.DurationInSeconds(),
                      toggl::Format::Improved));

    te.SetDurationInSeconds(0, false);
    te.SetDurationUserInput("0,5 minutes");
    ASSERT_EQ("0:00:30",
              toggl::Formatter::FormatDuration(te.DurationInSeconds(),
                      toggl::Format::Improved));

    te.SetDurationInSeconds(0, false);
    te.SetDurationUserInput("1 minute");
    ASSERT_EQ("0:01:00",
              toggl::Formatter::FormatDuration(te.DurationInSeconds(),
                      toggl::Format::Improved));

    te.SetDurationInSeconds(0, false);
    te.SetDurationUserInput("1,5 minutes");
    ASSERT_EQ("0:01:30",
              toggl::Formatter::FormatDuration(te.DurationInSeconds(),
                      toggl::Format::Improved));

    te.SetDurationInSeconds(0, false);
    te.SetDurationUserInput("1.5 minutes");
    ASSERT_EQ("0:01:30",
              toggl::Formatter::FormatDuration(te.DurationInSeconds(),
                      toggl::Format::Improved));

    te.SetDurationInSeconds(0, false);
    te.SetDurationUserInput("15");
    ASSERT_EQ("0:15:00",
              toggl::Formatter::FormatDuration(te.DurationInSeconds(),
                      toggl::Format::Improved));

    te.SetDurationInSeconds(0, false);
    te.SetDurationUserInput("0 seconds");
    ASSERT_EQ("0:00:00",
              toggl::Formatter::FormatDuration(te.DurationInSeconds(),
                      toggl::Format::Improved));

    te.SetDurationInSeconds(0, false);
    te.SetDurationUserInput("1 second");
    ASSERT_EQ("0:00:01",
              toggl::Formatter::FormatDuration(te.DurationInSeconds(),
                      toggl::Format::Improved));

    te.SetDurationInSeconds(0, false);
    te.SetDurationUserInput("1.5h");
    ASSERT_EQ("1:30:00",
              toggl::Formatter::FormatDuration(te.DurationInSeconds(),
                      toggl::Format::Improved));

    te.SetDurationInSeconds(0, false);
    te.SetDurationUserInput("1.5 h");
    ASSERT_EQ("1:30:00",
              toggl::Formatter::FormatDuration(te.DurationInSeconds(),
                      toggl::Format::Improved));

    te.SetDurationInSeconds(0, false);
    te.SetDurationUserInput("3h");
    ASSERT_EQ("3:00:00",
              toggl::Formatter::FormatDuration(te.DurationInSeconds(),
                      toggl::Format::Improved));

    te.SetDurationInSeconds(0, false);
    te.SetDurationUserInput("3 h");
    ASSERT_EQ("3:00:00",
              toggl::Formatter::FormatDuration(te.DurationInSeconds(),
                      toggl::Format::Improved));

    te.SetDurationInSeconds(0, false);
    te.SetDurationUserInput("15m");
    ASSERT_EQ("0:15:00",
              toggl::Formatter::FormatDuration(te.DurationInSeconds(),
                      toggl::Format::Improved));

    te.SetDurationInSeconds(0, false);
    te.SetDurationUserInput("15 m");
    ASSERT_EQ("0:15:00",
              toggl::Formatter::FormatDuration(te.DurationInSeconds(),
                      toggl::Format::Improved));

    te.SetDurationInSeconds(0, false);
    te.SetDurationUserInput("25s");
    ASSERT_EQ("0:00:25",
              toggl::Formatter::FormatDuration(te.DurationInSeconds(),
                      toggl::Format::Improved));

    te.SetDurationInSeconds(0, false);
    te.SetDurationUserInput("25 s");
    ASSERT_EQ("0:00:25",
              toggl::Formatter::FormatDuration(te.DurationInSeconds(),
                      toggl::Format::Improved));

    te.SetDurationInSeconds(0, false);
    te.SetDurationUserInput("1.5");
    ASSERT_EQ("1:30:00",
              toggl::Formatter::FormatDuration(te.DurationInSeconds(),
                      toggl::Format::Improved));

    te.SetDurationInSeconds(0, false);
    te.SetDurationUserInput("1,5");
    ASSERT_EQ("1:30:00",
              toggl::Formatter::FormatDuration(te.DurationInSeconds(),
                      toggl::Format::Improved));

    te.SetDurationInSeconds(0, false);
    te.SetDurationUserInput("0.25");
    ASSERT_EQ("0:15:00",
              toggl::Formatter::FormatDuration(te.DurationInSeconds(),
                      toggl::Format::Improved));

    te.SetDurationInSeconds(0, false);
    te.SetDurationUserInput("0.025");
    ASSERT_EQ("0:01:30",
              toggl::Formatter::FormatDuration(te.DurationInSeconds(),
                      toggl::Format::Improved));

    te.SetDurationInSeconds(0, false);
    te.SetDurationUserInput("2h45");
    ASSERT_EQ("2:45:00",
              toggl::Formatter::FormatDuration(te.DurationInSeconds(),
                      toggl::Format::Improved));

    te.SetDurationInSeconds(0, false);
    te.SetDurationUserInput("2h");
    ASSERT_EQ("2:00:00",
              toggl::Formatter::FormatDuration(te.DurationInSeconds(),
                      toggl::Format::Improved));

    te.SetDurationInSeconds(0, false);
    te.SetDurationUserInput("2h 18m");
    ASSERT_EQ("2:18:00",
              toggl::Formatter::FormatDuration(te.DurationInSeconds(),
                      toggl::Format::Improved));

    te.SetDurationInSeconds(0, false);
    te.SetDurationUserInput("2h 18m 50s");
    ASSERT_EQ("2:18:50",
              toggl::Formatter::FormatDuration(te.DurationInSeconds(),
                      toggl::Format::Improved));

    te.SetDurationInSeconds(0, false);
    te.SetDurationUserInput("1hr 25min 30sec");
    ASSERT_EQ("1:25:30",
              toggl::Formatter::FormatDuration(te.DurationInSeconds(),
                      toggl::Format::Improved));

    te.SetDurationInSeconds(0, false);
    te.SetDurationUserInput("1 hours 25 minutes 30 seconds");
    ASSERT_EQ("1:25:30",
              toggl::Formatter::FormatDuration(te.DurationInSeconds(),
                      toggl::Format::Improved));

    te.SetDurationInSeconds(0, false);
    te.SetDurationUserInput("1 hour 1 minute 1 second");
    ASSERT_EQ("1:01:01",
              toggl::Formatter::FormatDuration(te.DurationInSeconds(),
                      toggl::Format::Improved));
}

TEST(TimeEntry, ParseDurationLargerThan24Hours) {
    TimeEntry te;

    te.SetDurationInSeconds(0, false);
    te.SetDurationUserInput("90:10:00");
    ASSERT_EQ("90:10:00",
              toggl::Formatter::FormatDuration(te.DurationInSeconds(),
                      toggl::Format::Improved));
}

TEST(TimeEntry, InterpretsCrazyStartAsMissingValues) {
    TimeEntry te;

    ASSERT_EQ(Poco::UInt64(0), te.StartTime());
    te.SetStartString("0003-03-16T-7:-19:-24Z", false);
    ASSERT_EQ(Poco::UInt64(0), te.StartTime());
}

TEST(User, Continue) {
    testing::Database db;

    User user;
    ASSERT_EQ(noError,
              user.LoadUserAndRelatedDataFromJSONString(loadTestData(), true, false));

    TimeEntry *te = user.related.TimeEntryByID(89818605);
    ASSERT_TRUE(te);
    size_t count = user.related.TimeEntries.size();
    ASSERT_TRUE(user.Continue(te->GUID(), false));
    ASSERT_EQ(count + 1, user.related.TimeEntries.size());
}

TEST(TimeEntry, SetDurationOnRunningTimeEntryWithDurOnlySetting) {
    testing::Database db;

    std::string json = loadTestDataFile(std::string("../testdata/user_with_duronly.json"));

    User user;
    ASSERT_EQ(noError,
              user.LoadUserAndRelatedDataFromJSONString(json, true, false));

    TimeEntry *te = user.related.TimeEntryByID(164891639);
    ASSERT_TRUE(te);
    ASSERT_TRUE(te->IsTracking());
    ASSERT_LT(te->StartTime(), te->StopTime());

    te->SetDurationUserInput("00:59:47");
    ASSERT_TRUE(te->IsTracking());
    ASSERT_LT(te->StartTime(), te->StopTime());

    te->StopTracking();
    ASSERT_FALSE(te->IsTracking());
    ASSERT_LT(te->StartTime(), te->StopTime());
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
    std::string res = Formatter::JoinTaskName(0, 0);
    ASSERT_EQ("", res);

    Task t;
    t.SetName("Task name");
    res = Formatter::JoinTaskName(&t, 0);
    ASSERT_EQ("Task name", res);

    Project p;
    p.SetName("Project name");
    res = Formatter::JoinTaskName(0, &p);
    ASSERT_EQ(p.Name(), res);

    res = Formatter::JoinTaskName(&t, &p);
    ASSERT_EQ("Task name. Project name", res);

    p.SetCID(1);
    p.SetClientName("Client name");
    res = Formatter::JoinTaskName(&t, &p);
    ASSERT_EQ("Task name. Project name. Client name", res);

    res = Formatter::JoinTaskName(0, &p);
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

TEST(JSON, ConvertTimelineToJSON) {
    const std::string desktop_id("12345");

    TimelineEvent event;
    event.SetStartTime(time(0) - 10);
    event.SetEndTime(time(0));
    event.SetFilename("Is this the real life?");
    event.SetTitle("Is this just fantasy?");
    event.SetIdle(true);
    {
        std::vector<const TimelineEvent*> list;
        list.push_back(&event);

        std::string json = convertTimelineToJSON(list, desktop_id);
        Json::Value root = jsonStringToValue(json);
        ASSERT_EQ(std::size_t(1), root.size());

        const Json::Value v = root[0];
        ASSERT_EQ("timeline", v["created_with"].asString());
        ASSERT_EQ(desktop_id, v["desktop_id"].asString());
        ASSERT_EQ(event.Start(), v["start_time"].asUInt());
        ASSERT_EQ(event.EndTime(), v["end_time"].asUInt());
    }

    event.SetIdle(false);
    {
        std::vector<const TimelineEvent*> list;
        list.push_back(&event);

        std::string json = convertTimelineToJSON(list, desktop_id);
        Json::Value root = jsonStringToValue(json);
        ASSERT_EQ(std::size_t(1), root.size());

        const Json::Value v = root[0];
        ASSERT_EQ("timeline", v["created_with"].asString());
        ASSERT_EQ(desktop_id, v["desktop_id"].asString());
        ASSERT_EQ(event.Start(), v["start_time"].asUInt());
        ASSERT_EQ(event.EndTime(), v["end_time"].asUInt());
        ASSERT_EQ(event.Filename(), v["filename"].asString());
        ASSERT_EQ(event.Title(), v["title"].asString());
    }

    event.SetTitle("Õhtu jõuab, päev veereb {\"\b\t");
    {
        std::vector<const TimelineEvent*> list;
        list.push_back(&event);

        std::string json = convertTimelineToJSON(list, desktop_id);
        Json::Value root = jsonStringToValue(json);
        ASSERT_EQ(std::size_t(1), root.size());

        const Json::Value v = root[0];
        ASSERT_EQ(event.Title(), v["title"].asString());
    }
}

TEST(JSON, Tag) {
    std::string json("{\"id\":36253522,\"wid\":123456788,\"name\":\"create new\",\"at\":\"2013-10-15T08:51:46+00:00\",\"guid\":\"041390ba-ed9c-b477-b949-1a4ebb60a9ce\"}");  // NOLINT

    Tag t;
    t.LoadFromJSON(jsonStringToValue(json));
    ASSERT_EQ(Poco::UInt64(36253522), t.ID());
    ASSERT_EQ(Poco::UInt64(123456788), t.WID());
    ASSERT_EQ("create new", t.Name());
    //ASSERT_EQ("041390ba-ed9c-b477-b949-1a4ebb60a9ce", t.GUID());
}

TEST(JSON, Workspace) {
    std::string json("{\"id\":123456722,\"name\":\"A deleted workspace\",\"premium\":false,\"admin\":true,\"default_hourly_rate\":0,\"default_currency\":\"USD\",\"only_admins_may_create_projects\":false,\"only_admins_see_billable_rates\":false,\"rounding\":1,\"rounding_minutes\":0,\"at\":\"2013-07-02T13:45:36+00:00\",\"server_deleted_at\":\"2013-08-22T09:05:31+00:00\"}");  // NOLINT

    Workspace w;
    w.LoadFromJSON(jsonStringToValue(json));
    ASSERT_EQ(Poco::UInt64(123456722), w.ID());
    ASSERT_EQ("A deleted workspace", w.Name());
    ASSERT_FALSE(w.Premium());
    ASSERT_TRUE(w.Admin());
    ASSERT_FALSE(w.OnlyAdminsMayCreateProjects());
}

TEST(JSON, Task) {
    std::string json("{\"id\":1894712,\"name\":\"A deleted task\",\"wid\":123456789,\"pid\":2598305,\"active\":true,\"at\":\"2013-06-05T07:58:41+00:00\",\"estimated_seconds\":0,\"server_deleted_at\":\"2013-08-22T09:05:31+00:00\"}");  // NOLINT

    Task t;
    t.LoadFromJSON(jsonStringToValue(json));
    ASSERT_EQ(Poco::UInt64(1894712), t.ID());
    ASSERT_EQ("A deleted task", t.Name());
    ASSERT_EQ(Poco::UInt64(123456789), t.WID());
    ASSERT_EQ(Poco::UInt64(2598305), t.PID());
}

TEST(JSON, Project) {
    std::string json("{\"id\":2598323,\"guid\":\"2f0b8f11-f898-d992-3e1a-6bc261fc41ef\",\"wid\":123456789,\"name\":\"A deleted project\",\"billable\":true,\"is_private\":false,\"active\":false,\"template\":false,\"at\":\"2013-05-13T10:19:24+00:00\",\"color\":\"21\",\"auto_estimates\":true,\"server_deleted_at\":\"2013-08-22T09:05:31+00:00\"}");  // NOLINT

    Project p;
    p.LoadFromJSON(jsonStringToValue(json), false);
    ASSERT_EQ(Poco::UInt64(2598323), p.ID());
    ASSERT_EQ("A deleted project", p.Name());
    ASSERT_EQ(Poco::UInt64(123456789), p.WID());
    //ASSERT_EQ("2f0b8f11-f898-d992-3e1a-6bc261fc41ef", p.GUID());

    Project p2;
    p2.LoadFromJSON(p.SaveToJSON(), false);
    ASSERT_EQ(p.ID(), p2.ID());
    ASSERT_EQ(p.Name(), p2.Name());
    ASSERT_EQ(p.WID(), p2.WID());
    //ASSERT_EQ(p.GUID(), p2.GUID());
    ASSERT_EQ(p.CID(), p2.CID());
    //ASSERT_EQ(p.Billable(), p2.Billable());
    ASSERT_EQ(p.Private(), p2.Private());
    ASSERT_EQ(p.UIModifiedAt(), p2.UIModifiedAt());
}

TEST(JSON, Client) {
    std::string json("{\"id\":878318,\"guid\":\"59b464cd-0f8e-e601-ff44-f135225a6738\",\"wid\":123456789,\"name\":\"Big Client\",\"at\":\"2013-03-27T13:17:18+00:00\"}");  // NOLINT

    Client c;
    c.LoadFromJSON(jsonStringToValue(json), false);
    ASSERT_EQ(Poco::UInt64(878318), c.ID());
    ASSERT_EQ("Big Client", c.Name());
    ASSERT_EQ(Poco::UInt64(123456789), c.WID());
    //ASSERT_EQ("59b464cd-0f8e-e601-ff44-f135225a6738", c.GUID());

    Client c2;
    c2.LoadFromJSON(c.SaveToJSON(), false);
    ASSERT_EQ(c.ID(), c2.ID());
    ASSERT_EQ(c.Name(), c2.Name());
    ASSERT_EQ(c.WID(), c2.WID());
    //ASSERT_EQ(c.GUID(), c2.GUID());
}

TEST(JSON, TimeEntry) {
    std::string json("{\"id\":89818612,\"guid\":\"07fba193-91c4-0ec8-2345-820df0548123\",\"wid\":123456789,\"pid\":2567324,\"billable\":true,\"start\":\"2013-09-05T06:33:50+00:00\",\"stop\":\"2013-09-05T08:19:46+00:00\",\"duration\":6356,\"description\":\"A deleted time entry\",\"tags\":[\"ahaa\"],\"duronly\":false,\"at\":\"2013-09-05T08:19:45+00:00\",\"server_deleted_at\":\"2013-08-22T09:05:31+00:00\"}");  // NOLINT

    TimeEntry t;
    t.LoadFromJSON(jsonStringToValue(json), false);
    ASSERT_EQ(Poco::UInt64(0), t.ID()); // ID can only be updated from User class
    ASSERT_EQ(Poco::UInt64(2567324), t.PID());
    ASSERT_EQ(Poco::UInt64(123456789), t.WID());
    //ASSERT_EQ("07fba193-91c4-0ec8-2345-820df0548123", t.GUID());
    ASSERT_TRUE(t.Billable());
    ASSERT_EQ(Poco::UInt64(1378362830000 / 1000), t.StartTime());
    ASSERT_EQ(Poco::UInt64(1378369186000 / 1000), t.StopTime());
    ASSERT_EQ(Poco::Int64(6356), t.DurationInSeconds());
    ASSERT_EQ("A deleted time entry", t.Description());
    ASSERT_EQ("ahaa", t.Tags());
    ASSERT_FALSE(t.DurOnly());

    TimeEntry t2;
    t2.LoadFromJSON(t.SaveToJSON(), false);
    ASSERT_EQ(t.ID(), t2.ID());
    ASSERT_EQ(t.PID(), t2.PID());
    ASSERT_EQ(t.WID(), t2.WID());
    //ASSERT_EQ(t.GUID(), t2.GUID());
    ASSERT_EQ(t.Billable(), t2.Billable());
    ASSERT_EQ(t.StartTime(), t2.StartTime());
    ASSERT_EQ(t.StopTime(), t2.StopTime());
    ASSERT_EQ(t.DurationInSeconds(), t2.DurationInSeconds());
    ASSERT_EQ(t.Description(), t2.Description());
    ASSERT_EQ(t.Tags(), t2.Tags());
    ASSERT_EQ(t.DurOnly(), t2.DurOnly());
}

TEST(User, TimeOfDayFormat) {
    User u;
    u.SetTimeOfDayFormat("H:mm");
    ASSERT_EQ("H:mm", u.TimeOfDayFormat());
    ASSERT_EQ("H:mm", Formatter::TimeOfDayFormat);

    u.SetTimeOfDayFormat("h:mm A");
    ASSERT_EQ("h:mm A", u.TimeOfDayFormat());
    ASSERT_EQ("h:mm A", Formatter::TimeOfDayFormat);
}

TEST(User, DurationFormat) {
    User u;
    u.SetDurationFormat("classic");
    ASSERT_EQ("classic", u.DurationFormat());
    ASSERT_EQ("classic", Formatter::DurationFormat);

    u.SetDurationFormat("decimal");
    ASSERT_EQ("decimal", u.DurationFormat());
    ASSERT_EQ("decimal", Formatter::DurationFormat);
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

TEST(AutotrackerRule, Matches) {
    AutotrackerRule a;
    a.SetTerms("work");
    a.SetPID(123);

    TimelineEvent ev;
    ASSERT_FALSE(a.Matches(ev));

    ev.SetTitle("WORKING");
    ASSERT_TRUE(a.Matches(ev));

    ev.SetTitle("I was working late");
    ASSERT_TRUE(a.Matches(ev));

    ev.SetTitle("dork");
    ASSERT_FALSE(a.Matches(ev));

    // multiple terms in a single autotracker rule
    a.SetTerms("edge\tchrome\tfirefox");

    ev.SetTitle("toggl-open-source/toggldesktop: Toggl Desktop app for Windows, Mac and Linux - Google Chrome");
    ASSERT_TRUE(a.Matches(ev));

    ev.SetTitle("YouTube - Chromium");
    ASSERT_FALSE(a.Matches(ev));

    ev.SetTitle("(1) Home / Twitter - Mozilla Firefox");
    ASSERT_TRUE(a.Matches(ev));

    a.SetStartTime("9:00");
    a.SetEndTime("17:00");

    Poco::LocalDateTime eventTime(2020, 4, 3, 21); // Friday, 21:00
    ev.SetEndTime(eventTime.utc().timestamp().epochTime());
    ASSERT_FALSE(a.Matches(ev));

    eventTime = Poco::LocalDateTime(2020, 4, 3, 14); // Friday, 14:00
    ev.SetEndTime(eventTime.utc().timestamp().epochTime());
    ASSERT_TRUE(a.Matches(ev));

    a.SetDaysOfWeek(0); // default value, means no days restrictions
    ASSERT_TRUE(a.Matches(ev));

    a.SetDaysOfWeek(toggl::AutotrackerRule::DaysOfWeekIntoUInt8(false, true, true, true, true, true, false)); // weekdays
    ASSERT_TRUE(a.Matches(ev));

    a.SetDaysOfWeek(toggl::AutotrackerRule::DaysOfWeekIntoUInt8(false, true, true, true, true, false, false)); // Monday - Thursday
    ASSERT_FALSE(a.Matches(ev));
}

TEST(Settings, IsSame) {
    Settings s1;
    Settings s2;

    ASSERT_TRUE(s1.IsSame(s2));
    ASSERT_TRUE(s2.IsSame(s1));

    s1.use_idle_detection = true;
    ASSERT_FALSE(s1.IsSame(s2));
    ASSERT_FALSE(s2.IsSame(s1));

    Settings s3 = s1;
    ASSERT_TRUE(s3.IsSame(s1));
    ASSERT_TRUE(s1.IsSame(s3));
    ASSERT_FALSE(s3.IsSame(s2));
    ASSERT_FALSE(s2.IsSame(s3));
}

TEST(ColorConverter_HSV_0B83D9, IsCorrect) {
    TogglHsvColor color_1 = toggl::ColorConverter::GetAdaptiveColor("0B83D9", AdaptiveColorShapeOnLightBackground);
    ASSERT_NEAR(0.57, color_1.h, 0.01);
    ASSERT_NEAR(0.95, color_1.s, 0.01);
    ASSERT_NEAR(0.85, color_1.v, 0.01);

    TogglHsvColor color_2 = toggl::ColorConverter::GetAdaptiveColor("0B83D9", AdaptiveColorTextOnLightBackground);
    ASSERT_NEAR(0.57, color_2.h, 0.01);
    ASSERT_NEAR(0.95, color_2.s, 0.01);
    ASSERT_NEAR(0.7, color_2.v, 0.01);

    TogglHsvColor color_3 = toggl::ColorConverter::GetAdaptiveColor("0B83D9", AdaptiveColorShapeOnDarkBackground);
    ASSERT_NEAR(0.57, color_3.h, 0.01);
    ASSERT_NEAR(0.81, color_3.s, 0.01);
    ASSERT_NEAR(0.95, color_3.v, 0.01);

    TogglHsvColor color_4 = toggl::ColorConverter::GetAdaptiveColor("0B83D9", AdaptiveColorTextOnDarkBackground);
    ASSERT_NEAR(0.57, color_4.h, 0.01);
    ASSERT_NEAR(0.81, color_4.s, 0.01);
    ASSERT_NEAR(1.0, color_4.v, 0.01);
}

TEST(ColorConverter_RGB_0B83D9, IsCorrect) {
    TogglRgbColor color_1 = toggl::ColorConverter::GetRgbAdaptiveColor("0B83D9", AdaptiveColorShapeOnLightBackground);
    ASSERT_NEAR(0.04, color_1.r, 0.01);
    ASSERT_NEAR(0.51, color_1.g, 0.01);
    ASSERT_NEAR(0.85, color_1.b, 0.01);

    TogglRgbColor color_2 = toggl::ColorConverter::GetRgbAdaptiveColor("0B83D9", AdaptiveColorTextOnLightBackground);
    ASSERT_NEAR(0.03, color_2.r, 0.01);
    ASSERT_NEAR(0.42, color_2.g, 0.01);
    ASSERT_NEAR(0.70, color_2.b, 0.01);

    TogglRgbColor color_3 = toggl::ColorConverter::GetRgbAdaptiveColor("0B83D9", AdaptiveColorShapeOnDarkBackground);
    ASSERT_NEAR(0.18, color_3.r, 0.01);
    ASSERT_NEAR(0.63, color_3.g, 0.01);
    ASSERT_NEAR(0.95, color_3.b, 0.01);

    TogglRgbColor color_4 = toggl::ColorConverter::GetRgbAdaptiveColor("0B83D9", AdaptiveColorTextOnDarkBackground);
    ASSERT_NEAR(0.19, color_4.r, 0.01);
    ASSERT_NEAR(0.66, color_4.g, 0.01);
    ASSERT_NEAR(1.0, color_4.b, 0.01);
}

TEST(ColorConverter_RGB_991102, IsCorrect) {
    TogglRgbColor color_1 = toggl::ColorConverter::GetRgbAdaptiveColor("991102", AdaptiveColorShapeOnLightBackground);
    ASSERT_NEAR(0.60, color_1.r, 0.01);
    ASSERT_NEAR(0.07, color_1.g, 0.01);
    ASSERT_NEAR(0.01, color_1.b, 0.01);

    TogglRgbColor color_2 = toggl::ColorConverter::GetRgbAdaptiveColor("991102", AdaptiveColorTextOnLightBackground);
    ASSERT_NEAR(0.45, color_2.r, 0.01);
    ASSERT_NEAR(0.05, color_2.g, 0.01);
    ASSERT_NEAR(0.0, color_2.b, 0.01);

    TogglRgbColor color_3 = toggl::ColorConverter::GetRgbAdaptiveColor("991102", AdaptiveColorShapeOnDarkBackground);
    ASSERT_NEAR(0.86, color_3.r, 0.01);
    ASSERT_NEAR(0.40, color_3.g, 0.01);
    ASSERT_NEAR(0.35, color_3.b, 0.01);

    TogglRgbColor color_4 = toggl::ColorConverter::GetRgbAdaptiveColor("991102", AdaptiveColorTextOnDarkBackground);
    ASSERT_NEAR(0.91, color_4.r, 0.01);
    ASSERT_NEAR(0.43, color_4.g, 0.01);
    ASSERT_NEAR(0.37, color_4.b, 0.01);
}

TEST(ColorConverter_RGB_2DA608, IsCorrect) {
    TogglRgbColor color_1 = toggl::ColorConverter::GetRgbAdaptiveColor("2DA608", AdaptiveColorShapeOnLightBackground);
    ASSERT_NEAR(0.17, color_1.r, 0.01);
    ASSERT_NEAR(0.65, color_1.g, 0.01);
    ASSERT_NEAR(0.04, color_1.b, 0.01);

    TogglRgbColor color_2 = toggl::ColorConverter::GetRgbAdaptiveColor("2DA608", AdaptiveColorTextOnLightBackground);
    ASSERT_NEAR(0.14, color_2.r, 0.01);
    ASSERT_NEAR(0.50, color_2.g, 0.01);
    ASSERT_NEAR(0.03, color_2.b, 0.01);

    TogglRgbColor color_3 = toggl::ColorConverter::GetRgbAdaptiveColor("2DA608", AdaptiveColorShapeOnDarkBackground);
    ASSERT_NEAR(0.46, color_3.r, 0.01);
    ASSERT_NEAR(0.88, color_3.g, 0.01);
    ASSERT_NEAR(0.33, color_3.b, 0.01);

    TogglRgbColor color_4 = toggl::ColorConverter::GetRgbAdaptiveColor("2DA608", AdaptiveColorTextOnDarkBackground);
    ASSERT_NEAR(0.49, color_4.r, 0.01);
    ASSERT_NEAR(0.93, color_4.g, 0.01);
    ASSERT_NEAR(0.35, color_4.b, 0.01);
}

TEST(Sync, LegacyFormat) {
    testing::Database db;
    std::string json { "{\"data\" : {\"achievements_enabled\" : true,\"api_token\" : \"token\",\"at\" : \"2019-04-08T11:08:37+00:00\",\"beginning_of_week\" : 1,\"clients\" : [{\"at\" : \"2018-11-07T20:52:31+00:00\",\"id\" : 43289164,\"name\" : \"client\",\"wid\" : 2817276}],\"created_at\" : \"2018-06-24T17:56:16+00:00\",\"date_format\" : \"MM/DD/YYYY\",\"default_wid\" : 2817276,\"duration_format\" : \"improved\",\"email\" : \"m@rtinbriza.cz\",\"fullname\" : \"M\",\"id\" : 4187712,\"image_url\" : \"https://assets.toggl.space/images/profile.png\",\"invitation\" : {},\"jquery_date_format\" : \"m/d/Y\",\"jquery_timeofday_format\" : \"h:i A\",\"language\" : \"en_US\",\"last_blog_entry\" : \"\",\"new_blog_post\" : {},\"openid_email\" : \"m@rtinbriza.cz\",\"openid_enabled\" : true,\"projects\" : [{\"active\" : true,\"actual_hours\" : 362,\"at\" : \"2019-09-18T12:31:25+00:00\",\"auto_estimates\" : false,\"billable\" : false,\"cid\" : 43289164,\"color\" : \"0\",\"created_at\" : \"2019-09-18T12:31:25+00:00\",\"hex_color\" : \"#06aaf5\",\"id\" : 154073509,\"is_private\" : true,\"name\" : \"project\",\"template\" : false,\"wid\" : 2817276}],\"record_timeline\" : true,\"render_timeline\" : true,\"retention\" : 9,\"send_product_emails\" : true,\"send_timer_notifications\" : true,\"send_weekly_report\" : true,\"should_upgrade\" : true,\"sidebar_piechart\" : true,\"store_start_and_stop_time\" : true,\"tags\" : [{\"at\" : \"2019-10-18T10:08:01+00:00\",\"id\" : 6892625,\"name\" : \"tag\",\"wid\" : 2817276}],\"time_entries\": [{\"at\" : \"2020-05-27T15:40:37+00:00\",\"billable\" : false,\"description\" : \"time entry\",\"duration\" : 415,\"duronly\" : false,\"guid\" : \"a28b9092ec9055edea7af710fcd72459\",\"id\" : 1563187599,\"pid\" : 154073509,\"start\" : \"2020-05-27T15:33:42+00:00\",\"stop\" : \"2020-05-27T15:40:37+00:00\",\"uid\" : 4187712,\"wid\" : 2817276}],\"timeline_enabled\" : true,\"timeline_experiment\" : false,\"timeofday_format\" : \"h:mm A\",\"timezone\" : \"Europe/Warsaw\",\"workspaces\" : [{\"admin\" : true,\"api_token\" : \"token\",\"at\" : \"2018-09-01T08:27:56+00:00\",\"default_currency\" : \"USD\",\"default_hourly_rate\" : 0,\"ical_enabled\" : true,\"id\" : 2817276,\"name\" : \"workspace\",\"only_admins_may_create_projects\" : false,\"only_admins_see_billable_rates\" : false,\"only_admins_see_team_dashboard\" : false,\"premium\" : true,\"profile\" : 0,\"projects_billable_by_default\" : true,\"rounding\" : 1,\"rounding_minutes\" : 0}]},\"since\" : 1590592101}" };
    User user;
    error err = user.LoadUserAndRelatedDataFromJSONString(json, false, false);
    ASSERT_EQ(err, noError);

    ASSERT_EQ(user.Email(), "m@rtinbriza.cz");
    ASSERT_EQ(user.Fullname(), "M");
    ASSERT_EQ(user.ID(), 4187712);

    ASSERT_EQ(user.related.Clients.size(), 1);
    ASSERT_EQ(user.related.Projects.size(), 1);
    ASSERT_EQ(user.related.Tags.size(), 1);
    ASSERT_EQ(user.related.TimeEntries.size(), 1);
    ASSERT_EQ(user.related.Workspaces.size(), 1);

    ASSERT_EQ(user.related.Clients[0]->ID(), 43289164);
    ASSERT_EQ(user.related.Clients[0]->Name(), "client");
    ASSERT_EQ(user.related.Clients[0]->WID(), 2817276);

    ASSERT_EQ(user.related.Projects[0]->ID(), 154073509);
    ASSERT_EQ(user.related.Projects[0]->Name(), "project");
    ASSERT_EQ(user.related.Projects[0]->WID(), 2817276);
    ASSERT_EQ(user.related.Projects[0]->CID(), 43289164);
    ASSERT_EQ(user.related.Projects[0]->Active(), true);
    ASSERT_EQ(user.related.Projects[0]->Billable(), false);

    ASSERT_EQ(user.related.Tags[0]->ID(), 6892625);
    ASSERT_EQ(user.related.Tags[0]->Name(), "tag");
    ASSERT_EQ(user.related.Tags[0]->WID(), 2817276);

    ASSERT_EQ(user.related.TimeEntries[0]->ID(), 1563187599);
    ASSERT_EQ(user.related.TimeEntries[0]->Description(), "time entry");
    ASSERT_EQ(user.related.TimeEntries[0]->WID(), 2817276);
    ASSERT_EQ(user.related.TimeEntries[0]->UID(), 4187712);

    ASSERT_EQ(user.related.TimeEntries[0]->StartTime(), 1590593622);
    ASSERT_EQ(user.related.TimeEntries[0]->StopTime(), 1590594037);

    ASSERT_EQ(user.related.TimeEntries[0]->Duration(), 415);

    ASSERT_EQ(user.related.Workspaces[0]->ID(), 2817276);
    ASSERT_EQ(user.related.Workspaces[0]->Name(), "workspace");
    ASSERT_EQ(user.related.Workspaces[0]->Admin(), true);
    ASSERT_EQ(user.related.Workspaces[0]->Premium(), true);
}

TEST(Sync, BatchedFormat) {
    testing::Database db;
    std::string json { "{\"clients\" : [{\"at\" : \"2018-11-07T20:52:31+00:00\",\"id\" : 43289164,\"name\" : \"client\",\"wid\" : 2817276}],\"flags\" : {\"badges.master_seen\" : \"2019-10-21T08:53:51.051Z\",\"has_seen_toggl_master_campaign\" : true,\"notifications.snowball_weekly_report_rollout\" : true,\"shopify_discount_enabled\" : false,\"snowball_detailed_report_rollout\" : true,\"snowball_summary_report_rollout\" : true},\"preferences\" : {\"CollapseTimeEntries\" : true,\"alpha_features\" : [{\"code\" : \"snowball_projects_list\",\"enabled\" : true},{\"code\" : \"snowball_teams\",\"enabled\" : true},{\"code\" : \"snowball_project_teams\",\"enabled\" : true},{\"code\" : \"snowball_saved_reports\",\"enabled\" : true},{\"code\" : \"snowball_workspace_settings_general\",\"enabled\" : true},{\"code\" : \"snowball_workspace_settings_owner\",\"enabled\" : true},{\"code\" : \"snowball_workspace_settings_alerts\",\"enabled\" : true},{\"code\" : \"snowball_workspace_settings_reminders\",\"enabled\" : true},{\"code\" : \"snowball_workspace_creation\",\"enabled\" : true},{\"code\" : \"snowball_view_shared_report\",\"enabled\" : true},{\"code\" : \"snowball_project_edit\",\"enabled\" : true},{\"code\" : \"snowball_settings\",\"enabled\" : true},{\"code\" : \"snowball_weekly_report\",\"enabled\" : true},{\"code\" : \"snowball_workspace_settings_integrations\",\"enabled\" : false},{\"code\" : \"snowball_detailed_report\",\"enabled\" : false},{\"code\" : \"snowball_clients\",\"enabled\" : true},{\"code\" : \"snowball_workspace_settings_import\",\"enabled\" : false},{\"code\" : \"snowball_profile\",\"enabled\" : true},{\"code\" : \"mobile_sync_client\",\"enabled\" : false},{\"code\" : \"snowball_dashboard\",\"enabled\" : false},{\"code\" : \"new_react_router\",\"enabled\" : false},{\"code\" : \"web_sync_client\",\"enabled\" : false},{\"code\" : \"alpha_program\",\"enabled\" : false},{\"code\" : \"dekstop_sync_client\",\"enabled\" : false},{\"code\" : \"snowball_tags\",\"enabled\" : true},{\"code\" : \"single_sign_on\",\"enabled\" : false},{\"code\" : \"calendar_view\",\"enabled\" : false},{\"code\" : \"snowball_project_tasks\",\"enabled\" : true},{\"code\" : \"snowball_i18n\",\"enabled\" : false}],\"date_format\" : \"MM/DD/YYYY\",\"duration_format\" : \"improved\",\"record_timeline\" : true,\"send_product_emails\" : true,\"send_timer_notifications\" : true,\"send_weekly_report\" : true,\"timeofday_format\" : \"h:mm A\"},\"projects\" : [{\"active\" : true,\"actual_hours\" : 362,\"at\" : \"2019-09-18T12:31:25+00:00\",\"auto_estimates\" : null,\"billable\" : null,\"cid\" : 43289164,\"client_id\" : 43289164,\"color\" : \"#06aaf5\",\"created_at\" : \"2019-09-18T12:31:25+00:00\",\"currency\" : null,\"estimated_hours\" : null,\"id\" : 154073509,\"is_private\" : true,\"name\" : \"project\",\"rate\" : null,\"server_deleted_at\" : null,\"template\" : null,\"wid\" : 2817276,\"workspace_id\" : 2817276}],\"server_time\" : 1590592101,\"tags\" : [{\"at\" : \"2019-10-18T10:08:01.693372Z\",\"id\" : 6892625,\"name\" : \"tag\",\"workspace_id\" : 2817276}],\"tasks\" : [],\"time_entries\" : [{\"at\" : \"2020-05-27T15:40:37+00:00\",\"billable\" : false,\"description\" : \"time entry\",\"duration\" : 415,\"duronly\" : false,\"id\" : 1563187599,\"project_id\" : null,\"server_deleted_at\" : null,\"start\" : \"2020-05-27T15:33:42+00:00\",\"stop\" : \"2020-05-27T15:40:37.000000Z\",\"tag_ids\" : null,\"tags\" : null,\"task_id\" : null,\"uid\" : 4187712,\"user_id\" : 4187712,\"wid\" : 2817276,\"workspace_id\" : 2817276}],\"user\" : {\"api_token\" : \"token\",\"at\" : \"2020-05-27T15:08:21.468625Z\",\"beginning_of_week\" : 1,\"country_id\" : 59,\"created_at\" : \"2018-06-24T17:56:16.075815Z\",\"default_workspace_id\" : 2817276,\"email\" : \"m@rtinbriza.cz\",\"fullname\" : \"M\",\"has_password\" : true,\"id\" : 4187712,\"image_url\" : \"https://assets.toggl.com/images/profile.png\",\"intercom_hash\" : \"33ad107c55cc9f8e89b0b1940812194b9441f742771f0f3be14bf329f41f8f9b\",\"oauth_providers\" : [ \"google\" ],\"openid_email\" : \"m@rtinbriza.cz\",\"openid_enabled\" : true,\"timezone\" : \"Europe/Warsaw\",\"updated_at\" : \"2019-04-08T11:08:37.90838Z\"},\"workspace_features\" : [{\"features\" : [{\"enabled\" : true,\"feature_id\" : 0,\"name\" : \"free\"},{\"enabled\" : false,\"feature_id\" : 13,\"name\" : \"pro\"},{\"enabled\" : false,\"feature_id\" : 15,\"name\" : \"business\"},{\"enabled\" : false,\"feature_id\" : 50,\"name\" : \"scheduled_reports\"},{\"enabled\" : false,\"feature_id\" : 51,\"name\" : \"time_audits\"},{\"enabled\" : false,\"feature_id\" : 52,\"name\" : \"locking_time_entries\"},{\"enabled\" : false,\"feature_id\" : 53,\"name\" : \"edit_team_member_time_entries\"},{\"enabled\" : false,\"feature_id\" : 54,\"name\" : \"edit_team_member_profile\"},{\"enabled\" : false,\"feature_id\" : 55,\"name\" : \"tracking_reminders\"},{\"enabled\" : false,\"feature_id\" : 56,\"name\" : \"time_entry_constraints\"},{\"enabled\" : false,\"feature_id\" : 57,\"name\" : \"priority_support\"},{\"enabled\" : false,\"feature_id\" : 58,\"name\" : \"labour_cost\"},{\"enabled\" : false,\"feature_id\" : 59,\"name\" : \"report_employee_profitability\"},{\"enabled\" : false,\"feature_id\" : 60,\"name\" : \"report_project_profitability\"},{\"enabled\" : false,\"feature_id\" : 61,\"name\" : \"report_comparative\"},{\"enabled\" : false,\"feature_id\" : 62,\"name\" : \"report_data_trends\"},{\"enabled\" : false,\"feature_id\" : 63,\"name\" : \"report_export_xlsx\"},{\"enabled\" : false,\"feature_id\" : 64,\"name\" : \"tasks\"},{\"enabled\" : false,\"feature_id\" : 65,\"name\" : \"project_dashboard\"}],\"workspace_id\" : 2817276}],\"workspaces\" : [{\"admin\" : true,\"api_token\" : \"token\",\"at\" : \"2018-09-01T08:27:56+00:00\",\"business_ws\" : false,\"csv_upload\" : null,\"default_currency\" : \"USD\",\"default_hourly_rate\" : 0,\"ical_enabled\" : true,\"ical_url\" : \"/ical/workspace_user/cf8775d2110d5d874ffa633434c901bd\",\"id\" : 2817276,\"logo_url\" : \"https://assets.toggl.com/images/workspace.jpg\",\"name\" : \"workspace\",\"only_admins_may_create_projects\" : false,\"only_admins_see_billable_rates\" : false,\"only_admins_see_team_dashboard\" : false,\"premium\" : true,\"profile\" : 0,\"projects_billable_by_default\" : true,\"rounding\" : 1,\"rounding_minutes\" : 0,\"server_deleted_at\" : null,\"subscription\" : null,\"suspended_at\" : null}]}" };
    User user;
    error err = user.LoadUserAndRelatedDataFromJSONString(json, false, true);
    ASSERT_EQ(err, noError);

    ASSERT_EQ(user.Email(), "m@rtinbriza.cz");
    ASSERT_EQ(user.Fullname(), "M");
    ASSERT_EQ(user.ID(), 4187712);

    ASSERT_EQ(user.related.Clients.size(), 1);
    ASSERT_EQ(user.related.Projects.size(), 1);
    ASSERT_EQ(user.related.Tags.size(), 1);
    ASSERT_EQ(user.related.TimeEntries.size(), 1);
    ASSERT_EQ(user.related.Workspaces.size(), 1);

    ASSERT_EQ(user.related.Clients[0]->ID(), 43289164);
    ASSERT_EQ(user.related.Clients[0]->Name(), "client");
    ASSERT_EQ(user.related.Clients[0]->WID(), 2817276);

    ASSERT_EQ(user.related.Projects[0]->ID(), 154073509);
    ASSERT_EQ(user.related.Projects[0]->Name(), "project");
    ASSERT_EQ(user.related.Projects[0]->WID(), 2817276);
    ASSERT_EQ(user.related.Projects[0]->CID(), 43289164);
    ASSERT_EQ(user.related.Projects[0]->Active(), true);
    ASSERT_EQ(user.related.Projects[0]->Billable(), false);

    ASSERT_EQ(user.related.Tags[0]->ID(), 6892625);
    ASSERT_EQ(user.related.Tags[0]->Name(), "tag");
    ASSERT_EQ(user.related.Tags[0]->WID(), 2817276);

    ASSERT_EQ(user.related.TimeEntries[0]->ID(), 1563187599);
    ASSERT_EQ(user.related.TimeEntries[0]->Description(), "time entry");
    ASSERT_EQ(user.related.TimeEntries[0]->WID(), 2817276);
    ASSERT_EQ(user.related.TimeEntries[0]->UID(), 4187712);

    ASSERT_EQ(user.related.TimeEntries[0]->StartTime(), 1590593622);
    ASSERT_EQ(user.related.TimeEntries[0]->StopTime(), 1590594037);

    ASSERT_EQ(user.related.TimeEntries[0]->Duration(), 415);

    ASSERT_EQ(user.related.Workspaces[0]->ID(), 2817276);
    ASSERT_EQ(user.related.Workspaces[0]->Name(), "workspace");
    ASSERT_EQ(user.related.Workspaces[0]->Admin(), true);
    ASSERT_EQ(user.related.Workspaces[0]->Premium(), true);
}

}  // namespace toggl

int main(int argc, char **argv) {
    Poco::Logger &logger = Poco::Logger::get("");
#ifdef unix
    // log straight to stderr
    //logger.setChannel(new Poco::ConsoleChannel());
#endif
    logger.setLevel(Poco::Message::PRIO_DEBUG);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

// Copyright 2014 Toggl Desktop developers.

#include "gtest/gtest.h"

#include "./../user.h"
#include "./../workspace.h"
#include "./../client.h"
#include "./../project.h"
#include "./../task.h"
#include "./../time_entry.h"
#include "./../tag.h"
#include "./../database.h"
#include "./test_data.h"
#include "./../json.h"
#include "./../formatter.h"

#include "Poco/FileStream.h"
#include "Poco/File.h"
#include "Poco/LocalDateTime.h"

namespace toggl {

namespace testing {
class Database {
 public:
    Database() : db_(0) {
        Poco::File f(TESTDB);
        if (f.exists()) {
            f.remove(false);
        }
        db_ = new toggl::Database(TESTDB);
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

TEST(AppTest, TimeEntryReturnsTags) {
    TimeEntry te;
    te.SetTags("alfa|beeta");
    ASSERT_EQ(std::string("alfa|beeta"), te.Tags());
}

TEST(AppTest, ProjectsHaveColorCodes) {
    Project p;
    p.SetColor("1");
    ASSERT_EQ("#bc85e6", p.ColorCode());
    p.SetColor("");
    ASSERT_EQ("#999999", p.ColorCode());
    p.SetColor("-10");
    ASSERT_EQ("#14a88e", p.ColorCode());
    p.SetColor("0");
    ASSERT_EQ("#999999", p.ColorCode());
    p.SetColor("999");
    ASSERT_EQ("#a4506c", p.ColorCode());
}

TEST(AppTest, SaveAndLoadCurrentAPIToken) {
    testing::Database db;
    std::string api_token("");
    ASSERT_EQ(noError, db.instance()->CurrentAPIToken(&api_token));
    ASSERT_EQ("", api_token);

    api_token = "abc123";
    ASSERT_EQ(noError, db.instance()->SetCurrentAPIToken(api_token));
    ASSERT_EQ(noError, db.instance()->SetCurrentAPIToken(api_token));

    Poco::UInt64 n(0);
    ASSERT_EQ(noError,
              db.instance()->UInt("select count(1) from sessions", &n));
    ASSERT_EQ(Poco::UInt64(1), n);

    std::string api_token_from_db("");
    ASSERT_EQ(noError, db.instance()->CurrentAPIToken(&api_token_from_db));
    ASSERT_EQ("abc123", api_token_from_db);

    ASSERT_EQ(noError, db.instance()->ClearCurrentAPIToken());
    ASSERT_EQ(noError,
              db.instance()->UInt("select count(1) from sessions", &n));
    ASSERT_EQ(Poco::UInt64(0), n);

    ASSERT_EQ(noError, db.instance()->CurrentAPIToken(&api_token_from_db));
    ASSERT_EQ("", api_token_from_db);
}

TEST(AppTest, UpdatesTimeEntryFromJSON) {
    User user;
    user.LoadUserAndRelatedDataFromJSONString(loadTestData());

    TimeEntry *te = user.related.TimeEntryByID(89818605);
    ASSERT_TRUE(te);

    std::string json = "{\"id\":89818605,\"description\":\"Changed\"}";
    te->LoadFromJSONString(json);
    ASSERT_EQ("Changed", te->Description());
}

TEST(AppTest, AllowsSameEmail) {
    testing::Database db;

    User user;
    user.LoadUserAndRelatedDataFromJSONString(loadTestData());

    std::vector<ModelChange> changes;
    ASSERT_EQ(noError, db.instance()->SaveUser(&user, true, &changes));

    User user2;
    std::string json = loadTestDataFile("../testdata/same_email.json");
    user2.LoadUserAndRelatedDataFromJSONString(json);

    ASSERT_EQ(noError, db.instance()->SaveUser(&user2, true, &changes));

    ASSERT_EQ(user.Email(), user2.Email());
    ASSERT_NE(user.ID(), user2.ID());
    ASSERT_NE(user.APIToken(), user2.APIToken());
}

TEST(AppTest, EscapeJSONString) {
    std::string text("https://github.com/bartschuller");
    ASSERT_EQ(text, Formatter::EscapeJSONString(text));
    ASSERT_EQ("\"", Formatter::EscapeJSONString("\""));
    ASSERT_EQ(" ", Formatter::EscapeJSONString("\t"));
}

TEST(AppTest, EscapeControlCharactersInJSONString) {
    std::string text("\x16");
    ASSERT_EQ(" ", Formatter::EscapeJSONString(text));
}

TEST(AppTest, UpdatesTimeEntryFromFullUserJSON) {
    testing::Database db;

    std::string json = loadTestData();

    User user;
    user.LoadUserAndRelatedDataFromJSONString(loadTestData());

    TimeEntry *te = user.related.TimeEntryByID(89818605);
    ASSERT_TRUE(te);

    size_t n = json.find("Important things");
    ASSERT_TRUE(n);
    json = json.replace(n,
                        std::string("Important things").length(),
                        "Even more important!");

    user.LoadUserAndRelatedDataFromJSONString(json);
    te = user.related.TimeEntryByID(89818605);
    ASSERT_TRUE(te);
    ASSERT_EQ("Even more important!", te->Description());
}

TEST(AppTest, SavesAndLoadsUserFields) {
    testing::Database db;

    User user;
    user.LoadUserAndRelatedDataFromJSONString(loadTestData());

    ASSERT_TRUE(user.StoreStartAndStopTime());
    // Change fields
    user.SetStoreStartAndStopTime(false);

    std::vector<ModelChange> changes;
    ASSERT_EQ(noError, db.instance()->SaveUser(&user, true, &changes));

    // Load user into another instance
    User user2;
    ASSERT_EQ(noError, db.instance()->LoadUserByID(user.ID(), &user2));
    ASSERT_FALSE(user2.StoreStartAndStopTime());

    // Change fields, again
    user.SetStoreStartAndStopTime(true);
    ASSERT_EQ(noError, db.instance()->SaveUser(&user, true, &changes));

    // Load user into another instance
    User user3;
    ASSERT_EQ(noError, db.instance()->LoadUserByID(user.ID(), &user3));
    ASSERT_TRUE(user3.StoreStartAndStopTime());
}

TEST(AppTest, SavesModelsAndKnowsToUpdateWithSameUserInstance) {
    testing::Database db;

    User user;
    user.LoadUserAndRelatedDataFromJSONString(loadTestData());

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

TEST(AppTest,
     SavesModelsAndKnowsToUpdateWithSeparateUserInstances) {
    testing::Database db;

    std::string json = loadTestData();

    User user1;
    user1.LoadUserAndRelatedDataFromJSONString(json);

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

    user2.LoadUserAndRelatedDataFromJSONString(json);

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

TEST(AppTest, TestStartTimeEntryWithDuration) {
    testing::Database db;

    User user;
    user.LoadUserAndRelatedDataFromJSONString(loadTestData());

    size_t count = user.related.TimeEntries.size();

    user.Start("Old work", "1 hour", 0, 0);

    ASSERT_EQ(count + 1, user.related.TimeEntries.size());

    TimeEntry *te = user.related.TimeEntries[user.related.TimeEntries.size()-1];

    ASSERT_EQ(3600, te->DurationInSeconds());
}

TEST(AppTest, TestStartTimeEntryWithoutDuration) {
    testing::Database db;

    User user;
    user.LoadUserAndRelatedDataFromJSONString(loadTestData());

    user.Start("Old work", "", 0, 0);

    TimeEntry *te = user.RunningTimeEntry();
    ASSERT_TRUE(te);
    ASSERT_GT(0, te->DurationInSeconds());
}

TEST(AppTest, TestDeletionSteps) {
    testing::Database db;

    User user;
    user.LoadUserAndRelatedDataFromJSONString(loadTestData());

    // first, mark time entry as deleted
    user.Start("My new time entry", "", 0, 0);
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

TEST(AppTest, SavesModels) {
    User user;
    user.LoadUserAndRelatedDataFromJSONString(loadTestData());

    testing::Database db;

    std::vector<ModelChange> changes;
    ASSERT_EQ(noError, db.instance()->SaveUser(&user, false, &changes));

    ASSERT_EQ(noError, db.instance()->SaveUser(&user, false, &changes));
}

TEST(AppTest, AssignsGUID) {
    std::string json = loadTestData();
    ASSERT_FALSE(json.empty());

    User user;
    user.LoadUserAndRelatedDataFromJSONString(json);

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

TEST(AppTest, ParsesAndSavesData) {
    std::string json = loadTestData();
    ASSERT_FALSE(json.empty());

    User user;
    user.LoadUserAndRelatedDataFromJSONString(json);
    ASSERT_EQ(Poco::UInt64(1379068550), user.Since());
    ASSERT_EQ(Poco::UInt64(10471231), user.ID());
    ASSERT_EQ(Poco::UInt64(123456788), user.DefaultWID());
    ASSERT_EQ("30eb0ae954b536d2f6628f7fec47beb6", user.APIToken());
    ASSERT_EQ("John Smith", user.Fullname());

    // Projects
    ASSERT_EQ(uint(2), user.related.Projects.size());

    ASSERT_EQ(uint(2598305), user.related.Projects[0]->ID());
    ASSERT_EQ("2f0b8f51-f898-d992-3e1a-6bc261fc41ef",
              user.related.Projects[0]->GUID());
    ASSERT_EQ(uint(123456789), user.related.Projects[0]->WID());
    ASSERT_EQ("Testing stuff", user.related.Projects[0]->Name());
    ASSERT_EQ("21", user.related.Projects[0]->Color());
    ASSERT_EQ(user.ID(), user.related.Projects[0]->UID());
    ASSERT_FALSE(user.related.Projects[0]->Active());

    ASSERT_EQ(uint(2567324), user.related.Projects[1]->ID());
    ASSERT_EQ(uint(123456789), user.related.Projects[1]->WID());
    ASSERT_EQ("Even more work", user.related.Projects[1]->Name());
    ASSERT_EQ(uint(1385144), user.related.Projects[1]->CID());
    ASSERT_EQ(user.ID(), user.related.Projects[1]->UID());
    ASSERT_TRUE(user.related.Projects[1]->Active());

    // Time entries
    ASSERT_EQ(uint(5), user.related.TimeEntries.size());

    ASSERT_EQ(uint(89818605), user.related.TimeEntries[0]->ID());
    ASSERT_EQ("07fba193-91c4-0ec8-2894-820df0548a8f",
              user.related.TimeEntries[0]->GUID());
    ASSERT_EQ(uint(2567324), user.related.TimeEntries[0]->PID());
    ASSERT_TRUE(user.related.TimeEntries[0]->Billable());
    ASSERT_EQ(uint(1378362830), user.related.TimeEntries[0]->Start());
    ASSERT_EQ(uint(1378369186), user.related.TimeEntries[0]->Stop());
    ASSERT_EQ(6356, user.related.TimeEntries[0]->DurationInSeconds());
    ASSERT_EQ("Important things",
              user.related.TimeEntries[0]->Description());
    ASSERT_EQ(uint(0), user.related.TimeEntries[0]->TagNames.size());
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
    ASSERT_EQ("", user.related.Tags[0]->GUID());

    ASSERT_EQ(uint(36253522), user.related.Tags[1]->ID());
    ASSERT_EQ("create new", user.related.Tags[1]->Name());
    ASSERT_EQ(user.ID(), user.related.Tags[1]->UID());
    ASSERT_EQ(uint(123456788), user.related.Tags[1]->WID());
    ASSERT_EQ("041390ba-ed9c-b477-b949-1a4ebb60a9ce",
              user.related.Tags[1]->GUID());

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
    ASSERT_EQ(uint(123456789), user.related.Clients[0]->WID());
    ASSERT_EQ("Big Client", user.related.Clients[0]->Name());
    ASSERT_EQ("59b464cd-0f8e-e601-ff44-f135225a6738",
              user.related.Clients[0]->GUID());
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

TEST(AppTest, ParsesDurationLikeOnTheWeb) {
    TimeEntry te;

    te.SetDurationUserInput("00:00:15");
    ASSERT_EQ("00:00:15", te.DurationString());

    te.SetDurationInSeconds(0);
    te.SetDurationUserInput("00:23:15");
    ASSERT_EQ("00:23:15", te.DurationString());

    te.SetDurationInSeconds(0);
    te.SetDurationUserInput("12:34:56");
    ASSERT_EQ("12:34:56", te.DurationString());

    te.SetDurationInSeconds(0);
    te.SetDurationUserInput("0:1");
    ASSERT_EQ("00:01:00", te.DurationString());

    te.SetDurationInSeconds(0);
    te.SetDurationUserInput("1:2");
    ASSERT_EQ("01:02:00", te.DurationString());

    te.SetDurationInSeconds(0);
    te.SetDurationUserInput("1:0");
    ASSERT_EQ("01:00:00", te.DurationString());

    te.SetDurationInSeconds(0);
    te.SetDurationUserInput("05:22 min");
    ASSERT_EQ("00:05:22", te.DurationString());

    te.SetDurationInSeconds(0);
    te.SetDurationUserInput("00:22 min");
    ASSERT_EQ("00:00:22", te.DurationString());

    te.SetDurationInSeconds(0);
    te.SetDurationUserInput("0 hours");
    ASSERT_EQ("00:00:00", te.DurationString());

    te.SetDurationInSeconds(0);
    te.SetDurationUserInput("0.5 hours");
    ASSERT_EQ("00:30:00", te.DurationString());

    te.SetDurationInSeconds(0);
    te.SetDurationUserInput("0,5 hours");
    ASSERT_EQ("00:30:00", te.DurationString());

    te.SetDurationInSeconds(0);
    te.SetDurationUserInput("1 hour");
    ASSERT_EQ("01:00:00", te.DurationString());

    te.SetDurationInSeconds(0);
    te.SetDurationUserInput("1 hr");
    ASSERT_EQ("01:00:00", te.DurationString());

    te.SetDurationInSeconds(0);
    te.SetDurationUserInput("1,5 hours");
    ASSERT_EQ("01:30:00", te.DurationString());

    te.SetDurationInSeconds(0);
    te.SetDurationUserInput("1.5 hours");
    ASSERT_EQ("01:30:00", te.DurationString());

    te.SetDurationInSeconds(0);
    te.SetDurationUserInput("24 hours");
    ASSERT_EQ(86400, te.DurationInSeconds());

    te.SetDurationInSeconds(0);
    te.SetDurationUserInput("0 minutes");
    ASSERT_EQ("00:00:00", te.DurationString());

    te.SetDurationInSeconds(0);
    te.SetDurationUserInput("0 min");
    ASSERT_EQ("00:00:00", te.DurationString());

    te.SetDurationInSeconds(0);
    te.SetDurationUserInput("5 minutes");
    ASSERT_EQ("00:05:00", te.DurationString());

    te.SetDurationInSeconds(0);
    te.SetDurationUserInput("5minutes");
    ASSERT_EQ("00:05:00", te.DurationString());

    te.SetDurationInSeconds(0);
    te.SetDurationUserInput("0,5 minutes");
    ASSERT_EQ("00:00:30", te.DurationString());

    te.SetDurationInSeconds(0);
    te.SetDurationUserInput("1 minute");
    ASSERT_EQ("00:01:00", te.DurationString());

    te.SetDurationInSeconds(0);
    te.SetDurationUserInput("1,5 minutes");
    ASSERT_EQ("00:01:30", te.DurationString());

    te.SetDurationInSeconds(0);
    te.SetDurationUserInput("1.5 minutes");
    ASSERT_EQ("00:01:30", te.DurationString());

    te.SetDurationInSeconds(0);
    te.SetDurationUserInput("15");
    ASSERT_EQ("00:15:00", te.DurationString());

    te.SetDurationInSeconds(0);
    te.SetDurationUserInput("0 seconds");
    ASSERT_EQ("00:00:00", te.DurationString());

    te.SetDurationInSeconds(0);
    te.SetDurationUserInput("1 second");
    ASSERT_EQ("00:00:01", te.DurationString());

    te.SetDurationInSeconds(0);
    te.SetDurationUserInput("1.5h");
    ASSERT_EQ("01:30:00", te.DurationString());

    te.SetDurationInSeconds(0);
    te.SetDurationUserInput("1.5 h");
    ASSERT_EQ("01:30:00", te.DurationString());

    te.SetDurationInSeconds(0);
    te.SetDurationUserInput("3h");
    ASSERT_EQ("03:00:00", te.DurationString());

    te.SetDurationInSeconds(0);
    te.SetDurationUserInput("3 h");
    ASSERT_EQ("03:00:00", te.DurationString());

    te.SetDurationInSeconds(0);
    te.SetDurationUserInput("15m");
    ASSERT_EQ("00:15:00", te.DurationString());

    te.SetDurationInSeconds(0);
    te.SetDurationUserInput("15 m");
    ASSERT_EQ("00:15:00", te.DurationString());

    te.SetDurationInSeconds(0);
    te.SetDurationUserInput("25s");
    ASSERT_EQ("00:00:25", te.DurationString());

    te.SetDurationInSeconds(0);
    te.SetDurationUserInput("25 s");
    ASSERT_EQ("00:00:25", te.DurationString());

    te.SetDurationInSeconds(0);
    te.SetDurationUserInput("1.5");
    ASSERT_EQ("01:30:00", te.DurationString());

    te.SetDurationInSeconds(0);
    te.SetDurationUserInput("1,5");
    ASSERT_EQ("01:30:00", te.DurationString());

    te.SetDurationInSeconds(0);
    te.SetDurationUserInput("0.25");
    ASSERT_EQ("00:15:00", te.DurationString());

    te.SetDurationInSeconds(0);
    te.SetDurationUserInput("0.025");
    ASSERT_EQ("00:01:30", te.DurationString());

    te.SetDurationInSeconds(0);
    te.SetDurationUserInput("2h45");
    ASSERT_EQ("02:45:00", te.DurationString());

    te.SetDurationInSeconds(0);
    te.SetDurationUserInput("2h");
    ASSERT_EQ("02:00:00", te.DurationString());

    te.SetDurationInSeconds(0);
    te.SetDurationUserInput("2h 18m");
    ASSERT_EQ("02:18:00", te.DurationString());

    te.SetDurationInSeconds(0);
    te.SetDurationUserInput("2h 18m 50s");
    ASSERT_EQ("02:18:50", te.DurationString());

    te.SetDurationInSeconds(0);
    te.SetDurationUserInput("1hr 25min 30sec");
    ASSERT_EQ("01:25:30", te.DurationString());

    te.SetDurationInSeconds(0);
    te.SetDurationUserInput("1 hours 25 minutes 30 seconds");
    ASSERT_EQ("01:25:30", te.DurationString());

    te.SetDurationInSeconds(0);
    te.SetDurationUserInput("1 hour 1 minute 1 second");
    ASSERT_EQ("01:01:01", te.DurationString());
}

TEST(AppTest, ParseDurationLargerThan24Hours) {
    TimeEntry te;

    te.SetDurationInSeconds(0);
    te.SetDurationUserInput("90:10:00");
    ASSERT_EQ("90:10:00", te.DurationString());
}

TEST(AppTest, InterpretsCrazyStartAndStopAsMissingValues) {
    TimeEntry te;

    ASSERT_EQ(Poco::UInt64(0), te.Start());
    te.SetStartString("0003-03-16T-7:-19:-24Z");
    ASSERT_EQ(Poco::UInt64(0), te.Start());

    ASSERT_EQ(Poco::UInt64(0), te.Stop());
    te.SetStopString("0003-03-16T-5:-52:-51Z");
    ASSERT_EQ(Poco::UInt64(0), te.Stop());
}

TEST(AppTest, Continue) {
    testing::Database db;

    User user;
    user.LoadUserAndRelatedDataFromJSONString(loadTestData());

    // User wants to continue time entries,
    // not create new ones
    user.SetStoreStartAndStopTime(false);

    // Change an old time entry and
    // change its date to today. Continueing the
    // entry should not create new record, but
    // continue the old one.
    TimeEntry *te = user.related.TimeEntryByID(89818605);
    ASSERT_TRUE(te);
    te->SetStart(time(0));
    te->SetDurOnly(true);

    size_t count = user.related.TimeEntries.size();
    ASSERT_EQ(toggl::noError, user.Continue(te->GUID()));
    ASSERT_EQ(count, user.related.TimeEntries.size());

    // If the old time entry date is different than
    // today, it should create a new entry when
    // user continues it:
    te->SetStartString("2013-01-25T01:05:15-22:00");
    ASSERT_EQ(toggl::noError, user.Continue(te->GUID()));
    ASSERT_EQ(count+1, user.related.TimeEntries.size());
}

TEST(AppTest, SetDurationOnRunningTimeEntryWithDurOnlySetting) {
    testing::Database db;

    std::string json = loadTestDataFile("../testdata/user_with_duronly.json");

    User user;
    user.LoadUserAndRelatedDataFromJSONString(json);

    TimeEntry *te = user.related.TimeEntryByID(164891639);
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

TEST(FormatterTest, FormatTimeForTimeEntryEditor) {
    ASSERT_EQ("", Formatter::FormatTimeForTimeEntryEditor(0, "H:mm"));

    //  date -r 1412950844
    //  Fri Oct 10 16:20:44 CEST 2014
    time_t t(1412950844);

    std::string res = Formatter::FormatTimeForTimeEntryEditor(t, "H:mm");
    ASSERT_NE(std::string::npos, res.find(":20"));

    res = Formatter::FormatTimeForTimeEntryEditor(t, "h:mm A");
    ASSERT_NE(std::string::npos, res.find(":20"));
}

TEST(FormatterTest, FormatDateHeader) {
    ASSERT_EQ("", Formatter::FormatDateHeader(0));

    //  date -r 1412120844
    //  Wed Oct  1 01:47:24 CEST 2014
    time_t t(1412120844);
    std::string res = Formatter::FormatDateHeader(t);
    ASSERT_TRUE("Wed 01. Oct" == res || "Tue 30. Sep" == res);

    t = time(0);
    ASSERT_EQ("Today", Formatter::FormatDateHeader(t));

    t = t - 86400;
    ASSERT_EQ("Yesterday", Formatter::FormatDateHeader(t));
}


TEST(FormatterTest, ParseLastDate) {
    ASSERT_EQ(0, Formatter::ParseLastDate(0, 0));

    time_t now(0);
    Poco::DateTime now_date(Poco::Timestamp::fromEpochTime(now));

    //  date -r 1412220844
    //  Thu Oct  2 05:34:04 CEST 2014
    time_t last(1412220844);
    Poco::DateTime last_date(Poco::Timestamp::fromEpochTime(last));

    time_t res = Formatter::ParseLastDate(last, now);
    Poco::DateTime res_date(Poco::Timestamp::fromEpochTime(res));

    ASSERT_EQ(last_date.year(), res_date.year());
    ASSERT_EQ(last_date.month(), res_date.month());
    ASSERT_EQ(last_date.day(), res_date.day());

    ASSERT_EQ(now_date.hour(), res_date.hour());
    ASSERT_EQ(now_date.minute(), res_date.minute());
}

TEST(FormatterTest, Format8601) {
    ASSERT_EQ("null", Formatter::Format8601(0));

    //  date -r 1412220844
    //  Thu Oct  2 05:34:04 CEST 2014
    time_t t(1412220844);

    ASSERT_EQ("2014-10-02T03:34:04Z", Formatter::Format8601(t));
}

TEST(FormatterTest, Parse8601) {
    ASSERT_EQ(0, Formatter::Parse8601("null"));

    //  date -r 1412220844
    //  Thu Oct  2 05:34:04 CEST 2014
    time_t t(1412220844);

    ASSERT_EQ(t, Formatter::Parse8601("2014-10-02T03:34:04Z"));

    ASSERT_EQ(0, Formatter::Parse8601("invalid value"));
}

TEST(FormatterTest, FormatDurationInSecondsPrettyHHMM) {
    ASSERT_EQ("0 min", Formatter::FormatDurationInSecondsToHM(0));
    ASSERT_EQ("0 min", Formatter::FormatDurationInSecondsToHM(30));
    ASSERT_EQ("2 min", Formatter::FormatDurationInSecondsToHM(120));
}

TEST(FormatterTest, JoinTaskName) {
    std::string res = Formatter::JoinTaskName(0, 0, 0);
    ASSERT_EQ("", res);

    Task t;
    t.SetName("Task name");
    res = Formatter::JoinTaskName(&t, 0, 0);
    ASSERT_EQ("Task name", res);

    Project p;
    p.SetName("Project name");
    res = Formatter::JoinTaskName(0, &p, 0);
    ASSERT_EQ(p.Name(), res);

    res = Formatter::JoinTaskName(&t, &p, 0);
    ASSERT_EQ("Task name. Project name", res);

    Client c;
    c.SetName("Customer name");
    res = Formatter::JoinTaskName(0, 0, &c);
    ASSERT_EQ(c.Name(), res);

    res = Formatter::JoinTaskName(&t, 0, &c);
    ASSERT_EQ("Task name. Customer name", res);

    res = Formatter::JoinTaskName(0, &p, &c);
    ASSERT_EQ("Project name. Customer name", res);

    res = Formatter::JoinTaskName(&t, &p, &c);
    ASSERT_EQ("Task name. Project name. Customer name", res);
}

TEST(FormatterTest, JoinTaskNameReverse) {
    std::string res = Formatter::JoinTaskName(0, 0, 0);
    ASSERT_EQ("", res);

    Task t;
    t.SetName("Task name");
    res = Formatter::JoinTaskNameReverse(&t, 0, 0);
    ASSERT_EQ("Task name", res);

    Project p;
    p.SetName("Project name");
    res = Formatter::JoinTaskNameReverse(0, &p, 0);
    ASSERT_EQ(p.Name(), res);

    res = Formatter::JoinTaskNameReverse(&t, &p, 0);
    ASSERT_EQ("Project name. Task name", res);

    Client c;
    c.SetName("Customer name");
    res = Formatter::JoinTaskNameReverse(0, 0, &c);
    ASSERT_EQ(c.Name(), res);

    res = Formatter::JoinTaskNameReverse(&t, 0, &c);
    ASSERT_EQ("Customer name. Task name", res);

    res = Formatter::JoinTaskNameReverse(0, &p, &c);
    ASSERT_EQ("Customer name. Project name", res);

    res = Formatter::JoinTaskNameReverse(&t, &p, &c);
    ASSERT_EQ("Customer name. Project name. Task name", res);
}

}  // namespace toggl

int main(int argc, char **argv) {
    Poco::Logger &logger = Poco::Logger::get("");
    logger.setLevel(Poco::Message::PRIO_DEBUG);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

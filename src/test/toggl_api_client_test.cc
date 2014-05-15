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

namespace kopsik {

namespace testing {
class Database {
 public:
    Database() : db_(0) {
        Poco::File f(TESTDB);
        if (f.exists()) {
            f.remove(false);
        }
        db_ = new kopsik::Database(TESTDB);
    }
    ~Database() {
        if (db_) {
            delete db_;
        }
    }
    kopsik::Database *instance() {
        return db_;
    }

 private:
    kopsik::Database *db_;
};
}  // namespace testing

TEST(TogglApiClientTest, TimeEntryReturnsTags) {
    TimeEntry te;
    te.SetTags("alfa|beeta");
    ASSERT_EQ(std::string("alfa|beeta"), te.Tags());
}

TEST(TogglApiClientTest, ProjectsHaveColorCodes) {
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

TEST(TogglApiClientTest, SaveAndLoadCurrentAPIToken) {
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

TEST(TogglApiClientTest, UpdatesTimeEntryFromJSON) {
    testing::Database db;

    User user("kopsik_test", "0.1");
    LoadUserFromJSONString(&user, loadTestData(), true, true);

    TimeEntry *te = user.TimeEntryByID(89818605);
    ASSERT_TRUE(te);

    std::string json = "{\"id\":89818605,\"description\":\"Changed\"}";
    te->LoadFromJSONString(json);
    ASSERT_EQ("Changed", te->Description());
}

TEST(TogglApiClientTest, EscapeJSONString) {
    std::string text("https://github.com/bartschuller");
    ASSERT_EQ(text, Formatter::EscapeJSONString(text));
}

TEST(TogglApiClientTest, EscapeControlCharactersInJSONString) {
    std::string text("\x16");
    ASSERT_EQ(" ", Formatter::EscapeJSONString(text));
}

TEST(TogglApiClientTest, UpdatesTimeEntryFromFullUserJSON) {
    testing::Database db;

    std::string json = loadTestData();

    User user("kopsik_test", "0.1");
    LoadUserFromJSONString(&user, loadTestData(), true, true);

    TimeEntry *te = user.TimeEntryByID(89818605);
    ASSERT_TRUE(te);

    size_t n = json.find("Important things");
    ASSERT_TRUE(n);
    json = json.replace(n,
                        std::string("Important things").length(),
                        "Even more important!");

    LoadUserFromJSONString(&user, json, true, true);
    te = user.TimeEntryByID(89818605);
    ASSERT_TRUE(te);
    ASSERT_EQ("Even more important!", te->Description());
}

TEST(TogglApiClientTest, SavesAndLoadsUserFields) {
    testing::Database db;

    User user("kopsik_test", "0.1");
    LoadUserFromJSONString(&user, loadTestData(), true, true);

    ASSERT_TRUE(user.StoreStartAndStopTime());
    // Change fields
    user.SetStoreStartAndStopTime(false);

    std::vector<ModelChange> changes;
    ASSERT_EQ(noError, db.instance()->SaveUser(&user, true, &changes));

    // Load user into another instance
    User user2("kopsik_test", "0.1");
    ASSERT_EQ(noError, db.instance()->LoadUserByID(user.ID(), &user2, false));
    ASSERT_FALSE(user2.StoreStartAndStopTime());

    // Change fields, again
    user.SetStoreStartAndStopTime(true);
    ASSERT_EQ(noError, db.instance()->SaveUser(&user, true, &changes));

    // Load user into another instance
    User user3("kopsik_test", "0.1");
    ASSERT_EQ(noError, db.instance()->LoadUserByID(user.ID(), &user3, false));
    ASSERT_TRUE(user3.StoreStartAndStopTime());
}

TEST(TogglApiClientTest, SavesModelsAndKnowsToUpdateWithSameUserInstance) {
    testing::Database db;

    User user("kopsik_test", "0.1");
    LoadUserFromJSONString(&user, loadTestData(), true, true);

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

TEST(TogglApiClientTest,
     SavesModelsAndKnowsToUpdateWithSeparateUserInstances) {
    testing::Database db;

    std::string json = loadTestData();

    User user1("kopsik_test", "0.1");
    LoadUserFromJSONString(&user1, json, true, true);

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

    User user2("kopsik_test", "0.1");
    ASSERT_EQ(noError, db.instance()->LoadUserByID(user1.ID(), &user2, true));

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

    LoadUserFromJSONString(&user2, json, true, true);

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

TEST(TogglApiClientTest, TestStartTimeEntryWithDuration) {
    testing::Database db;

    User user("kopsik_test", "0.1");
    LoadUserFromJSONString(&user, loadTestData(), true, true);

    TimeEntry *te = user.Start("Old work", "1 hour", 0, 0);
    ASSERT_EQ(3600, te->DurationInSeconds());
}

TEST(TogglApiClientTest, TestStartTimeEntryWithoutDuration) {
    testing::Database db;

    User user("kopsik_test", "0.1");
    LoadUserFromJSONString(&user, loadTestData(), true, true);

    TimeEntry *te = user.Start("Old work", "1 hour", 0, 0);
    ASSERT_LT(0, te->DurationInSeconds());
}

TEST(TogglApiClientTest, TestDeletionSteps) {
    testing::Database db;

    User user("kopsik_test", "0.1");
    LoadUserFromJSONString(&user, loadTestData(), true, true);

    // first, mark time entry as deleted
    TimeEntry *te = user.Start("My new time entry", "", 0, 0);
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

TEST(TogglApiClientTest, SavesModels) {
    User user("kopsik_test", "0.1");
    LoadUserFromJSONString(&user, loadTestData(), true, true);

    testing::Database db;

    std::vector<ModelChange> changes;
    ASSERT_EQ(noError, db.instance()->SaveUser(&user, false, &changes));

    ASSERT_EQ(noError, db.instance()->SaveUser(&user, false, &changes));
}

TEST(TogglApiClientTest, AssignsGUID) {
    std::string json = loadTestData();
    ASSERT_FALSE(json.empty());

    User user("kopsik_test", "0.1");
    LoadUserFromJSONString(&user, json, true, true);

    ASSERT_EQ(uint(5), user.related.TimeEntries.size());
    TimeEntry *te = user.TimeEntryByID(89837445);
    ASSERT_TRUE(te);

    ASSERT_NE("", te->GUID());
    ASSERT_TRUE(te->GUID().size());

    TimeEntry *te2 = user.TimeEntryByGUID(te->GUID());
    ASSERT_TRUE(te2);

    ASSERT_EQ(te->GUID(), te2->GUID());
    ASSERT_EQ(te->ID(), te2->ID());
}

TEST(TogglApiClientTest, ParsesAndSavesData) {
    std::string json = loadTestData();
    ASSERT_FALSE(json.empty());

    User user("kopsik_test", "0.1");
    LoadUserFromJSONString(&user, json, true, true);
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
    User user2("kopsik_test", "0.1");
    ASSERT_EQ(noError, db.instance()->LoadUserByID(user.ID(), &user2, true));

    ASSERT_TRUE(user2.ID());
    ASSERT_EQ(user.ID(), user2.ID());
    ASSERT_TRUE(user2.Since());
    ASSERT_EQ(user.Since(), user2.Since());
    ASSERT_EQ(user.ID(), user2.ID());
    ASSERT_EQ(user.DefaultWID(), user2.DefaultWID());

    ASSERT_EQ(uint(2), user2.related.Projects.size());
    Project *project_from_db =
        user2.ProjectByID(user.related.Projects[0]->ID());
    ASSERT_TRUE(project_from_db);
    ASSERT_EQ(user.related.Projects[0]->String(),
              project_from_db->String());
    project_from_db = user2.ProjectByID(user.related.Projects[1]->ID());
    ASSERT_EQ(user.related.Projects[1]->String(),
              project_from_db->String());

    ASSERT_EQ(uint(5), user2.related.TimeEntries.size());
    TimeEntry *te_from_db =
        user2.TimeEntryByID(user.related.TimeEntries[0]->ID());
    ASSERT_TRUE(te_from_db);
    ASSERT_EQ(user.related.TimeEntries[0]->String(), te_from_db->String());
    te_from_db = user2.TimeEntryByID(user.related.TimeEntries[1]->ID());
    ASSERT_TRUE(te_from_db);
    ASSERT_EQ(user.related.TimeEntries[1]->String(), te_from_db->String());
    te_from_db = user2.TimeEntryByID(user.related.TimeEntries[2]->ID());
    ASSERT_TRUE(te_from_db);
    ASSERT_EQ(user.related.TimeEntries[2]->String(), te_from_db->String());

    ASSERT_EQ(uint(2), user2.related.Workspaces.size());
    Workspace *ws_from_db =
        user2.WorkspaceByID(user.related.Workspaces[0]->ID());
    ASSERT_TRUE(ws_from_db);
    ASSERT_EQ(user.related.Workspaces[0]->String(), ws_from_db->String());
    ws_from_db = user2.WorkspaceByID(user.related.Workspaces[1]->ID());
    ASSERT_TRUE(ws_from_db);
    ASSERT_EQ(user.related.Workspaces[1]->String(), ws_from_db->String());

    ASSERT_EQ(uint(2), user2.related.Tasks.size());
    Task *task_from_db = user2.TaskByID(user2.related.Tasks[0]->ID());
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

TEST(TogglApiClientTest, ParsesDurationLikeOnTheWeb) {
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
    te.SetDurationUserInput("15m");
    ASSERT_EQ("00:15:00", te.DurationString());

    te.SetDurationInSeconds(0);
    te.SetDurationUserInput("25s");
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
}

TEST(TogglApiClientTest, ParseDurationLargerThan24Hours) {
    TimeEntry te;

    te.SetDurationInSeconds(0);
    te.SetDurationUserInput("90:10:00");
    ASSERT_EQ("90:10:00", te.DurationString());
}

TEST(TogglApiClientTest, InterpretsCrazyStartAndStopAsMissingValues) {
    TimeEntry te;

    ASSERT_EQ(Poco::UInt64(0), te.Start());
    te.SetStartString("0003-03-16T-7:-19:-24Z");
    ASSERT_EQ(Poco::UInt64(0), te.Start());

    ASSERT_EQ(Poco::UInt64(0), te.Stop());
    te.SetStopString("0003-03-16T-5:-52:-51Z");
    ASSERT_EQ(Poco::UInt64(0), te.Stop());
}

TEST(TogglApiClientTest, Continue) {
    testing::Database db;

    User user("kopsik_test", "0.1");
    LoadUserFromJSONString(&user, loadTestData(), true, true);

    // User wants to continue time entries,
    // not create new ones
    user.SetStoreStartAndStopTime(false);

    // Change an old time entry and
    // change its date to today. Continueing the
    // entry should not create new record, but
    // continue the old one.
    TimeEntry *te = user.TimeEntryByID(89818605);
    ASSERT_TRUE(te);
    te->SetStart(time(0));
    te->SetDurOnly(true);

    size_t count = user.related.TimeEntries.size();
    ASSERT_EQ(kopsik::noError, user.Continue(te->GUID()));
    ASSERT_EQ(count, user.related.TimeEntries.size());

    // If the old time entry date is different than
    // today, it should create a new entry when
    // user continues it:
    te->SetStartString("2013-01-25T01:05:15-22:00");
    ASSERT_EQ(kopsik::noError, user.Continue(te->GUID()));
    ASSERT_EQ(count+1, user.related.TimeEntries.size());
}

}  // namespace kopsik

int main(int argc, char **argv) {
    Poco::Logger &logger = Poco::Logger::get("");
    logger.setLevel(Poco::Message::PRIO_DEBUG);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

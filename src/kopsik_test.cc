// Copyright 2013 Tanel Lebedev

#include "gtest/gtest.h"
#include "./toggl_api_client.h"
#include "./kopsik_api_test.h"
#include "./database.h"

#include "Poco/FileStream.h"
#include "Poco/File.h"

#define TESTDB "test.db"

namespace kopsik {

    std::string loadTestData() {
        Poco::FileStream fis("testdata/me.json", std::ios::binary);
        std::stringstream ss;
        ss << fis.rdbuf();
        fis.close();
        return ss.str();
    }

    TEST(KopsikTest, TimeEntryReturnsTags) {
        TimeEntry te;
        te.SetTags("alfa|beeta");
        ASSERT_EQ(std::string("alfa|beeta"), te.Tags());
    }

    TEST(KopsikTest, ProjectsHaveColorCodes) {
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

    TEST(KopsikTest, SaveAndLoadCurrentAPIToken) {
        Poco::File f(TESTDB);
        if (f.exists()) {
            f.remove(false);
        }
        Database db(TESTDB);

        std::string api_token("");
        ASSERT_EQ(noError, db.CurrentAPIToken(&api_token));
        ASSERT_EQ("", api_token);

        api_token = "abc123";
        ASSERT_EQ(noError, db.SetCurrentAPIToken(api_token));
        ASSERT_EQ(noError, db.SetCurrentAPIToken(api_token));

        Poco::UInt64 n(0);
        ASSERT_EQ(noError, db.UInt("select count(1) from sessions", &n));
        ASSERT_EQ(Poco::UInt64(1), n);

        std::string api_token_from_db("");
        ASSERT_EQ(noError, db.CurrentAPIToken(&api_token_from_db));
        ASSERT_EQ("abc123", api_token_from_db);

        ASSERT_EQ(noError, db.ClearCurrentAPIToken());
        ASSERT_EQ(noError, db.UInt("select count(1) from sessions", &n));
        ASSERT_EQ(Poco::UInt64(0), n);

        ASSERT_EQ(noError, db.CurrentAPIToken(&api_token_from_db));
        ASSERT_EQ("", api_token_from_db);
    }

    TEST(KopsikTest, UpdatesTimeEntryFromJSON) {
        Poco::File f(TESTDB);
        if (f.exists()) {
            f.remove(false);
        }
        Database db(TESTDB);

        User user("kopsik_test", "0.1");
        user.LoadFromJSONString(loadTestData(), true, true);

        TimeEntry *te = user.GetTimeEntryByID(89818605);
        ASSERT_TRUE(te);

        std::string json = "{\"id\":89818605,\"description\":\"Changed\"}";
        te->LoadFromJSONString(json);
        ASSERT_EQ("Changed", te->Description());
    }

    TEST(KopsikTest, UpdatesTimeEntryFromFullUserJSON) {
        Poco::File f(TESTDB);
        if (f.exists()) {
            f.remove(false);
        }
        Database db(TESTDB);

        std::string json = loadTestData();

        User user("kopsik_test", "0.1");
        user.LoadFromJSONString(loadTestData(), true, true);

        TimeEntry *te = user.GetTimeEntryByID(89818605);
        ASSERT_TRUE(te);

        size_t n = json.find("Important things");
        ASSERT_TRUE(n);
        json = json.replace(n,
            std::string("Important things").length(), "Even more important!");

        user.LoadFromJSONString(json, true, true);
        te = user.GetTimeEntryByID(89818605);
        ASSERT_TRUE(te);
        ASSERT_EQ("Even more important!", te->Description());
    }

    TEST(KopsikTest, SavesModelsAndKnowsToUpdateWithSameUserInstance) {
        Poco::File f(TESTDB);
        if (f.exists()) {
            f.remove(false);
        }
        Database db(TESTDB);

        User user("kopsik_test", "0.1");
        user.LoadFromJSONString(loadTestData(), true, true);

        Poco::UInt64 n;
        ASSERT_EQ(noError, db.UInt("select count(1) from users", &n));
        ASSERT_EQ(Poco::UInt64(0), n);

        for (int i = 0; i < 3; i++) {
            std::vector<ModelChange> changes;
            ASSERT_EQ(noError, db.SaveUser(&user, true, &changes));

            ASSERT_EQ(noError, db.UInt("select count(1) from users", &n));
            ASSERT_EQ(Poco::UInt64(1), n);

            ASSERT_EQ(noError, db.UInt("select count(1) from workspaces", &n));
            ASSERT_EQ(uint(2), n);

            ASSERT_EQ(noError, db.UInt("select count(1) from clients", &n));
            ASSERT_EQ(uint(1), n);  // 2 clients in json, but one is deleted

            ASSERT_EQ(noError, db.UInt("select count(1) from projects", &n));
            ASSERT_EQ(uint(2), n);

            ASSERT_EQ(noError, db.UInt("select count(1) from tasks", &n));
            ASSERT_EQ(uint(2), n);

            ASSERT_EQ(noError, db.UInt("select count(1) from tags", &n));
            ASSERT_EQ(uint(2), n);

            ASSERT_EQ(noError, db.UInt("select count(1) from time_entries",
                    &n));
            ASSERT_EQ(uint(3), n);
        }
    }

    TEST(KopsikTest, SavesModelsAndKnowsToUpdateWithSeparateUserInstances) {
        Poco::File f(TESTDB);
        if (f.exists()) {
            f.remove(false);
        }
        Database db(TESTDB);

        std::string json = loadTestData();

        User user1("kopsik_test", "0.1");
        user1.LoadFromJSONString(json, true, true);

        std::vector<ModelChange> changes;
        ASSERT_EQ(noError, db.SaveUser(&user1, true, &changes));

        Poco::UInt64 n;
        ASSERT_EQ(noError, db.UInt("select count(1) from users", &n));
        ASSERT_EQ(Poco::UInt64(1), n);

        ASSERT_EQ(noError, db.UInt("select count(1) from workspaces", &n));
        ASSERT_EQ(uint(2), n);

        ASSERT_EQ(noError, db.UInt("select count(1) from clients", &n));
        ASSERT_EQ(uint(1), n);  // 2 clients in JSON but one is deleted

        ASSERT_EQ(noError, db.UInt("select count(1) from projects", &n));
        ASSERT_EQ(uint(2), n);

        ASSERT_EQ(noError, db.UInt("select count(1) from tasks", &n));
        ASSERT_EQ(uint(2), n);

        ASSERT_EQ(noError, db.UInt("select count(1) from tags", &n));
        ASSERT_EQ(uint(2), n);

        ASSERT_EQ(noError, db.UInt("select count(1) from time_entries", &n));
        ASSERT_EQ(uint(3), n);

        User user2("kopsik_test", "0.1");
        ASSERT_EQ(noError, db.LoadUserByID(user1.ID(), &user2, true));

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

        user2.LoadFromJSONString(json, true, true);

        ASSERT_EQ(noError, db.SaveUser(&user2, true, &changes));

        ASSERT_EQ(noError, db.UInt("select count(1) from users", &n));
        ASSERT_EQ(Poco::UInt64(1), n);

        ASSERT_EQ(noError, db.UInt("select count(1) from workspaces", &n));
        ASSERT_EQ(uint(2), n);

        ASSERT_EQ(noError, db.UInt("select count(1) from clients", &n));
        ASSERT_EQ(uint(1), n);  // 2 clients in JSON but 1 is deleted

        ASSERT_EQ(noError, db.UInt("select count(1) from projects", &n));
        ASSERT_EQ(uint(2), n);

        ASSERT_EQ(noError, db.UInt("select count(1) from tasks", &n));
        ASSERT_EQ(uint(2), n);

        ASSERT_EQ(noError, db.UInt("select count(1) from tags", &n));
        ASSERT_EQ(uint(2), n);

        ASSERT_EQ(noError, db.UInt("select count(1) from time_entries", &n));
        ASSERT_EQ(uint(3), n);
    }

    TEST(KopsikTest, TestDeletionSteps) {
        Poco::File f(TESTDB);
        if (f.exists()) {
            f.remove(false);
        }
        Database db(TESTDB);

        User user("kopsik_test", "0.1");
        user.LoadFromJSONString(loadTestData(), true, true);

        // first, mark time entry as deleted
        TimeEntry *te = user.Start("My new time entry", 0, 0);
        std::vector<ModelChange> changes;
        ASSERT_EQ(noError, db.SaveUser(&user, true, &changes));

        te->MarkAsDeletedOnServer();
        {
            Poco::UInt64 te_count(0);
            std::stringstream query;
            query << "select count(1) from time_entries where local_id = "
                << te->LocalID();
            ASSERT_EQ(noError, db.UInt(query.str(), &te_count));
            ASSERT_EQ(Poco::UInt64(1), te_count);
        }

        // now, really delete it
        te->MarkAsDeletedOnServer();
        ASSERT_EQ(noError, db.SaveUser(&user, true, &changes));
        {
            Poco::UInt64 te_count(0);
            std::stringstream query;
            query << "select count(1) from time_entries where local_id = "
                << te->LocalID();
            ASSERT_EQ(noError, db.UInt(query.str(), &te_count));
            ASSERT_EQ(Poco::UInt64(0), te_count);
        }
    }

    TEST(KopsikTest, SavesModels) {
        Poco::FileStream fis("testdata/me.json", std::ios::binary);
        ASSERT_TRUE(fis.good());
        std::stringstream ss;
        ss << fis.rdbuf();
        fis.close();
        User user("kopsik_test", "0.1");
        user.LoadFromJSONString(ss.str(), true, true);

        Poco::File f(TESTDB);
        if (f.exists()) {
            f.remove(false);
        }
        Database db(TESTDB);

        std::vector<ModelChange> changes;
        ASSERT_EQ(noError, db.SaveUser(&user, false, &changes));

        // Time entry
        TimeEntry te;
        te.SetUID(user.ID());
        ASSERT_EQ(noError, db.SaveTimeEntry(&te, &changes));
        ASSERT_TRUE(te.LocalID());
        ASSERT_EQ(noError, db.SaveTimeEntry(&te, &changes));

        // Tag
        Tag t;
        t.SetUID(user.ID());
        ASSERT_EQ(noError, db.SaveTag(&t));
        ASSERT_TRUE(t.LocalID());
        ASSERT_EQ(noError, db.SaveTag(&t));
        ASSERT_EQ(noError, db.DeleteTag(&t));

        // Workspace
        Workspace w;
        w.SetUID(user.ID());
        ASSERT_EQ(noError, db.SaveWorkspace(&w));
        ASSERT_TRUE(w.LocalID());
        ASSERT_EQ(noError, db.SaveWorkspace(&w));
        ASSERT_EQ(noError, db.DeleteWorkspace(&w));

        // Project
        Project p;
        p.SetUID(user.ID());
        ASSERT_EQ(noError, db.SaveProject(&p));
        ASSERT_TRUE(p.LocalID());
        ASSERT_EQ(noError, db.SaveProject(&p));
        ASSERT_EQ(noError, db.DeleteProject(&p));

        // Client
        Client c;
        c.SetUID(user.ID());
        ASSERT_EQ(noError, db.SaveClient(&c));
        ASSERT_TRUE(c.LocalID());
        ASSERT_EQ(noError, db.SaveClient(&c));
        ASSERT_EQ(noError, db.DeleteClient(&c));

        // Workspace
        Task task;
        task.SetUID(user.ID());
        ASSERT_EQ(noError, db.SaveTask(&task));
        ASSERT_TRUE(task.LocalID());
        ASSERT_EQ(noError, db.SaveTask(&task));
        ASSERT_EQ(noError, db.DeleteTask(&task));
    }

    TEST(KopsikTest, ParsesAndSavesData) {
        Poco::FileStream fis("testdata/me.json", std::ios::binary);
        ASSERT_TRUE(fis.good());
        std::stringstream ss;
        ss << fis.rdbuf();
        fis.close();
        std::string json = ss.str();
        ASSERT_FALSE(json.empty());

        User user("kopsik_test", "0.1");
        user.LoadFromJSONString(json, true, true);
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
        ASSERT_EQ(false, user.related.Projects[0]->Active());

        ASSERT_EQ(uint(2567324), user.related.Projects[1]->ID());
        ASSERT_EQ(uint(123456789), user.related.Projects[1]->WID());
        ASSERT_EQ("Even more work", user.related.Projects[1]->Name());
        ASSERT_EQ(uint(1385144), user.related.Projects[1]->CID());
        ASSERT_EQ(user.ID(), user.related.Projects[1]->UID());
        ASSERT_EQ(true, user.related.Projects[1]->Active());

        // Time entries
        ASSERT_EQ(uint(3), user.related.TimeEntries.size());

        ASSERT_EQ(uint(89818605), user.related.TimeEntries[0]->ID());
        ASSERT_EQ("07fba193-91c4-0ec8-2894-820df0548a8f",
            user.related.TimeEntries[0]->GUID());
        ASSERT_EQ(uint(2567324), user.related.TimeEntries[0]->PID());
        ASSERT_EQ(true, user.related.TimeEntries[0]->Billable());
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

        // Clients (2 in JSON but 1 is deleted)
        ASSERT_EQ(uint(1), user.related.Clients.size());

        ASSERT_EQ(uint(878318), user.related.Clients[0]->ID());
        ASSERT_EQ(uint(123456789), user.related.Clients[0]->WID());
        ASSERT_EQ("Big Client", user.related.Clients[0]->Name());
        ASSERT_EQ("59b464cd-0f8e-e601-ff44-f135225a6738",
            user.related.Clients[0]->GUID());
        ASSERT_EQ(user.ID(), user.related.Clients[0]->UID());

        Poco::File f(TESTDB);
        if (f.exists()) {
            f.remove(false);
        }
        Database db(TESTDB);

        Poco::UInt64 n;
        ASSERT_EQ(noError, db.UInt("select count(1) from users", &n));
        ASSERT_EQ(Poco::UInt64(0), n);

        // Insert
        std::vector<ModelChange> changes;
        ASSERT_EQ(noError, db.SaveUser(&user, true, &changes));
        ASSERT_GT(user.LocalID(), uint(0));
        ASSERT_GT(user.ID(), uint(0));
        ASSERT_FALSE(user.APIToken().empty());

        ASSERT_EQ(noError, db.UInt("select count(1) from users", &n));
        ASSERT_EQ(Poco::UInt64(1), n);

        ASSERT_EQ(noError, db.UInt("select count(1) from workspaces", &n));
        ASSERT_EQ(Poco::UInt64(user.related.Workspaces.size()), n);

        ASSERT_EQ(noError, db.UInt("select count(1) from clients", &n));
        ASSERT_EQ(Poco::UInt64(user.related.Clients.size()), n);

        ASSERT_EQ(noError, db.UInt("select count(1) from projects", &n));
        ASSERT_EQ(Poco::UInt64(user.related.Projects.size()), n);

        ASSERT_EQ(noError, db.UInt("select count(1) from tasks", &n));
        ASSERT_EQ(Poco::UInt64(user.related.Tasks.size()), n);

        ASSERT_EQ(noError, db.UInt("select count(1) from tags", &n));
        ASSERT_EQ(Poco::UInt64(user.related.Tags.size()), n);

        ASSERT_EQ(noError, db.UInt("select count(1) from time_entries", &n));
        ASSERT_EQ(Poco::UInt64(user.related.TimeEntries.size()), n);

        // Update
        ASSERT_EQ(noError, db.SaveUser(&user, true, &changes));
        ASSERT_EQ(noError, db.UInt("select count(1) from users", &n));
        ASSERT_EQ(Poco::UInt64(1), n);

        // Select
        User user2("kopsik_test", "0.1");
        ASSERT_EQ(noError, db.LoadUserByID(user.ID(), &user2, true));

        ASSERT_TRUE(user2.ID());
        ASSERT_EQ(user.ID(), user2.ID());
        ASSERT_TRUE(user2.Since());
        ASSERT_EQ(user.Since(), user2.Since());
        ASSERT_EQ(user.ID(), user2.ID());
        ASSERT_EQ(user.DefaultWID(), user2.DefaultWID());

        ASSERT_EQ(uint(2), user2.related.Projects.size());
        Project *project_from_db =
            user2.GetProjectByID(user.related.Projects[0]->ID());
        ASSERT_TRUE(project_from_db);
        ASSERT_EQ(user.related.Projects[0]->String(),
            project_from_db->String());
        project_from_db = user2.GetProjectByID(user.related.Projects[1]->ID());
        ASSERT_EQ(user.related.Projects[1]->String(),
            project_from_db->String());

        ASSERT_EQ(uint(3), user2.related.TimeEntries.size());
        TimeEntry *te_from_db =
            user2.GetTimeEntryByID(user.related.TimeEntries[0]->ID());
        ASSERT_TRUE(te_from_db);
        ASSERT_EQ(user.related.TimeEntries[0]->String(), te_from_db->String());
        te_from_db = user2.GetTimeEntryByID(user.related.TimeEntries[1]->ID());
        ASSERT_TRUE(te_from_db);
        ASSERT_EQ(user.related.TimeEntries[1]->String(), te_from_db->String());
        te_from_db = user2.GetTimeEntryByID(user.related.TimeEntries[2]->ID());
        ASSERT_TRUE(te_from_db);
        ASSERT_EQ(user.related.TimeEntries[2]->String(), te_from_db->String());

        ASSERT_EQ(uint(2), user2.related.Workspaces.size());
        Workspace *ws_from_db =
            user2.GetWorkspaceByID(user.related.Workspaces[0]->ID());
        ASSERT_TRUE(ws_from_db);
        ASSERT_EQ(user.related.Workspaces[0]->String(), ws_from_db->String());
        ws_from_db = user2.GetWorkspaceByID(user.related.Workspaces[1]->ID());
        ASSERT_TRUE(ws_from_db);
        ASSERT_EQ(user.related.Workspaces[1]->String(), ws_from_db->String());

        ASSERT_EQ(uint(2), user2.related.Tasks.size());
        Task *task_from_db = user2.GetTaskByID(user2.related.Tasks[0]->ID());
        ASSERT_EQ(user.related.Tasks[0]->String(), task_from_db->String());
        ASSERT_EQ(user.related.Tasks[1]->String(),
            user2.related.Tasks[1]->String());

        ASSERT_EQ(uint(1), user2.related.Clients.size());
        ASSERT_EQ(user.related.Clients[0]->String(),
            user2.related.Clients[0]->String());

        // Delete
        ASSERT_EQ(noError, db.DeleteUser(&user, true));

        ASSERT_EQ(noError, db.UInt("select count(1) from users", &n));
        ASSERT_EQ(Poco::UInt64(0), n);

        ASSERT_EQ(noError, db.UInt("select count(1) from projects", &n));
        ASSERT_EQ(Poco::UInt64(0), n);

        ASSERT_EQ(noError, db.UInt("select count(1) from workspaces", &n));
        ASSERT_EQ(Poco::UInt64(0), n);

        ASSERT_EQ(noError, db.UInt("select count(1) from tasks", &n));
        ASSERT_EQ(Poco::UInt64(0), n);

        ASSERT_EQ(noError, db.UInt("select count(1) from time_entries", &n));
        ASSERT_EQ(Poco::UInt64(0), n);

        ASSERT_EQ(noError, db.UInt("select count(1) from tags", &n));
        ASSERT_EQ(Poco::UInt64(0), n);

        ASSERT_EQ(noError, db.UInt("select count(1) from clients", &n));
        ASSERT_EQ(Poco::UInt64(0), n);
    }

    TEST(KopsikTest, ParsesDurationLikeOnTheWeb) {
        TimeEntry te;

        te.SetDurationString("00:00:15");
        ASSERT_EQ("00:00:15", te.DurationString());

        te.SetDurationInSeconds(0);
        te.SetDurationString("00:23:15");
        ASSERT_EQ("00:23:15", te.DurationString());

        te.SetDurationInSeconds(0);
        te.SetDurationString("12:34:56");
        ASSERT_EQ("12:34:56", te.DurationString());

        te.SetDurationInSeconds(0);
        te.SetDurationString("0:1");
        ASSERT_EQ("00:01:00", te.DurationString());

        te.SetDurationInSeconds(0);
        te.SetDurationString("1:2");
        ASSERT_EQ("01:02:00", te.DurationString());

        te.SetDurationInSeconds(0);
        te.SetDurationString("1:0");
        ASSERT_EQ("01:00:00", te.DurationString());

        te.SetDurationInSeconds(0);
        te.SetDurationString("05:22 min");
        ASSERT_EQ("00:05:22", te.DurationString());

        te.SetDurationInSeconds(0);
        te.SetDurationString("00:22 min");
        ASSERT_EQ("00:00:22", te.DurationString());

        te.SetDurationInSeconds(0);
        te.SetDurationString("0 hours");
        ASSERT_EQ("00:00:00", te.DurationString());

        te.SetDurationInSeconds(0);
        te.SetDurationString("0.5 hours");
        ASSERT_EQ("00:30:00", te.DurationString());

        te.SetDurationInSeconds(0);
        te.SetDurationString("0,5 hours");
        ASSERT_EQ("00:30:00", te.DurationString());

        te.SetDurationInSeconds(0);
        te.SetDurationString("1 hour");
        ASSERT_EQ("01:00:00", te.DurationString());

        te.SetDurationInSeconds(0);
        te.SetDurationString("1,5 hours");
        ASSERT_EQ("01:30:00", te.DurationString());

        te.SetDurationInSeconds(0);
        te.SetDurationString("1.5 hours");
        ASSERT_EQ("01:30:00", te.DurationString());

        te.SetDurationInSeconds(0);
        te.SetDurationString("24 hours");
        ASSERT_EQ(86400, te.DurationInSeconds());

        te.SetDurationInSeconds(0);
        te.SetDurationString("0 minutes");
        ASSERT_EQ("00:00:00", te.DurationString());

        te.SetDurationInSeconds(0);
        te.SetDurationString("0 min");
        ASSERT_EQ("00:00:00", te.DurationString());

        te.SetDurationInSeconds(0);
        te.SetDurationString("0,5 minutes");
        ASSERT_EQ("00:00:30", te.DurationString());

        te.SetDurationInSeconds(0);
        te.SetDurationString("1 minute");
        ASSERT_EQ("00:01:00", te.DurationString());

        te.SetDurationInSeconds(0);
        te.SetDurationString("1,5 minutes");
        ASSERT_EQ("00:01:30", te.DurationString());

        te.SetDurationInSeconds(0);
        te.SetDurationString("1.5 minutes");
        ASSERT_EQ("00:01:30", te.DurationString());

        te.SetDurationInSeconds(0);
        te.SetDurationString("15");
        ASSERT_EQ("00:15:00", te.DurationString());

        te.SetDurationInSeconds(0);
        te.SetDurationString("0 seconds");
        ASSERT_EQ("00:00:00", te.DurationString());

        te.SetDurationInSeconds(0);
        te.SetDurationString("1 second");
        ASSERT_EQ("00:00:01", te.DurationString());

        te.SetDurationInSeconds(0);
        te.SetDurationString("1.5h");
        ASSERT_EQ("01:30:00", te.DurationString());

        te.SetDurationInSeconds(0);
        te.SetDurationString("15m");
        ASSERT_EQ("00:15:00", te.DurationString());

        te.SetDurationInSeconds(0);
        te.SetDurationString("25s");
        ASSERT_EQ("00:00:25", te.DurationString());
    }

}  // namespace kopsik

int main(int argc, char **argv) {
    Poco::Logger &logger = Poco::Logger::get("");
    logger.setLevel(Poco::Message::PRIO_DEBUG);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

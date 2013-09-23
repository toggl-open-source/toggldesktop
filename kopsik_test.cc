// Copyright 2013 Tanel Lebedev

#include "gtest/gtest.h"
#include "./toggl_api_client.h"
#include "./database.h"

#include "Poco/FileStream.h"
#include "Poco/File.h"

namespace kopsik {

    TEST(KopsikTest, SaveAndLoadCurrentAPIToken) {
        Poco::File f("test.db");
        if (f.exists()) {
            f.remove(false);
        }
        Database db("test.db");

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

    TEST(KopsikTest, SavesModelsAndKnowsToUpdateWithSameUserInstance) {
        Poco::File f("test.db");
        if (f.exists()) {
            f.remove(false);
        }
        Database db("test.db");

        Poco::FileStream fis("testdata/me.json", std::ios::binary);
        std::stringstream ss;
        ss << fis.rdbuf();
        fis.close();

        User user;
        ASSERT_EQ(noError, user.LoadFromJSONString(ss.str(), true));

        Poco::UInt64 n;
        ASSERT_EQ(noError, db.UInt("select count(1) from users", &n));
        ASSERT_EQ(Poco::UInt64(0), n);

        for (int i = 0; i < 3; i++) {
            ASSERT_EQ(noError, db.SaveUser(&user, true));

            ASSERT_EQ(noError, db.UInt("select count(1) from users", &n));
            ASSERT_EQ(Poco::UInt64(1), n);

            ASSERT_EQ(noError, db.UInt("select count(1) from workspaces", &n));
            ASSERT_EQ(uint(2), n);

            ASSERT_EQ(noError, db.UInt("select count(1) from clients", &n));
            ASSERT_EQ(uint(2), n);

            ASSERT_EQ(noError, db.UInt("select count(1) from projects", &n));
            ASSERT_EQ(uint(2), n);

            ASSERT_EQ(noError, db.UInt("select count(1) from tasks", &n));
            ASSERT_EQ(uint(2), n);

            ASSERT_EQ(noError, db.UInt("select count(1) from tags", &n));
            ASSERT_EQ(uint(0), n);

            ASSERT_EQ(noError, db.UInt("select count(1) from time_entries",
                    &n));
            ASSERT_EQ(uint(3), n);
        }
    }

    TEST(KopsikTest, SavesModelsAndKnowsToUpdateWithSeparateUserInstances) {
        Poco::File f("test.db");
        if (f.exists()) {
            f.remove(false);
        }
        Database db("test.db");

        Poco::FileStream fis("testdata/me.json", std::ios::binary);
        std::stringstream ss;
        ss << fis.rdbuf();
        fis.close();

        std::string json = ss.str();

        User user1;
        ASSERT_EQ(noError, user1.LoadFromJSONString(json, true));

        ASSERT_EQ(noError, db.SaveUser(&user1, true));

        Poco::UInt64 n;
        ASSERT_EQ(noError, db.UInt("select count(1) from users", &n));
        ASSERT_EQ(Poco::UInt64(1), n);

        ASSERT_EQ(noError, db.UInt("select count(1) from workspaces", &n));
        ASSERT_EQ(uint(2), n);

        ASSERT_EQ(noError, db.UInt("select count(1) from clients", &n));
        ASSERT_EQ(uint(2), n);

        ASSERT_EQ(noError, db.UInt("select count(1) from projects", &n));
        ASSERT_EQ(uint(2), n);

        ASSERT_EQ(noError, db.UInt("select count(1) from tasks", &n));
        ASSERT_EQ(uint(2), n);

        ASSERT_EQ(noError, db.UInt("select count(1) from tags", &n));
        ASSERT_EQ(uint(0), n);

        ASSERT_EQ(noError, db.UInt("select count(1) from time_entries", &n));
        ASSERT_EQ(uint(3), n);

        User user2;
        ASSERT_EQ(noError, db.LoadUserByID(user1.ID, &user2, true));

        ASSERT_EQ(user1.Workspaces.size(), user2.Workspaces.size());
        ASSERT_EQ(user1.Clients.size(), user2.Clients.size());
        ASSERT_EQ(user1.Projects.size(), user2.Projects.size());
        ASSERT_EQ(user1.Tasks.size(), user2.Tasks.size());
        ASSERT_EQ(user1.Tags.size(), user2.Tags.size());
        ASSERT_EQ(user1.TimeEntries.size(), user2.TimeEntries.size());

        ASSERT_EQ(noError, user2.LoadFromJSONString(json, true));

        ASSERT_EQ(noError, db.SaveUser(&user2, true));

        ASSERT_EQ(noError, db.UInt("select count(1) from users", &n));
        ASSERT_EQ(Poco::UInt64(1), n);

        ASSERT_EQ(noError, db.UInt("select count(1) from workspaces", &n));
        ASSERT_EQ(uint(2), n);

        ASSERT_EQ(noError, db.UInt("select count(1) from clients", &n));
        ASSERT_EQ(uint(2), n);

        ASSERT_EQ(noError, db.UInt("select count(1) from projects", &n));
        ASSERT_EQ(uint(2), n);

        ASSERT_EQ(noError, db.UInt("select count(1) from tasks", &n));
        ASSERT_EQ(uint(2), n);

        ASSERT_EQ(noError, db.UInt("select count(1) from tags", &n));
        ASSERT_EQ(uint(0), n);

        ASSERT_EQ(noError, db.UInt("select count(1) from time_entries", &n));
        ASSERT_EQ(uint(3), n);
    }

    TEST(KopsikTest, SavesModels) {
        Poco::FileStream fis("testdata/me.json", std::ios::binary);
        ASSERT_TRUE(fis.good());
        std::stringstream ss;
        ss << fis.rdbuf();
        fis.close();
        User user;
        ASSERT_EQ(noError, user.LoadFromJSONString(ss.str(), true));

        Poco::File f("test.db");
        if (f.exists()) {
            f.remove(false);
        }
        Database db("test.db");

        ASSERT_EQ(noError, db.SaveUser(&user, false));

        // Time entry
        TimeEntry te;
        te.UID = user.ID;
        ASSERT_EQ(noError, db.SaveTimeEntry(&te));
        ASSERT_TRUE(te.LocalID);
        ASSERT_EQ(noError, db.SaveTimeEntry(&te));
        ASSERT_EQ(noError, db.DeleteTimeEntry(&te));

        // Tag
        Tag t;
        t.UID = user.ID;
        ASSERT_EQ(noError, db.SaveTag(&t));
        ASSERT_TRUE(t.LocalID);
        ASSERT_EQ(noError, db.SaveTag(&t));
        ASSERT_EQ(noError, db.DeleteTag(&t));

        // Workspace
        Workspace w;
        w.UID = user.ID;
        ASSERT_EQ(noError, db.SaveWorkspace(&w));
        ASSERT_TRUE(w.LocalID);
        ASSERT_EQ(noError, db.SaveWorkspace(&w));
        ASSERT_EQ(noError, db.DeleteWorkspace(&w));

        // Project
        Project p;
        p.UID = user.ID;
        ASSERT_EQ(noError, db.SaveProject(&p));
        ASSERT_TRUE(p.LocalID);
        ASSERT_EQ(noError, db.SaveProject(&p));
        ASSERT_EQ(noError, db.DeleteProject(&p));

        // Client
        Client c;
        c.UID = user.ID;
        ASSERT_EQ(noError, db.SaveClient(&c));
        ASSERT_TRUE(c.LocalID);
        ASSERT_EQ(noError, db.SaveClient(&c));
        ASSERT_EQ(noError, db.DeleteClient(&c));

        // Workspace
        Task task;
        task.UID = user.ID;
        ASSERT_EQ(noError, db.SaveTask(&task));
        ASSERT_TRUE(task.LocalID);
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

        User user;
        ASSERT_EQ(noError, user.LoadFromJSONString(json, true));
        ASSERT_EQ(Poco::UInt64(1379068550), user.Since);
        ASSERT_EQ(Poco::UInt64(1047), user.ID);
        ASSERT_EQ(Poco::UInt64(123456788), user.DefaultWID);
        ASSERT_EQ("30eb0ae954b536d2f6628f7fec47beb6", user.APIToken);
        ASSERT_EQ("John Smith", user.Fullname);

        // Projects
        ASSERT_EQ(uint(2), user.Projects.size());

        ASSERT_EQ(uint(2598305), user.Projects[0]->ID);
        ASSERT_EQ("2f0b8f51-f898-d992-3e1a-6bc261fc41ef",
            user.Projects[0]->GUID);
        ASSERT_EQ(uint(123456789), user.Projects[0]->WID);
        ASSERT_EQ("Testing stuff", user.Projects[0]->Name);
        ASSERT_EQ(user.ID, user.Projects[0]->UID);

        ASSERT_EQ(uint(2567324), user.Projects[1]->ID);
        ASSERT_EQ(uint(123456789), user.Projects[1]->WID);
        ASSERT_EQ("Even more work", user.Projects[1]->Name);
        ASSERT_EQ(uint(1129807), user.Projects[1]->CID);
        ASSERT_EQ(user.ID, user.Projects[1]->UID);

        // Time entries
        ASSERT_EQ(uint(3), user.TimeEntries.size());

        ASSERT_EQ(uint(89818605), user.TimeEntries[0]->ID);
        ASSERT_EQ("07fba193-91c4-0ec8-2894-820df0548a8f",
            user.TimeEntries[0]->GUID);
        ASSERT_EQ(uint(2567324), user.TimeEntries[0]->PID);
        ASSERT_EQ(true, user.TimeEntries[0]->Billable);
        ASSERT_EQ(uint(1378362830), user.TimeEntries[0]->Start);
        ASSERT_EQ(uint(1378369186), user.TimeEntries[0]->Stop);
        ASSERT_EQ(6356, user.TimeEntries[0]->DurationInSeconds);
        ASSERT_EQ("Important things", user.TimeEntries[0]->Description);
        ASSERT_EQ(uint(0), user.TimeEntries[0]->TagNames.size());
        ASSERT_EQ(false, user.TimeEntries[0]->DurOnly);
        ASSERT_EQ(user.ID, user.TimeEntries[0]->UID);

        ASSERT_EQ(uint(2), user.Tasks.size());

        ASSERT_EQ(uint(1894794), user.Tasks[0]->ID);
        ASSERT_EQ("blog (writing)", user.Tasks[0]->Name);
        ASSERT_EQ(uint(123456789), user.Tasks[0]->WID);
        ASSERT_EQ(uint(2585208), user.Tasks[0]->PID);
        ASSERT_EQ(user.ID, user.Tasks[0]->UID);

        // FIXME: Tags

        // Workspaces
        ASSERT_EQ(uint(2), user.Workspaces.size());

        ASSERT_EQ(uint(123456788), user.Workspaces[0]->ID);
        ASSERT_EQ("stuff", user.Workspaces[0]->Name);
        ASSERT_EQ(user.ID, user.Workspaces[0]->UID);

        // Clients
        ASSERT_EQ(uint(2), user.Clients.size());

        ASSERT_EQ(uint(1385144), user.Clients[0]->ID);
        ASSERT_EQ(uint(123456789), user.Clients[0]->WID);
        ASSERT_EQ("ABC", user.Clients[0]->Name);
        ASSERT_EQ("59b464cd-0f8e-e601-ff44-f135225a6738",
            user.Clients[1]->GUID);
        ASSERT_EQ(user.ID, user.Clients[0]->UID);

        Poco::File f("test.db");
        if (f.exists()) {
            f.remove(false);
        }
        Database db("test.db");

        Poco::UInt64 n;
        ASSERT_EQ(noError, db.UInt("select count(1) from users", &n));
        ASSERT_EQ(Poco::UInt64(0), n);

        // Insert
        ASSERT_EQ(noError, db.SaveUser(&user, true));
        ASSERT_GT(user.LocalID, uint(0));
        ASSERT_GT(user.ID, uint(0));
        ASSERT_FALSE(user.APIToken.empty());

        ASSERT_EQ(noError, db.UInt("select count(1) from users", &n));
        ASSERT_EQ(Poco::UInt64(1), n);

        ASSERT_EQ(noError, db.UInt("select count(1) from workspaces", &n));
        ASSERT_EQ(Poco::UInt64(user.Workspaces.size()), n);

        ASSERT_EQ(noError, db.UInt("select count(1) from clients", &n));
        ASSERT_EQ(Poco::UInt64(user.Clients.size()), n);

        ASSERT_EQ(noError, db.UInt("select count(1) from projects", &n));
        ASSERT_EQ(Poco::UInt64(user.Projects.size()), n);

        ASSERT_EQ(noError, db.UInt("select count(1) from tasks", &n));
        ASSERT_EQ(Poco::UInt64(user.Tasks.size()), n);

        ASSERT_EQ(noError, db.UInt("select count(1) from tags", &n));
        ASSERT_EQ(Poco::UInt64(user.Tags.size()), n);

        ASSERT_EQ(noError, db.UInt("select count(1) from time_entries", &n));
        ASSERT_EQ(Poco::UInt64(user.TimeEntries.size()), n);

        // Update
        ASSERT_EQ(noError, db.SaveUser(&user, true));
        ASSERT_EQ(noError, db.UInt("select count(1) from users", &n));
        ASSERT_EQ(Poco::UInt64(1), n);

        // Select
        User user2;
        ASSERT_EQ(noError, db.LoadUserByID(user.ID, &user2, true));

        ASSERT_TRUE(user2.ID);
        ASSERT_EQ(user.ID, user2.ID);
        ASSERT_TRUE(user2.Since);
        ASSERT_EQ(user.Since, user2.Since);
        ASSERT_EQ(user.ID, user2.ID);
        ASSERT_EQ(user.DefaultWID, user2.DefaultWID);

        ASSERT_EQ(uint(2), user2.Projects.size());
        Project *project_from_db = user2.GetProjectByID(user.Projects[0]->ID);
        ASSERT_TRUE(project_from_db);
        ASSERT_EQ(user.Projects[0]->String(), project_from_db->String());
        project_from_db = user2.GetProjectByID(user.Projects[1]->ID);
        ASSERT_EQ(user.Projects[1]->String(), project_from_db->String());

        ASSERT_EQ(uint(3), user2.TimeEntries.size());
        TimeEntry *te_from_db = user2.GetTimeEntryByID(user.TimeEntries[0]->ID);
        ASSERT_TRUE(te_from_db);
        ASSERT_EQ(user.TimeEntries[0]->String(), te_from_db->String());
        te_from_db = user2.GetTimeEntryByID(user.TimeEntries[1]->ID);
        ASSERT_TRUE(te_from_db);
        ASSERT_EQ(user.TimeEntries[1]->String(), te_from_db->String());
        te_from_db = user2.GetTimeEntryByID(user.TimeEntries[2]->ID);
        ASSERT_TRUE(te_from_db);
        ASSERT_EQ(user.TimeEntries[2]->String(), te_from_db->String());

        ASSERT_EQ(uint(2), user2.Workspaces.size());
        Workspace *ws_from_db = user2.GetWorkspaceByID(user.Workspaces[0]->ID);
        ASSERT_TRUE(ws_from_db);
        ASSERT_EQ(user.Workspaces[0]->String(), ws_from_db->String());
        ws_from_db = user2.GetWorkspaceByID(user.Workspaces[1]->ID);
        ASSERT_TRUE(ws_from_db);
        ASSERT_EQ(user.Workspaces[1]->String(), ws_from_db->String());

        ASSERT_EQ(uint(2), user2.Tasks.size());
        Task *task_from_db = user2.GetTaskByID(user2.Tasks[0]->ID);
        ASSERT_EQ(user.Tasks[0]->String(), task_from_db->String());
        ASSERT_EQ(user.Tasks[1]->String(), user2.Tasks[1]->String());

        ASSERT_EQ(uint(2), user2.Clients.size());
        ASSERT_EQ(user.Clients[0]->String(), user2.Clients[0]->String());
        ASSERT_EQ(user.Clients[1]->String(), user2.Clients[1]->String());

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

}  // namespace kopsik

int main(int argc, char **argv) {
    Poco::Logger &logger = Poco::Logger::get("");
    logger.setLevel(Poco::Message::PRIO_DEBUG);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

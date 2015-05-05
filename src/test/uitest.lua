
-- set timeouts for testers

local seconds = 1

-- disable update check

print("disable update check")

toggl.disable_update_check()

-- set_cacert_path

print("set_cacert_path")

toggl.set_cacert_path("src/ssl/cacert.pem")

-- set test env

print("set test env")

toggl.set_environment("test")

local env = toggl.environment()

assert("test" == env)

-- load test profile

print("load test profile")

assert(toggl.logout())

local f = assert(io.open("testdata/me.json", "r"))
local json = assert(f:read("*all"))
f:close()

assert(toggl.set_logged_in_user(json))

toggl.sleep(seconds)

-- get username and email

print("get username and email")

local fullname = assert(toggl.user_fullname())
local email = assert(toggl.user_email())

assert("John Smith" == fullname)
assert("johnsmith@toggl.com" == email)

toggl.sleep(seconds)

-- start timer

print("start timer")

local description = "tracking time like a boss"
local duration = ""
local task_id = 0
local project_id = 0

local guid = toggl.start(description, duration, task_id, project_id)

assert(guid)

toggl.sleep(seconds)

-- stop timer

print("stop timer")

assert(toggl.stop())

toggl.sleep(seconds)

-- continue time entry

print("continue time entry")

assert(toggl.continue(guid))

toggl.sleep(seconds)

-- show app

print("show app")

toggl.show_app()

-- edit time entry

print("edit time entry")

toggl.edit(guid, false, "")

toggl.sleep(seconds)

-- set time entry duration

print("set time entry duration")

assert(toggl.set_time_entry_duration(guid, "1 hour"))

toggl.sleep(seconds)

-- set time entry project

print("set time entry project")

local tid = 0
local pid = 0
local pguid = ""

assert(toggl.set_time_entry_project(guid, tid, pid, pguid))

toggl.sleep(seconds)

-- set time entry date

print("set time entry date")

local unix_timestamp = 1426200477

assert(toggl.set_time_entry_date(guid, unix_timestamp))

toggl.sleep(seconds)

-- set time entry start

print("set time entry start")

local start = "12:34"

assert(toggl.set_time_entry_start(guid, start))

toggl.sleep(seconds)

-- set time entry end

print("set time entry end")

local endtime ="23:45"

assert(toggl.set_time_entry_end(guid, endtime))

toggl.sleep(seconds)

-- set time entry tags

print("set time entry tags")

local tags = "a\tb\tc"

assert(toggl.set_time_entry_tags(guid, tags))

toggl.sleep(seconds)

-- set time entry billable

print("set time entry billable")

local billable = true

assert(toggl.set_time_entry_billable(guid, billable))

toggl.sleep(seconds)

-- set time entry description

print("set time entry description")

local description = "doing things"

assert(toggl.set_time_entry_description(guid, description))

toggl.sleep(seconds)

-- delete time entry

print("delete time entry")

assert(toggl.delete_time_entry(guid))

toggl.sleep(seconds)

-- continue deleted entry

print("continue deleted entry")

assert(not toggl.continue(guid))

toggl.sleep(seconds)

-- delete deleted entry

print("delete deleted entry")

assert(not toggl.delete_time_entry(guid))

toggl.sleep(seconds)

-- create project

print("create project")

local wid = 123456789
local cid = 878318
local project_name = "Startup"
local is_private = false

assert(toggl.create_project(wid, cid, project_name, is_private))

toggl.sleep(seconds)

-- create client

print("create client")

local client_name = "Even Bigger Client"

assert(toggl.create_client(wid, client_name))

toggl.sleep(seconds)

-- create large number of time entries

print("create large number of time entries")

description = "tracking time like a boss"
duration = ""
task_id = 0
project_id = 0

for i=1,500 do
	local guid = toggl.start(description, duration, task_id, project_id)
	assert(guid)
end

-- log user out

print("log user out")

assert(toggl.logout())

assert(string.len(toggl.user_fullname()) == 0)
assert(string.len(toggl.user_email()) == 0)

toggl.sleep(seconds)

-- clear cache

print("clear cache")

assert(toggl.set_logged_in_user(json))

assert(string.len(toggl.user_fullname()) > 0)
assert(string.len(toggl.user_email()) > 0)

assert(toggl.clear_cache())

assert(string.len(toggl.user_fullname()) == 0)
assert(string.len(toggl.user_email()) == 0)

toggl.sleep(seconds)

-- done

print("UI test finished")

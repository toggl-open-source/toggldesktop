
-- set timeouts for testers

local seconds = 0

-- disable update check

print("disable update check")

toggl.disable_update_check()

toggl.sleep(seconds)

-- set_cacert_path

print("set_cacert_path")

toggl.set_cacert_path("src/ssl/cacert.pem")

toggl.sleep(seconds)

-- set test env

print("set test env")

toggl.set_environment("test")

local env = toggl.environment()

assert("test" == env)

toggl.sleep(seconds)

-- set_db_path

print("set DB path")

toggl.set_db_path("test/uitest.db")

toggl.sleep(seconds)

-- get and set update path

print("get and set update path")

local update_path = toggl.update_path()
assert(update_path)

toggl.set_update_path(update_path)

toggl.sleep(seconds)

-- debug

toggl.debug("this is a debug message")

toggl.sleep(seconds)

-- init another instance of context

print("init another instance of context")

local another_context = toggl.context_init("lua app", "1.0.0")
assert(another_context)

toggl.sleep(seconds)

-- clear another instance of context

print("clear another instance of context")

toggl.context_clear(another_context)

toggl.sleep(seconds)

-- load test profile

print("load test profile")

assert(toggl.logout())

local f = assert(io.open("testdata/me.json", "r"))
local json = assert(f:read("*all"))
f:close()

assert(toggl.set_logged_in_user(json))

toggl.sleep(seconds)

-- timeline_is_recording_enabled

print("timeline_is_recording_enabled")

assert(toggl.timeline_is_recording_enabled())

toggl.sleep(seconds)

-- toggl_timeline_toggle_recording

print("toggl_timeline_toggle_recording")

assert(toggl.timeline_toggle_recording(false))

assert(not toggl.timeline_is_recording_enabled())

toggl.sleep(seconds)

-- sync

print("sync")

toggl.sync()

toggl.sleep(seconds)

-- open in browser

print("open in browser")

toggl.open_in_browser()

toggl.sleep(seconds)

-- edit preferences

print("edit preferences")

toggl.edit_preferences()

toggl.sleep(seconds)

-- view_time_entry_list

print("view_time_entry_list")

toggl.view_time_entry_list()

toggl.sleep(seconds)

-- password_forgot

print("password_forgot")

toggl.password_forgot()

toggl.sleep(seconds)

-- set_default_project_id

print("set_default_project_id")

assert(toggl.set_default_project(2567324, 0))
assert(2567324 == toggl.get_default_project_id())
assert("Even more work" == toggl.get_default_project_name())

toggl.sleep(seconds)

-- set_log_level

print("set_log_level")

toggl.set_log_level("trace")

toggl.sleep(seconds)

-- run_script

print("run_script")

local script_result = toggl.run_script("print(123)")
assert(script_result)

toggl.sleep(seconds)

-- get_support

print("get_support")

toggl.get_support()

toggl.sleep(seconds)

-- set_online

print("set_online")

toggl.set_online()

toggl.sleep(seconds)

-- set_sleep

print("set_sleep")

toggl.set_sleep()

toggl.sleep(seconds)

-- set_idle_seconds

print("set_idle_seconds")

toggl.set_idle_seconds(3600)

toggl.sleep(seconds)

-- set_wake

print("set_wake")

toggl.set_wake()

toggl.sleep(seconds)

-- check_view_struct_size

print("check_view_struct_size")

local errmsg = toggl.check_view_struct_size(1, 2, 3, 4, 5)
assert(errmsg)

toggl.sleep(seconds)

-- parse_duration_string_into_seconds

print("parse_duration_string_into_seconds")

assert(12 == toggl.parse_duration_string_into_seconds("12 sec"))

toggl.sleep(seconds)

-- get_update_channel

print("get_update_channel")

local channel = toggl.get_update_channel()
assert("stable" == channel, channel)

toggl.sleep(seconds)

-- set_update_channel

print("set_update_channel")

assert(toggl.set_update_channel("beta"))

channel = toggl.get_update_channel()
assert("beta" == channel, channel)

toggl.sleep(seconds)

-- get_user_fullname

print("get_user_fullname")

assert("John Smith" == toggl.get_user_fullname())

toggl.sleep(seconds)

-- get_user_email

print("get_user_email")

assert("johnsmith@toggl.com" == toggl.get_user_email())

toggl.sleep(seconds)

-- format_tracking_time_duration

print("format_tracking_time_duration")

local formatted_tracking_time = toggl.format_tracking_time_duration(120)
assert("02:00 min" == formatted_tracking_time, formatted_tracking_time)

toggl.sleep(seconds)

-- format_tracked_time_duration

print("format_tracked_time_duration")

local formatted_tracked_time = toggl.format_tracked_time_duration(120)
assert("0:02" == formatted_tracked_time, formatted_tracked_time)

toggl.sleep(seconds)

-- add obm action

print("add OBM action")

assert(toggl.add_obm_action(123, "some", "thing"))

toggl.sleep(seconds)

-- add autotracker rule

print("add autotracker rule")

local rule_id = toggl.autotracker_add_rule("facebook", 2598305);
assert(rule_id)

toggl.sleep(seconds)

-- delete autotracker rule

print("delete autotracker rule")

assert(toggl.autotracker_delete_rule(rule_id))

toggl.sleep(seconds)

-- login/logout in loop to check if it hangs

print("login/logout in loop to check if it hangs")

for i=1,10 do
	print(i)

	assert(toggl.logout())
	assert(toggl.set_logged_in_user(json))

	toggl.sleep(seconds)
end

-- get username and email

print("get username and email")

local fullname = assert(toggl.get_user_fullname())
local email = assert(toggl.get_user_email())

assert("John Smith" == fullname)
assert("johnsmith@toggl.com" == email)

toggl.sleep(seconds)

-- start timer

print("start timer")

local description = "tracking time like a boss"
local duration = ""
local task_id = 0
local project_id = 0

local guid = toggl.start(description, duration, task_id, project_id, "", "")

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

local project_guid = toggl.create_project(wid, cid, "startup project", false)
assert(project_guid)

for i=1,500 do
	local guid = toggl.start("tracking time like a boss", "10 minutes", 0, 0, project_guid, "")
	assert(guid)

	local res = toggl.set_time_entry_tags(guid, "a|b|c|foo|bar|and|one|more|tag")
	assert(res)
end

-- log user out

print("log user out")

assert(toggl.logout())

assert(string.len(toggl.get_user_fullname()) == 0)
assert(string.len(toggl.get_user_email()) == 0)

toggl.sleep(seconds)

-- clear cache

print("clear cache")

assert(toggl.set_logged_in_user(json))

assert(string.len(toggl.get_user_fullname()) > 0)
assert(string.len(toggl.get_user_email()) > 0)

assert(toggl.clear_cache())

assert(string.len(toggl.get_user_fullname()) == 0)
assert(string.len(toggl.get_user_email()) == 0)

toggl.sleep(seconds)

-- done

print("UI test finished")

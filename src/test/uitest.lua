
-- set timeouts for testers

local seconds = 2

-- set test env

toggl.set_environment("test")

-- check test env

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

-- log user out

print("log user out")

assert(toggl.logout())

toggl.sleep(seconds)

-- finish

print("UI test finished")

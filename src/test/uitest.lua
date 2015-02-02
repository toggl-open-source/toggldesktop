
toggl.set_environment("test")

assert(toggl.logout())

local f = assert(io.open("testdata/me.json", "r"))
local json = assert(f:read("*all"))
f:close()

assert(toggl.set_logged_in_user(json))

local seconds = 2

toggl.sleep(seconds)

local description = "tracking time like a boss"
local duration = ""
local task_id = 0
local project_id = 0

assert(toggl.start(description, duration, task_id, project_id))

toggl.sleep(seconds)

assert(toggl.stop())

toggl.sleep(seconds)

local seconds = 2

toggl.sleep(seconds)

local description = "tracking time like a boss"
local duration = ""
local task_id = 0
local project_id = 0

toggl.start(description, duration, task_id, project_id)

toggl.sleep(seconds)

toggl.stop()

toggl.sleep(seconds)
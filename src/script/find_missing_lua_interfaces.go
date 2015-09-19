package main

import (
	"io/ioutil"
	"os"
	"strings"
)

func main() {
	missing, err := findMissingInterfaces()
	if err != nil {
		println(err.Error())
		os.Exit(1)
	}
	if len(missing) > 0 {
		println("Following API methods are not available via Lua:")
		println("------------------------------------------------")
		for _, f := range missing {
			println(f)
		}
	} else {
		println("All API methods are available via Lua. Nice.")
	}
	os.Exit(0)
}

func findMissingInterfaces() ([]string, error) {
	apiFuncs, err := findApiFuncs()
	if err != nil {
		return nil, err
	}
	luaFuncs, err := findLuaFuncs()
	if err != nil {
		return nil, err
	}
	var missing []string
	for _, apiFunc := range apiFuncs {
		found := false
		for _, luaFunc := range luaFuncs {
			if apiFunc == luaFunc {
				found = true
				break
			}
		}
		if !found {
			missing = append(missing, apiFunc)
		}
	}
	return missing, nil
}

func findLuaFuncs() ([]string, error) {
	// read in API declarations
	b, err := ioutil.ReadFile("src/toggl_api_lua.h")
	if err != nil {
		return nil, err
	}
	// look for funcs in the source
	l := strings.Split(string(b), "\n")
	var funcs []string
	for _, s := range l {
		if !strings.Contains(s, "static int l_toggl_") {
			continue
		}
		s = strings.Replace(s, "static int l_toggl_", "", -1)
		s = strings.Replace(s, "(lua_State *L) {", "", -1)
		funcs = append(funcs, s)
	}
	return funcs, nil
}

func findApiFuncs() ([]string, error) {
	// read in API declarations
	b, err := ioutil.ReadFile("src/toggl_api.h")
	if err != nil {
		return nil, err
	}
	// look for funcs in the source
	l := strings.Split(string(b), "\n")
	var funcs []string
	for _, s := range l {
		// accept only exported funcs
		if !strings.Contains(s, "TOGGL_EXPORT") {
			continue
		}
		// accept only funcs
		if !strings.Contains(s, "toggl_") {
			continue
		}
		// skip callbacks
		if strings.Contains(s, "toggl_on_") {
			continue
		}
		// get func name without prefix
		parts := strings.Split(s, "toggl_")
		if len(parts) < 2 {
			continue
		}
		name := strings.Replace(parts[1], "(", "", -1)
		funcs = append(funcs, name)
	}
	return funcs, nil
}

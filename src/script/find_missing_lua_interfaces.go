package main

import (
	"io/ioutil"
	"os"
	"strings"
)

func main() {
	var apiFuncs, luaFuncs, testFuncs []string
	var err error
	if err == nil {
		apiFuncs, err = findAPIFuncs()
	}
	if err == nil {
		luaFuncs, err = findLuaFuncs()
	}
	if err == nil {
		testFuncs, err = findTestFuncs()
	}
	if err != nil {
		println(err.Error())
		os.Exit(1)
	}
	// Print out missing Lua funcs
	if missing := findMissingInterfaces(apiFuncs, luaFuncs); len(missing) > 0 {
		println("------------------------------------------------")
		println("Following API methods are not available via Lua:")
		println("------------------------------------------------")
		for _, f := range missing {
			println(f)
		}
	} else {
		println("All API methods are available via Lua. Nice.")
	}
	// Print out untested API funcs
	if missing := findMissingInterfaces(apiFuncs, testFuncs); len(missing) > 0 {
		println("----------------------------------------------------")
		println("Following API methods are not tested via Lua script:")
		println("----------------------------------------------------")
		for _, f := range missing {
			println(f)
		}
	} else {
		println("All API methods are available via Lua. Nice.")
	}
	os.Exit(0)
}

func findMissingInterfaces(expected []string, actual []string) []string {
	var missing []string
	for _, expectedFunc := range expected {
		found := false
		for _, actualFunc := range actual {
			if expectedFunc == actualFunc {
				found = true
				break
			}
		}
		if !found {
			missing = append(missing, expectedFunc)
		}
	}
	return missing
}

func findTestFuncs() ([]string, error) {
	b, err := ioutil.ReadFile("src/test/uitest.lua")
	if err != nil {
		return nil, err
	}
	// look for funcs in the source
	l := strings.Split(string(b), "\n")
	uniq := make(map[string]bool)
	for _, s := range l {
		if !strings.Contains(s, "toggl.") {
			continue
		}
		parts := strings.Split(s, "toggl.")
		if len(parts) < 2 {
			continue
		}
		s = parts[1]
		parts = strings.Split(s, "(")
		if len(parts) < 2 {
			continue
		}
		s = parts[0]
		uniq[s] = true
	}
	var funcs []string
	for f := range uniq {
		funcs = append(funcs, f)
	}
	return funcs, nil
}

func findLuaFuncs() ([]string, error) {
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

func findAPIFuncs() ([]string, error) {
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
